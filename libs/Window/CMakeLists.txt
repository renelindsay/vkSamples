cmake_minimum_required(VERSION 3.18.1)
project(Window)

if(TARGET ${PROJECT_NAME})
    return()  #exit if target already exists
endif()

#============================= SOURCE ============================
aux_source_directory(. SRC_LIST)
#=================================================================

#==============================LINUX==============================
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")    
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")  #for find_package
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    #add_definitions(-DENABLE_MULTITOUCH)
    #add_definitions(-DENABLE_XCB_IMAGE)
    #add_definitions(-DENABLE_XCB_CURSOR)

    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})
    link_libraries(${X11_LIBRARIES})
    link_libraries(${X11_xcb_LIB})        #               (libxcb.so)
    link_libraries(${X11_X11_xcb_LIB})    #               (libX11-xcb.so)
    link_libraries(${X11_xkbcommon_LIB})  # Keyboard      (libxkbcommon.so)
    link_libraries(${X11_Xinput_LIB})     # MultiTouch    (libXi.so)
    link_libraries(${X11_xcb_image_LIB})  # ShowImage     (libxcb-image.so)
    link_libraries(${X11_xcb_cursor_LIB}) # Mouse cursors (libxcb-cursor.so)

    add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

    # Add LIBEVDEV for Gamepad support (requires package: libevdev_dev)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBEVDEV REQUIRED libevdev)
    target_include_directories(${PROJECT_NAME} PUBLIC ${LIBEVDEV_INCLUDE_DIRS})  # "/usr/include/libevdev-1.0"
    target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBEVDEV_LIBRARIES})          # evdev
endif()
#=================================================================
#=============================WINDOWS=============================
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    remove_definitions(-Dfar)
    remove_definitions(-Dnear)
    remove_definitions(-DLoadImage)
    add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} Xinput.lib)    # for Gamepad
endif()
#=================================================================
#=============================ANDROID=============================
if(ANDROID)
    message("Platform = ${ANDROID_PLATFORM}")
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)

    # Set path to source files.
    aux_source_directory(${ANDROID_NDK}/sources/android/native_app_glue SRC_LIST)
    aux_source_directory(android SRC_LIST)

    # Create native shared library    
    add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
    target_link_libraries(${PROJECT_NAME} log android)

    # Set path to header files.
    target_include_directories(${PROJECT_NAME} PUBLIC android)
    target_include_directories(${PROJECT_NAME} PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)
endif()
#=================================================================

#===============================ALL===============================
# Share include directories to apps that use this lib
target_include_directories(${PROJECT_NAME} PUBLIC .)
#=================================================================
