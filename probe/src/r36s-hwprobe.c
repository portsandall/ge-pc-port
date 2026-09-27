// SPDX-License-Identifier: MIT
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/auxv.h>
#include <sys/utsname.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <vulkan/vulkan.h>

#ifndef GL_COMPUTE_SHADER
#define GL_COMPUTE_SHADER 0x91B9
#endif

static int g_failures = 0;

static void kv(const char *key, const char *value)
{
    printf("%s=%s\n", key, value ? value : "(null)");
}

static void kv_u64(const char *key, uint64_t value)
{
    printf("%s=%" PRIu64 "\n", key, value);
}

static void status(const char *key, int ok)
{
    printf("%s=%s\n", key, ok ? "PASS" : "FAIL");
    if (!ok)
        g_failures++;
}

static int contains_ci(const char *haystack, const char *needle)
{
    if (!haystack || !needle)
        return 0;
    size_t hlen = strlen(haystack), nlen = strlen(needle);
    for (size_t i = 0; i + nlen <= hlen; ++i) {
        size_t j = 0;
        for (; j < nlen; ++j) {
            char a = haystack[i + j], b = needle[j];
            if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
            if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
            if (a != b) break;
        }
        if (j == nlen) return 1;
    }
    return 0;
}

static int open_first(const char *const *nodes, const char **chosen)
{
    for (size_t i = 0; nodes[i]; ++i) {
        int fd = open(nodes[i], O_RDWR | O_CLOEXEC);
        if (fd >= 0) {
            if (chosen) *chosen = nodes[i];
            return fd;
        }
    }
    return -1;
}

static double now_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void probe_system(void)
{
    struct utsname u;
    if (uname(&u) == 0) {
        kv("system.sysname", u.sysname);
        kv("system.release", u.release);
        kv("system.machine", u.machine);
    }

    kv_u64("cpu.hwcap", (uint64_t)getauxval(AT_HWCAP));
#ifdef AT_HWCAP2
    kv_u64("cpu.hwcap2", (uint64_t)getauxval(AT_HWCAP2));
#endif

    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0)
        kv_u64("memory.physical_bytes", (uint64_t)pages * (uint64_t)page_size);

    const size_t bytes = 64u * 1024u * 1024u;
    unsigned char *src = NULL, *dst = NULL;
    if (posix_memalign((void **)&src, 64, bytes) == 0 &&
        posix_memalign((void **)&dst, 64, bytes) == 0) {
        memset(src, 0x5a, bytes);
        memset(dst, 0, bytes);
        const int loops = 8;
        double t0 = now_seconds();
        for (int i = 0; i < loops; ++i)
            memcpy(dst, src, bytes);
        double elapsed = now_seconds() - t0;
        volatile unsigned char sink = dst[bytes / 2];
        (void)sink;
        if (elapsed > 0.0) {
            double gib = ((double)bytes * (double)loops) / (1024.0 * 1024.0 * 1024.0);
            printf("memory.memcpy_gib_s=%.3f\n", gib / elapsed);
            status("memory.memcpy_test", 1);
        }
    } else {
        status("memory.memcpy_test", 0);
    }
    free(src);
    free(dst);
}

static void print_drm_cap(int fd, uint64_t cap, const char *name)
{
    uint64_t value = 0;
    int rc = drmGetCap(fd, cap, &value);
    printf("drm.cap.%s=%s", name, rc == 0 ? "yes" : "unsupported");
    if (rc == 0) printf(":%" PRIu64, value);
    putchar('\n');
}

