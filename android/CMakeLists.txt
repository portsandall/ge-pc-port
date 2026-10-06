cmake_minimum_required(VERSION 3.22)
project(geand_android LANGUAGES C CXX ASM)

if(NOT ANDROID)
  message(FATAL_ERROR "GEAND Android build must use the Android NDK toolchain")
endif()

set(GEAND_ROOT "${CMAKE_CURRENT_LIST_DIR}/../work/goldeneye-pc-port")

message(STATUS "GEAND Android")
message(STATUS "ABI=${ANDROID_ABI}")
message(STATUS "API=${ANDROID_PLATFORM}")
message(STATUS "NDK=${ANDROID_NDK}")
message(STATUS "core=${GEAND_ROOT}")

add_library(geand_platform_probe OBJECT platform_probe.c)
target_compile_features(geand_platform_probe PRIVATE c_std_11)
target_compile_definitions(geand_platform_probe PRIVATE
  GEAND_ANDROID=1
  PLATFORM_ANDROID=1
  PORT=1
)
