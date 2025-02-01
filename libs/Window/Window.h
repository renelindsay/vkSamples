/*
*--------------------------------------------------------------------------
*
*  The CWindow class creates a window and main event processing loop.
*  It provides functions for querying the current state of the window, keyboard,
*  and mouse. Also, events may be processed via either polling or callbacks.
*
*  For polling, use the "GetEvent" function to return one event at a time,
*  and process, using a case statement.  For an example, see the "ProcessEvents" implementation.
*
*  For callbacks, use the "ProcessEvents" function to dispatch all queued events to their
*  appropriate event handlers.  To create event handlers, derive your class from Window,
*  and override the virtual event handler functions. (See WindowBase.h)
*
*--------------------------------------------------------------------------
*/

// NOTE: Window.h MUST be #included BEFORE stdio.h, for printf to work correctly on Android.

#ifdef ANDROID
//#include <native.h>
#endif

#ifdef WIN32
  #include <direct.h>
  #define getcwd _getcwd
  #define chdir _chdir

  //for MINGW
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT 0x0A00
  #undef  WINVER
  #define WINVER 0x0A00

#endif  
  
#ifdef LINUX
  #include <cstdlib>
  #include <unistd.h>
#endif

#include "WindowBase.h"
#include "window_xcb.h"
#include "window_win32.h"
#include "window_android.h"

#ifndef CWINDOW_H
#define CWINDOW_H

#if defined(VK_USE_PLATFORM_XCB_KHR)
    typedef Window_xcb CWindow;
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    typedef Window_win32 CWindow;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    typedef Window_android CWindow;
#endif

#endif
