cmake_minimum_required(VERSION 3.18.1)
project(DearImGui)

if(TARGET ${PROJECT_NAME})
    return() # exit if target already exists
endif()

add_subdirectory(../Window  ../Window/build)
add_subdirectory(../vkUtils ../vkUtils/build)
#============================= SOURCE ============================
aux_source_directory(.                SRC_LIST)
aux_source_directory(./imgui          SRC_LIST)
aux_source_directory(./imgui/misc/cpp SRC_LIST)
#list(APPEND SRC_LIST ./imgui/backends/imgui_impl_vulkan.cpp)
#=================================================================
#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)  # smaller exe -flto
    add_library(${PROJECT_NAME} ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} Window)
    target_link_libraries(${PROJECT_NAME} vkUtils)
endif()
#=================================================================
#=============================WINDOWS=============================
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    add_library(${PROJECT_NAME} ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} Window)
    target_link_libraries(${PROJECT_NAME} vkUtils)
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    add_library(${PROJECT_NAME} ${SRC_LIST})

    # Link to external libraries
    #target_link_libraries(${PROJECT_NAME} log android)
    target_link_libraries(${PROJECT_NAME} Window)
    target_link_libraries(${PROJECT_NAME} vkUtils)
endif()
#=================================================================
#===============================ALL===============================
target_include_directories(${PROJECT_NAME} PUBLIC .               )
target_include_directories(${PROJECT_NAME} PUBLIC ./imgui         )
target_include_directories(${PROJECT_NAME} PUBLIC ./imgui/misc/cpp)
#target_include_directories(${PROJECT_NAME} PUBLIC ./imgui/backends)
#target_include_directories(${PROJECT_NAME} PUBLIC ../Window       )
#target_include_directories(${PROJECT_NAME} PUBLIC ../vkUtils      )
#=================================================================
