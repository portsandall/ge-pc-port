cmake_minimum_required(VERSION 3.22)
project(geand_bootstrap C)

if(NOT ANDROID)
  message(FATAL_ERROR "GEAND bootstrap is Android-only")
endif()

add_library(geand_bootstrap SHARED geand_loader.c)
target_compile_features(geand_bootstrap PRIVATE c_std_11)
target_link_libraries(geand_bootstrap PRIVATE android dl log)
set_target_properties(geand_bootstrap PROPERTIES OUTPUT_NAME "geand_bootstrap")
