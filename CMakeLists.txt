cmake_minimum_required(VERSION 3.18.1)
project(vkSamples)

#  Reduce size of MinSizeRel builds
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # smaller exe -flto
endif()

# Libraries
#add_subdirectory(libs/Window  ../../libs/Window/build)
#add_subdirectory(libs/vkUtils ../../libs/vkUtils/build)

# Samples
add_subdirectory(01_keyEvents ../../01_keyEvents/build)
add_subdirectory(02_cube      ../../02_cube/build)
add_subdirectory(03_glTF      ../../03_glTF/build)
add_subdirectory(04_vkRay     ../../04_vkRay/build)
add_subdirectory(05_ImGui     ../../05_ImGui/build)
