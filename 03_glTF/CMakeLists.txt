cmake_minimum_required(VERSION 3.18.1)

project(glTF VERSION 0.4)

#============================ PROFILER ===========================
option(ENABLE_PROFILER "Generate gmon.out for gprof" OFF)
if (ENABLE_PROFILER)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()
#=================================================================

#============================== LIBS =============================
add_subdirectory(../libs/Window  Window)
add_subdirectory(../libs/vkUtils vkUtils)
#=================================================================
#============================= SOURCE ============================
include_directories (cpp)
include_directories (cpp/sg)
aux_source_directory(cpp    SRC_LIST)
aux_source_directory(cpp/sg SRC_LIST)
#=================================================================
#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(${PROJECT_NAME} ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} Window)
    target_link_libraries(${PROJECT_NAME} vkUtils)

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
    add_executable(${PROJECT_NAME} ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} Window)
    target_link_libraries(${PROJECT_NAME} vkUtils)
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    message("Platform = ${ANDROID_PLATFORM}")
    add_library(native SHARED ${SRC_LIST})
    target_link_libraries(native log android)
    target_link_libraries(native Window)
    target_link_libraries(native vkUtils)
endif()
#=================================================================
