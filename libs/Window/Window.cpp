#include "Window.h"

#define WINDOW_IMPLEMENTATION
#include "window_xcb.h"
#include "window_win32.h"
#include "window_android.h"

#define FIND_ASSETS_FOLDER
#ifdef  FIND_ASSETS_FOLDER

// On startup, set the current working directory to
// the 'assets' folder, to match Android behavior.

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
        int     err = chdir("./assets");
        if(err) err = chdir("../assets");
        if(err) err = chdir("../../assets");
        if(err) err = chdir("../../../assets");

        char buf[4096] {};
        printf("Current Dir : %s\n", getcwd(buf, 4096));
        if(err){printf("ERROR: Failed to change CWD to assets folder.\n"); abort();}
    }
} InitDir;
#endif  // not ANDROID

#endif //FIND_ASSETS_FOLDER


//--------------------------------------------------

