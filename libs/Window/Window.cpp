#include "Window.h"

#define WINDOW_IMPLEMENTATION
#include "window_xcb.h"
#include "window_win32.h"
#include "window_android.h"
#include "Logging.h"

#define FIND_ASSETS_FOLDER
#ifdef  FIND_ASSETS_FOLDER

// On startup, set the current working directory to
// the 'assets' folder, to match Android behavior.

#if defined(WIN32) || defined(__LINUX__)
#ifdef WIN32
  #include <direct.h>
  #define getcwd _getcwd
  #define chdir _chdir
#else
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
        LOGI("Current Dir : %s\n", getcwd(buf, 4096));
        if(err){LOGE("Failed to change CWD to assets folder.\n"); abort();}
    }
} InitDir;
#endif

#endif //FIND_ASSETS_FOLDER


//--------------------------------------------------