static void probe_kms(void)
{
    static const char *const cards[] = { "/dev/dri/card0", "/dev/dri/card1", NULL };
    const char *node = NULL;
    int fd = open_first(cards, &node);
    if (fd < 0) {
        status("kms.card_open", 0);
        return;
    }
    kv("kms.node", node);
    status("kms.card_open", 1);

    drmVersionPtr ver = drmGetVersion(fd);
    if (ver) {
        printf("kms.driver=%.*s\n", ver->name_len, ver->name);
        printf("kms.driver_version=%d.%d.%d\n",
               ver->version_major, ver->version_minor, ver->version_patchlevel);
        drmFreeVersion(ver);
    }

    print_drm_cap(fd, DRM_CAP_DUMB_BUFFER, "dumb_buffer");
#ifdef DRM_CAP_PRIME
    print_drm_cap(fd, DRM_CAP_PRIME, "prime");
#endif
#ifdef DRM_CAP_ADDFB2_MODIFIERS
    print_drm_cap(fd, DRM_CAP_ADDFB2_MODIFIERS, "addfb2_modifiers");
#endif
#ifdef DRM_CAP_SYNCOBJ
    print_drm_cap(fd, DRM_CAP_SYNCOBJ, "syncobj");
#endif
#ifdef DRM_CAP_SYNCOBJ_TIMELINE
    print_drm_cap(fd, DRM_CAP_SYNCOBJ_TIMELINE, "syncobj_timeline");
#endif

    int atomic = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1) == 0;
    status("kms.atomic_client_cap", atomic);

    drmModeRes *res = drmModeGetResources(fd);
    if (res) {
        printf("kms.connectors=%d\n", res->count_connectors);
        printf("kms.crtcs=%d\n", res->count_crtcs);
        printf("kms.encoders=%d\n", res->count_encoders);
        int connected = 0;
        for (int i = 0; i < res->count_connectors; ++i) {
            drmModeConnector *c = drmModeGetConnector(fd, res->connectors[i]);
            if (!c) continue;
            if (c->connection == DRM_MODE_CONNECTED) connected++;
            printf("kms.connector.%u.connection=%d\n", c->connector_id, c->connection);
            printf("kms.connector.%u.modes=%d\n", c->connector_id, c->count_modes);
            drmModeFreeConnector(c);
        }
        printf("kms.connected=%d\n", connected);
        status("kms.resources", 1);
        drmModeFreeResources(res);
    } else {
        status("kms.resources", 0);
    }
    close(fd);
}

typedef const GLubyte *(*PFNGLGETSTRINGPROC_)(GLenum);
typedef void (*PFNGLCLEARCOLORPROC_)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (*PFNGLCLEARPROC_)(GLbitfield);
typedef void (*PFNGLREADPIXELSPROC_)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
typedef GLenum (*PFNGLGETERRORPROC_)(void);
typedef GLuint (*PFNGLCREATESHADERPROC_)(GLenum);
typedef void (*PFNGLSHADERSOURCEPROC_)(GLuint, GLsizei, const GLchar *const*, const GLint*);
typedef void (*PFNGLCOMPILESHADERPROC_)(GLuint);
typedef void (*PFNGLGETSHADERIVPROC_)(GLuint, GLenum, GLint*);
typedef void (*PFNGLDELETESHADERPROC_)(GLuint);
typedef GLuint (*PFNGLCREATEPROGRAMPROC_)(void);
typedef void (*PFNGLATTACHSHADERPROC_)(GLuint, GLuint);
typedef void (*PFNGLLINKPROGRAMPROC_)(GLuint);
typedef void (*PFNGLGETPROGRAMIVPROC_)(GLuint, GLenum, GLint*);
typedef void (*PFNGLDELETEPROGRAMPROC_)(GLuint);

struct gl_api {
    PFNGLGETSTRINGPROC_ GetString;
    PFNGLCLEARCOLORPROC_ ClearColor;
    PFNGLCLEARPROC_ Clear;
    PFNGLREADPIXELSPROC_ ReadPixels;
    PFNGLGETERRORPROC_ GetError;
    PFNGLCREATESHADERPROC_ CreateShader;
    PFNGLSHADERSOURCEPROC_ ShaderSource;
    PFNGLCOMPILESHADERPROC_ CompileShader;
    PFNGLGETSHADERIVPROC_ GetShaderiv;
    PFNGLDELETESHADERPROC_ DeleteShader;
    PFNGLCREATEPROGRAMPROC_ CreateProgram;
    PFNGLATTACHSHADERPROC_ AttachShader;
    PFNGLLINKPROGRAMPROC_ LinkProgram;
    PFNGLGETPROGRAMIVPROC_ GetProgramiv;
    PFNGLDELETEPROGRAMPROC_ DeleteProgram;
};

