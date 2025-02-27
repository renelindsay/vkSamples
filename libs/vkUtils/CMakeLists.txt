cmake_minimum_required(VERSION 3.18.1)
project(vkUtils)

if(TARGET ${PROJECT_NAME})
    return() # exit if target already exists
endif()

#========================= SUBDIRECTORIES ========================
add_subdirectory(Vexel)
#=================================================================
#============================= SOURCE ============================
aux_source_directory(.               SRC_LIST)
aux_source_directory(./vkRay         SRC_LIST)
aux_source_directory(./thirdparty    SRC_LIST)
#aux_source_directory(./VulkanWrapper SRC_LIST)
#=================================================================
add_library(${PROJECT_NAME} ${SRC_LIST})
#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #target_compile_options(${PROJECT_NAME} PUBLIC -fno-strict-aliasing)  # for union in matrix.h
    #set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)  # smaller exe -flto
endif()
#=================================================================
#=============================WINDOWS=============================
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    #target_link_libraries(${PROJECT_NAME} log android)
endif()
#=================================================================
#===============================ALL===============================
target_include_directories(${PROJECT_NAME} PUBLIC .              )
target_include_directories(${PROJECT_NAME} PUBLIC ./vkRay        )
target_include_directories(${PROJECT_NAME} PUBLIC ./thirdparty   )
#target_include_directories(${PROJECT_NAME} PUBLIC ./VulkanWrapper)

target_link_libraries(${PROJECT_NAME} Vexel)
target_link_libraries(${PROJECT_NAME} Window)
#=================================================================
