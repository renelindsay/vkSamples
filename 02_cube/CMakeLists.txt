cmake_minimum_required(VERSION 3.18.1)

project(cube VERSION 0.1)

#============================== LIBS =============================
add_subdirectory(../libs/Window  Window)
add_subdirectory(../libs/vkUtils vkUtils)
#=================================================================
#============================= SOURCE ============================
include_directories (cpp)
aux_source_directory(cpp SRC_LIST)
#=================================================================
#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(${PROJECT_NAME} ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} Window)
    target_link_libraries(${PROJECT_NAME} vkUtils)
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
