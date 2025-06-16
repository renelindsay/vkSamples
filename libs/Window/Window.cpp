#include "Window.h"

#define WINDOW_IMPLEMENTATION
#include "window_xcb.h"
#include "window_win32.h"
#include "window_android.h"

#define FIND_ASSETS_FOLDER
#ifdef  FIND_ASSETS_FOLDER

#if !defined(__ANDROID__)
#ifdef _WIN32
  #include <direct.h>
  #define getcwd _getcwd
  #define chdir _chdir
#elif __linux__
  #include <cstdlib>
  #include <unistd.h>
#endif

struct InitDir {
    InitDir() {
        int     err = chdir("./assets/..");
        if(err) err = chdir("../assets/..");
        if(err) err = chdir("../../assets/..");

        char buf[4096] {};
        printf("Current Dir : %s\n", getcwd(buf, 4096));
        if(err){printf("ERROR: Failed to find assets folder.\n"); abort();}
    }
} InitDir;
#endif  // not ANDROID

#endif //FIND_ASSETS_FOLDER


//--------------------------------------------------