static void *gp(const char *name)
{
    return (void *)eglGetProcAddress(name);
}

static int load_gl_api(struct gl_api *g)
{
    memset(g, 0, sizeof(*g));
#define LOAD(field, name) do { g->field = (void *)gp(name); if (!g->field) return 0; } while (0)
    LOAD(GetString, "glGetString");
    LOAD(ClearColor, "glClearColor");
    LOAD(Clear, "glClear");
    LOAD(ReadPixels, "glReadPixels");
    LOAD(GetError, "glGetError");
    LOAD(CreateShader, "glCreateShader");
    LOAD(ShaderSource, "glShaderSource");
    LOAD(CompileShader, "glCompileShader");
    LOAD(GetShaderiv, "glGetShaderiv");
    LOAD(DeleteShader, "glDeleteShader");
    LOAD(CreateProgram, "glCreateProgram");
    LOAD(AttachShader, "glAttachShader");
    LOAD(LinkProgram, "glLinkProgram");
    LOAD(GetProgramiv, "glGetProgramiv");
    LOAD(DeleteProgram, "glDeleteProgram");
#undef LOAD
    return 1;
}

static int compile_program(struct gl_api *g, const char *vs_src, const char *fs_src)
{
    GLuint vs = g->CreateShader(GL_VERTEX_SHADER);
    GLuint fs = g->CreateShader(GL_FRAGMENT_SHADER);
    if (!vs || !fs) return 0;
    g->ShaderSource(vs, 1, &vs_src, NULL);
    g->ShaderSource(fs, 1, &fs_src, NULL);
    g->CompileShader(vs);
    g->CompileShader(fs);
    GLint vok = 0, fok = 0;
    g->GetShaderiv(vs, GL_COMPILE_STATUS, &vok);
    g->GetShaderiv(fs, GL_COMPILE_STATUS, &fok);
    if (!vok || !fok) {
        g->DeleteShader(vs);
        g->DeleteShader(fs);
        return 0;
    }
    GLuint p = g->CreateProgram();
    if (!p) {
        g->DeleteShader(vs);
        g->DeleteShader(fs);
        return 0;
    }
    g->AttachShader(p, vs);
    g->AttachShader(p, fs);
    g->LinkProgram(p);
    GLint linked = 0;
    g->GetProgramiv(p, GL_LINK_STATUS, &linked);
    g->DeleteProgram(p);
    g->DeleteShader(vs);
    g->DeleteShader(fs);
    return linked == GL_TRUE;
}

static int compile_compute(struct gl_api *g)
{
    const char *cs_src = "#version 310 es\nlayout(local_size_x=1) in; void main(){}\n";
    GLuint cs = g->CreateShader(GL_COMPUTE_SHADER);
    if (!cs) return 0;
    g->ShaderSource(cs, 1, &cs_src, NULL);
    g->CompileShader(cs);
    GLint ok = 0;
    g->GetShaderiv(cs, GL_COMPILE_STATUS, &ok);
    g->DeleteShader(cs);
    return ok == GL_TRUE;
}

