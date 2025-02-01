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
    #add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    #link_libraries (${CMAKE_DL_LIBS}) # for dlsym
    #set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)  # smaller exe -flto
    #add_library(${PROJECT_NAME} ${SRC_LIST})
    #target_link_libraries(${PROJECT_NAME} Window)
endif()
#=================================================================
#=============================WINDOWS=============================
if(WIN32)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    #add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    #add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    #add_library(${PROJECT_NAME} ${SRC_LIST})
    #target_link_libraries(${PROJECT_NAME} Window)
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    #message("Platform = ${ANDROID_PLATFORM}")
    #add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
    #add_library(${PROJECT_NAME} ${SRC_LIST})

    # Link to external libraries
    #target_link_libraries(${PROJECT_NAME} log android)
    #target_link_libraries(${PROJECT_NAME} Window)
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
