//===============================================LOGGING================================================
//  This unit provides 6 LOG* macros, for printing to the Console or Android Logcat.
//  LOG* functions can be used in the same way as printf, but uses color-coding, for better readability.
//  Undefine the ENABLE_LOGGING flag to strip out log messages and reduce exe size for release.

#define ENABLE_LOGGING 1

#ifndef LOGGING_H
#define LOGGING_H

#if defined(__linux__) && !defined(__ANDROID__)  // Linux (desktop only)
#define __LINUX__ 1
#endif

#ifdef _WIN32
    #ifndef VK_USE_PLATFORM_WIN32_KHR
    #define VK_USE_PLATFORM_WIN32_KHR
    #endif
    #undef near
    #undef far
    #undef LoadImage
    #undef  NOMINMAX
    #define NOMINMAX

    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0A00
    #define WINVER 0x0A00
    #endif
    #include <Windows.h>
    #include <stdio.h>

    #define cTICK "\xFB" /* On Windows, use Square-root as tick mark */
    #define PAUSE system("pause")
#elif __ANDROID__
    #ifndef VK_USE_PLATFORM_ANDROID_KHR
    #define VK_USE_PLATFORM_ANDROID_KHR
    #endif
    #include <native.h>
    #define cTICK "\u2713"
    #define PAUSE
#elif __LINUX__
    #ifndef VK_USE_PLATFORM_XCB_KHR
    #define VK_USE_PLATFORM_XCB_KHR
    #endif
    #include <xkbcommon/xkbcommon.h>  // install with: sudo apt-get install libxkbcommon-dev
    #define cTICK "\u2713"
    #define PAUSE
#endif

//#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef ANDROID
#define RED
#define GREEN
#define YELLOW
#define BLUE
#define MAGENTA
#define CYAN
#define WHITE
#define FAINT
#define RED2
#define GREEN2
#define YELLOW2
#define BLUE2
#define MAGENTA2
#define CYAN2
#define WHITE2
#define RESET
#else
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define BLUE     "\033[34m"
#define MAGENTA  "\033[35m"
#define CYAN     "\033[36m"
#define WHITE    "\033[37m"
#define FAINT    "\033[90m"
#define RED2     "\033[91m"
#define GREEN2   "\033[92m"
#define YELLOW2  "\033[93m"
#define BLUE2    "\033[94m"
#define MAGENTA2 "\033[95m"
#define CYAN2    "\033[96m"
#define WHITE2   "\033[97m"
#define RESET    "\033[0m"

#define STRIKE   "\033[9m"
#define NOSTRIKE "\033[29m"
#endif
//-------------------------Text Color--------------------------
enum eColor { eRESET, eRED, eGREEN, eYELLOW, eBLUE, eMAGENTA, eCYAN, eWHITE,     // normal colors
              eFAINT,eBRED,eBGREEN,eBYELLOW,eBBLUE,eBMAGENTA,eBCYAN, eBRIGHT };  // bright colors

static void txt_color(eColor color) {  // Sets Terminal text color (Win32/Linux)
#ifdef _WIN32
    const char bgr[] = {7, 4, 2, 6, 1, 5, 3, 7, 8, 12, 10, 14, 9, 13, 11, 15};  // RGB-to-BGR
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, bgr[color]);
#elif __LINUX__
    if (color == eFAINT) { printf("\033[37m\033[2m"); return; } // set faint white
    printf("\033[%dm", (color & 8) ? 1 : 0);                    // bright or normal
    if (color) printf("\033[3%dm", color & 7);                  // set text color
#endif
}
//-------------------------------------------------------------
#define ENABLE_ERRLOG

// clang-format off
    #define print(COLOR,...) { txt_color(COLOR); printf(__VA_ARGS__);  txt_color(eRESET); }
