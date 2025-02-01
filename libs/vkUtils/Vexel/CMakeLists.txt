set(LIBRARY_NAME Vexel)

file(GLOB SRC_LIST "*.h" "*.cpp")                       # List all source files from current directory
add_library (${LIBRARY_NAME} STATIC ${SRC_LIST})        # and add them to this library.

target_include_directories (${LIBRARY_NAME} PUBLIC .)
target_include_directories (${LIBRARY_NAME} PUBLIC "vulkan")

#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
endif()
#=================================================================
#=============================WINDOWS=============================
if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
    target_link_libraries(${LIBRARY_NAME} log android)
endif()
#=================================================================