static EGLConfig choose_config(EGLDisplay dpy, EGLint renderable)
{
    EGLConfig cfg = NULL;
    EGLint n = 0;
    const EGLint attrs[] = {
        EGL_RENDERABLE_TYPE, renderable,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    if (eglChooseConfig(dpy, attrs, &cfg, 1, &n) && n > 0)
        return cfg;

    const EGLint relaxed[] = {
        EGL_RENDERABLE_TYPE, renderable,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_NONE
    };
    n = 0;
    if (eglChooseConfig(dpy, relaxed, &cfg, 1, &n) && n > 0)
        return cfg;
    return NULL;
}

static int run_gl_context(EGLDisplay dpy, const char *tag, EGLenum api, int major)
{
    EGLint renderable = api == EGL_OPENGL_API ? EGL_OPENGL_BIT : EGL_OPENGL_ES2_BIT;
#ifdef EGL_OPENGL_ES3_BIT_KHR
    if (api == EGL_OPENGL_ES_API && major >= 3)
        renderable = EGL_OPENGL_ES3_BIT_KHR;
#endif

    if (!eglBindAPI(api)) {
        printf("%s.bind_api=FAIL:0x%04x\n", tag, eglGetError());
        g_failures++;
        return 0;
    }

    EGLConfig cfg = choose_config(dpy, renderable);
    if (!cfg) {
        printf("%s.config=FAIL:0x%04x\n", tag, eglGetError());
        g_failures++;
        return 0;
    }

    EGLContext ctx = EGL_NO_CONTEXT;
    if (api == EGL_OPENGL_ES_API) {
        const EGLint attrs[] = { EGL_CONTEXT_CLIENT_VERSION, major, EGL_NONE };
        ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, attrs);
    } else {
#ifdef EGL_CONTEXT_MAJOR_VERSION_KHR
        const char *ext = eglQueryString(dpy, EGL_EXTENSIONS);
        if (ext && strstr(ext, "EGL_KHR_create_context")) {
            const EGLint attrs[] = {
                EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
                EGL_CONTEXT_MINOR_VERSION_KHR, 1,
                EGL_NONE
            };
            ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, attrs);
        }
#endif
        if (ctx == EGL_NO_CONTEXT)
            ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, NULL);
    }

    if (ctx == EGL_NO_CONTEXT) {
        printf("%s.context=FAIL:0x%04x\n", tag, eglGetError());
        g_failures++;
        return 0;
    }

    const EGLint pbattrs[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
    EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbattrs);
    if (surf == EGL_NO_SURFACE) {
        printf("%s.pbuffer=FAIL:0x%04x\n", tag, eglGetError());
        eglDestroyContext(dpy, ctx);
        g_failures++;
        return 0;
    }

    if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
        printf("%s.make_current=FAIL:0x%04x\n", tag, eglGetError());
        eglDestroySurface(dpy, surf);
        eglDestroyContext(dpy, ctx);
        g_failures++;
        return 0;
    }

    struct gl_api g;
    if (!load_gl_api(&g)) {
        printf("%s.dispatch=FAIL\n", tag);
        g_failures++;
    } else {
        const char *vendor = (const char *)g.GetString(GL_VENDOR);
        const char *renderer = (const char *)g.GetString(GL_RENDERER);
        const char *version = (const char *)g.GetString(GL_VERSION);
        const char *sl = (const char *)g.GetString(GL_SHADING_LANGUAGE_VERSION);

        printf("%s.vendor=%s\n", tag, vendor ? vendor : "(null)");
        printf("%s.renderer=%s\n", tag, renderer ? renderer : "(null)");
        printf("%s.version=%s\n", tag, version ? version : "(null)");
        printf("%s.glsl=%s\n", tag, sl ? sl : "(null)");
        printf("%s.software_renderer=%s\n", tag,
               contains_ci(renderer, "llvmpipe") ||
               contains_ci(renderer, "softpipe") ||
               contains_ci(renderer, "swrast") ? "yes" : "no");

        g.ClearColor(1.0f, 0.25f, 0.5f, 1.0f);
        g.Clear(GL_COLOR_BUFFER_BIT);
        unsigned char px[4] = {0, 0, 0, 0};
        g.ReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
        GLenum err = g.GetError();
        int readback_ok = err == GL_NO_ERROR &&
                          px[0] > 220 &&
                          px[1] > 35 && px[1] < 95 &&
                          px[2] > 95 && px[2] < 170;
        printf("%s.readback_pixel=%u,%u,%u,%u\n",
               tag, px[0], px[1], px[2], px[3]);
        status(tag, readback_ok);

        const char *vs = NULL;
        const char *fs = NULL;
        if (api == EGL_OPENGL_API) {
            vs = "#version 140\nin vec4 a; void main(){gl_Position=a;}\n";
            fs = "#version 140\nout vec4 c; void main(){c=vec4(1.0,0.0,1.0,1.0);}\n";
        } else if (major >= 3) {
            vs = "#version 300 es\nin vec4 a; void main(){gl_Position=a;}\n";
            fs = "#version 300 es\nprecision mediump float; out vec4 c; void main(){c=vec4(1.0,0.0,1.0,1.0);}\n";
        } else {
            vs = "attribute vec4 a; void main(){gl_Position=a;}\n";
            fs = "precision mediump float; void main(){gl_FragColor=vec4(1.0,0.0,1.0,1.0);}\n";
        }

        printf("%s.shader_link=%s\n",
               tag, compile_program(&g, vs, fs) ? "PASS" : "FAIL");

        if (api == EGL_OPENGL_ES_API && major >= 3) {
            int compute = compile_compute(&g);
            printf("%s.compute_shader=%s\n",
                   tag, compute ? "PASS" : "FAIL_OR_UNSUPPORTED");
        }
    }

    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(dpy, surf);
    eglDestroyContext(dpy, ctx);
    return 1;
}

