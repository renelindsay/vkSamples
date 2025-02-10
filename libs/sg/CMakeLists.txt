cmake_minimum_required(VERSION 3.18.1)
project(sg)

if(TARGET ${PROJECT_NAME})
    return() # exit if target already exists
endif()

#============================= SOURCE ============================
aux_source_directory(. SRC_LIST)
#=================================================================
add_library(${PROJECT_NAME} ${SRC_LIST})
#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
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
target_include_directories(${PROJECT_NAME} PUBLIC .)

target_link_libraries(${PROJECT_NAME} vkUtils)
target_link_libraries(${PROJECT_NAME} Window)
#=================================================================