#ifdef ENABLE_LOGGING
    //#define print(COLOR,...) { txt_color(COLOR); printf(__VA_ARGS__);  txt_color(eRESET); }
    #ifdef ANDROID
        #include <jni.h>
        #include <android/log.h>
        #define LOG_TAG    "Window"
        #define LOG(...)    __android_log_print(ANDROID_LOG_INFO   ,LOG_TAG,__VA_ARGS__)
        #define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
        #define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG  ,LOG_TAG,__VA_ARGS__)
        #define LOGI(...)   __android_log_print(ANDROID_LOG_INFO   ,LOG_TAG,__VA_ARGS__)
        #define LOGW(...)   __android_log_print(ANDROID_LOG_WARN   ,LOG_TAG,__VA_ARGS__)
        #define LOGE(...)   __android_log_print(ANDROID_LOG_ERROR  ,LOG_TAG,__VA_ARGS__)
        //#define printf(...)  __android_log_print(ANDROID_LOG_INFO   ,LOG_TAG,__VA_ARGS__)
        #define ASSERT(EXPRESSION, ...) { if(!(EXPRESSION)) { LOGE(__VA_ARGS__); printf("%s:%d\n", __FILE__, __LINE__); abort(); }  }
    #else //desktop
        #define LOG(...)  {                            printf(__VA_ARGS__);}
        #define LOGV(...) {print(eCYAN,  "PERF : "  ); printf(__VA_ARGS__);}
        #define LOGD(...) {print(eBLUE,  "DEBUG: "  ); printf(__VA_ARGS__);}
        #define LOGI(...) {print(eGREEN, "INFO : "  ); printf(__VA_ARGS__);}
        #define LOGW(...) {print(eYELLOW,"WARNING: "); printf(__VA_ARGS__);}
        #define LOGE(...) {print(eRED,   "ERROR: "  ); printf(__VA_ARGS__);}
        #define ASSERT(EXPRESSION, ...) { if(!(EXPRESSION)) { LOGE(__VA_ARGS__); printf("%s:%d\n", __FILE__, __LINE__); abort(); }  }
    #endif
#elif defined ENABLE_ERRLOG
        #define LOG(...)  {}
        #define LOGV(...) {print(eCYAN,  "PERF : "  ); printf(__VA_ARGS__);}
        #define LOGD(...) {print(eBLUE,  "DEBUG: "  ); printf(__VA_ARGS__);}
        #define LOGI(...) {}
        #define LOGW(...) {print(eYELLOW,"WARNING: "); printf(__VA_ARGS__);}
        #define LOGE(...) {print(eRED,   "ERROR: "  ); printf(__VA_ARGS__);}
        #define ASSERT(EXPRESSION, ...) { if(!(EXPRESSION)) { LOGE(__VA_ARGS__); printf("%s:%d\n", __FILE__, __LINE__); abort(); }  }
#else // logging disabled
    #define  LOG(...)  {}
    #define  LOGV(...) {}
    #define  LOGD(...) {}
    #define  LOGI(...) {}
    #define  LOGW(...) {}
    #define  LOGE(...) {}
    #define ASSERT(EXPRESSION, ...)
    //#define print(COLOR,...) {}
#endif  // ENABLE_LOGGING

//======================================================================================================
#include <chrono>
using namespace std::chrono;

class Timer {
    high_resolution_clock::time_point last;

  public:
    Timer(const char* str="") { printf("%s", str); Start(); }
    void Start() {last = high_resolution_clock::now();}
    double Span(bool reset = true) {
        high_resolution_clock::time_point curr = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(curr - last);
        if (reset) last = curr;
        return time_span.count();
    }

    void Print(const char* str="") {
        print(eCYAN,  "%s : ", str); printf("%.3fs\n", Span());
    }
};
//======================================================================================================

#ifdef _WIN32
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING  0x0004

static bool EnableVTMode() {
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return false;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) return false;
    return true;
}
#endif  // Win32

#endif  // LOGGING_H
//======================================================================================================