static void probe_gpu(void)
{
    static const char *const nodes[] = {
        "/dev/dri/renderD128",
        "/dev/dri/renderD129",
        "/dev/dri/card0",
        NULL
    };

    const char *node = NULL;
    int fd = open_first(nodes, &node);
    if (fd < 0) {
        status("drm.render_open", 0);
        return;
    }

    kv("drm.node", node);
    status("drm.render_open", 1);

    int gpu_drm = 0;
    drmVersionPtr ver = drmGetVersion(fd);
    if (ver) {
        printf("drm.driver=%.*s\n", ver->name_len, ver->name);
        printf("drm.version=%d.%d.%d\n",
               ver->version_major, ver->version_minor, ver->version_patchlevel);
        gpu_drm =
            (ver->name && (
                contains_ci(ver->name, "panfrost") ||
                contains_ci(ver->name, "panthor") ||
                contains_ci(ver->name, "lima")));
        printf("drm.gpu_render_driver=%s\n", gpu_drm ? "yes" : "no");
        drmFreeVersion(ver);
    }

    if (!gpu_drm) {
        puts("gbm.path=SKIP_DISPLAY_DRM_NOT_GPU");
        puts("gpu.vendor_mali_node_check=/dev/mali0");
        if (access("/dev/mali0", R_OK | W_OK) == 0)
            puts("gpu.vendor_mali_node=PASS");
        else if (access("/dev/mali0", F_OK) == 0)
            puts("gpu.vendor_mali_node=PRESENT_NO_RW");
        else
            puts("gpu.vendor_mali_node=ABSENT");

        /*
         * Legacy/vendor Mali stacks are not represented by a Panfrost DRM
         * render node. Probe EGL_DEFAULT_DISPLAY separately instead of
         * passing the Rockchip display controller to GBM.
         */
        puts("egl.vendor_path=TRY_DEFAULT_DISPLAY");
        EGLDisplay vdpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (vdpy == EGL_NO_DISPLAY) {
            printf("egl.vendor_default_display=FAIL:0x%04x\n", eglGetError());
        } else {
            EGLint vmaj = 0, vmin = 0;
            if (!eglInitialize(vdpy, &vmaj, &vmin)) {
                printf("egl.vendor_initialize=FAIL:0x%04x\n", eglGetError());
            } else {
                printf("egl.vendor_version=%d.%d\n", vmaj, vmin);
                kv("egl.vendor_name", eglQueryString(vdpy, EGL_VENDOR));
                kv("egl.vendor_client_apis", eglQueryString(vdpy, EGL_CLIENT_APIS));
                kv("egl.vendor_extensions", eglQueryString(vdpy, EGL_EXTENSIONS));
                run_gl_context(vdpy, "vendor_gles2", EGL_OPENGL_ES_API, 2);
                run_gl_context(vdpy, "vendor_gles3", EGL_OPENGL_ES_API, 3);
                run_gl_context(vdpy, "vendor_opengl", EGL_OPENGL_API, 0);
                eglTerminate(vdpy);
            }
        }
        close(fd);
        return;
    }

    puts("gbm.path=TRY_GPU_DRM");
    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        status("gbm.device", 0);
        close(fd);
        return;
    }

    status("gbm.device", 1);
    kv("gbm.backend", gbm_device_get_backend_name(gbm));

    struct gbm_bo *bo = gbm_bo_create(
        gbm, 64, 64, GBM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING);
    if (bo) {
        status("gbm.bo_rendering", 1);
        int dmabuf = gbm_bo_get_fd(bo);
        if (dmabuf >= 0) {
            status("gbm.dmabuf_export", 1);
            close(dmabuf);
        } else {
            status("gbm.dmabuf_export", 0);
        }
        gbm_bo_destroy(bo);
    } else {
        status("gbm.bo_rendering", 0);
    }

    PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");

    EGLDisplay dpy = EGL_NO_DISPLAY;
    if (get_platform_display)
        dpy = get_platform_display(EGL_PLATFORM_GBM_KHR, gbm, NULL);

