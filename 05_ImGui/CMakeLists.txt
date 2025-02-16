cmake_minimum_required(VERSION 3.18.1)

project(ImGui VERSION 0.1)
set(TARGET_NAME ${PROJECT_NAME})

#============================ PROFILER ===========================
option(ENABLE_PROFILER "Generate gmon.out for gprof" OFF)
if (ENABLE_PROFILER)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()
#=================================================================
option(ENABLE_IMGUI "Enable ImGui Library" ON)
if(ENABLE_IMGUI)
    add_compile_definitions(ENABLE_IMGUI)
endif()
#============================== LIBS =============================
add_subdirectory(../libs/Window  Window)
add_subdirectory(../libs/vkUtils vkUtils)
add_subdirectory(../libs/imgui   DearImGui)
#=================================================================
#============================= SOURCE ============================
include_directories (cpp)
include_directories (cpp/sg)
aux_source_directory(cpp    SRC_LIST)
aux_source_directory(cpp/sg SRC_LIST)
#=================================================================
#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(${TARGET_NAME} ${SRC_LIST})

    #Build static libstdc++
    option(STATIC_LINK_LIBSTDCPP "Required for NVidia Nsight" OFF)
    if (STATIC_LINK_LIBSTDCPP)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    endif()

endif()
#=================================================================
#=============================WINDOWS=============================
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    add_executable(${TARGET_NAME} ${SRC_LIST})
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    set(TARGET_NAME native)
    add_library(${TARGET_NAME} SHARED ${SRC_LIST})
endif()
#=================================================================
#===============================ALL===============================
target_link_libraries(${TARGET_NAME} Window)
target_link_libraries(${TARGET_NAME} vkUtils)

option(ENABLE_IMGUI "Enable ImGui Library" ON)
if(ENABLE_IMGUI)
    add_compile_definitions(ENABLE_IMGUI)
    target_link_libraries(${TARGET_NAME} DearImGui)
endif()

#=================================================================