#if defined(EGL_VERSION_1_5)
    if (dpy == EGL_NO_DISPLAY)
        dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, NULL);
#endif

    if (dpy == EGL_NO_DISPLAY) {
        printf("egl.display=FAIL:0x%04x\n", eglGetError());
        g_failures++;
        gbm_device_destroy(gbm);
        close(fd);
        return;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(dpy, &major, &minor)) {
        printf("egl.initialize=FAIL:0x%04x\n", eglGetError());
        g_failures++;
        gbm_device_destroy(gbm);
        close(fd);
        return;
    }

    printf("egl.version=%d.%d\n", major, minor);
    kv("egl.vendor", eglQueryString(dpy, EGL_VENDOR));
    kv("egl.client_apis", eglQueryString(dpy, EGL_CLIENT_APIS));
    kv("egl.extensions", eglQueryString(dpy, EGL_EXTENSIONS));

    run_gl_context(dpy, "gles2", EGL_OPENGL_ES_API, 2);
    run_gl_context(dpy, "gles3", EGL_OPENGL_ES_API, 3);
    run_gl_context(dpy, "opengl", EGL_OPENGL_API, 0);

    eglTerminate(dpy);
    gbm_device_destroy(gbm);
    close(fd);
}

static void probe_vulkan(void)
{
    void *lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        kv("vulkan.loader", "UNAVAILABLE");
        return;
    }

    PFN_vkGetInstanceProcAddr gip =
        (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr");
    if (!gip) {
        kv("vulkan.loader", "BROKEN");
        dlclose(lib);
        return;
    }

    kv("vulkan.loader", "PASS");

    uint32_t api = VK_API_VERSION_1_0;
    PFN_vkEnumerateInstanceVersion enumver =
        (PFN_vkEnumerateInstanceVersion)
        gip(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
    if (enumver)
        enumver(&api);

    printf("vulkan.instance_api=%u.%u.%u\n",
           VK_API_VERSION_MAJOR(api),
           VK_API_VERSION_MINOR(api),
           VK_API_VERSION_PATCH(api));

    PFN_vkCreateInstance create_instance =
        (PFN_vkCreateInstance)gip(VK_NULL_HANDLE, "vkCreateInstance");
    if (!create_instance) {
        kv("vulkan.instance", "FAIL_NO_ENTRYPOINT");
        dlclose(lib);
        return;
    }

    VkApplicationInfo app = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "r36s-hwprobe",
        .apiVersion = api
    };
    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app
    };

    VkInstance inst = VK_NULL_HANDLE;
    VkResult vr = create_instance(&ci, NULL, &inst);
    if (vr != VK_SUCCESS) {
        printf("vulkan.instance=FAIL:%d\n", vr);
        dlclose(lib);
        return;
    }
    kv("vulkan.instance", "PASS");

    PFN_vkEnumeratePhysicalDevices enum_phys =
        (PFN_vkEnumeratePhysicalDevices)
        gip(inst, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceProperties get_props =
        (PFN_vkGetPhysicalDeviceProperties)
        gip(inst, "vkGetPhysicalDeviceProperties");
    PFN_vkGetPhysicalDeviceQueueFamilyProperties get_qprops =
        (PFN_vkGetPhysicalDeviceQueueFamilyProperties)
        gip(inst, "vkGetPhysicalDeviceQueueFamilyProperties");
    PFN_vkCreateDevice create_dev =
        (PFN_vkCreateDevice)
        gip(inst, "vkCreateDevice");
    PFN_vkDestroyInstance destroy_inst =
        (PFN_vkDestroyInstance)
        gip(inst, "vkDestroyInstance");

    if (!enum_phys || !get_props || !destroy_inst) {
        kv("vulkan.enumeration", "FAIL_ENTRYPOINTS");
        if (destroy_inst)
            destroy_inst(inst, NULL);
        dlclose(lib);
        return;
    }

    uint32_t count = 0;
    vr = enum_phys(inst, &count, NULL);
    printf("vulkan.physical_device_count=%u\n",
           vr == VK_SUCCESS ? count : 0);

    if (vr == VK_SUCCESS && count > 0) {
        VkPhysicalDevice *devs = calloc(count, sizeof(*devs));
        if (devs && enum_phys(inst, &count, devs) == VK_SUCCESS) {
            for (uint32_t i = 0; i < count; ++i) {
                VkPhysicalDeviceProperties p;
                get_props(devs[i], &p);
                printf("vulkan.device.%u.name=%s\n", i, p.deviceName);
                printf("vulkan.device.%u.api=%u.%u.%u\n", i,
                       VK_API_VERSION_MAJOR(p.apiVersion),
                       VK_API_VERSION_MINOR(p.apiVersion),
                       VK_API_VERSION_PATCH(p.apiVersion));
                printf("vulkan.device.%u.vendor_id=0x%04x\n", i, p.vendorID);
                printf("vulkan.device.%u.device_id=0x%04x\n", i, p.deviceID);

                if (i == 0 && get_qprops && create_dev) {
                    uint32_t qn = 0;
                    get_qprops(devs[i], &qn, NULL);
                    VkQueueFamilyProperties *qp = calloc(qn, sizeof(*qp));
                    if (qp) {
                        get_qprops(devs[i], &qn, qp);
                        uint32_t qidx = UINT32_MAX;
                        for (uint32_t q = 0; q < qn; ++q) {
                            if (qp[q].queueFlags &
                                (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
                                qidx = q;
                                break;
                            }
                        }

                        if (qidx != UINT32_MAX) {
                            float priority = 1.0f;
                            VkDeviceQueueCreateInfo qci = {
                                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                .queueFamilyIndex = qidx,
                                .queueCount = 1,
                                .pQueuePriorities = &priority
                            };
                            VkDeviceCreateInfo dci = {
                                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                .queueCreateInfoCount = 1,
                                .pQueueCreateInfos = &qci
                            };
                            VkDevice logical = VK_NULL_HANDLE;
                            vr = create_dev(devs[i], &dci, NULL, &logical);
                            printf("vulkan.device_create=%s:%d\n",
                                   vr == VK_SUCCESS ? "PASS" : "FAIL", vr);
                            if (vr == VK_SUCCESS) {
                                PFN_vkDestroyDevice destroy_dev =
                                    (PFN_vkDestroyDevice)
                                    gip(inst, "vkDestroyDevice");
                                if (destroy_dev)
                                    destroy_dev(logical, NULL);
                            }
                        }
                        free(qp);
                    }
                }
            }
        }
        free(devs);
    }

    destroy_inst(inst, NULL);
    dlclose(lib);
}

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    puts("probe.version=3");
    puts("probe.stage=system");
    probe_system();
    puts("probe.stage=kms");
    probe_kms();
    puts("probe.stage=gpu");
    probe_gpu();
    puts("probe.stage=vulkan");
    probe_vulkan();
    printf("probe.failures=%d\n", g_failures);
    printf("probe.result=%s\n",
           g_failures ? "PARTIAL_OR_FAIL" : "PASS");
    return 0;
}
