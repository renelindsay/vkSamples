#pragma warning(disable:4996)  // for fopen

#include "Window.h"
//#include "vkWindow.h"  // requires vkUtils
//#include "CInstance.h" // requires vkUtils

const char *type[] {"up  ", "down", "move"};  // Action types for mouse, keyboard and touch-screen.

//-- EVENT HANDLERS --
//class MainWindow : public vkWindow {  // With Vulkan (requires vkUtils)
class MainWindow : public CWindow {  // Without Vulkan
    void OnMouseEvent (eAction action, int16_t x, int16_t y, uint8_t btn) { printf("%s %d x %d Btn:%d\n", type[action], x, y, btn); }
    void OnTouchEvent (eAction action, float x, float y, uint8_t id) { printf("Touch: %s %f x %f id:%d\n", type[action], x, y, id); }
    void OnKeyEvent   (eAction action, eKeycode keycode) { printf("Key: %s keycode:%d\n", type[action], keycode); }
    void OnTextEvent  (const char *str) { printf("Text: %s\n", str); }
    void OnMoveEvent  (int16_t x, int16_t y) { printf("Window Move: x=%d y=%d\n", x, y); }
    void OnFocusEvent (bool hasFocus) { printf("Focus: %s\n", hasFocus ? "True" : "False"); }
    void OnResizeEvent(uint16_t width, uint16_t height) { printf("Window Resize: width=%4d height=%4d\n", width, height); }
    void OnCloseEvent () { printf("Window Closing.\n"); }
};

int main(int argc, char *argv[]) {
    setvbuf(stdout, 0, _IONBF, 0);  // Prevent printf buffering in QtCreator

    //-------------------------Print text file--------------------------
    // Test fopen, and printf to ensure consistent behavior on Windows, Linux and Android.
    // On Android, the assets folder gets included in the APK file, and 
    // fopen uses the Android AssetManager to read from the assets folder.
    // printf prints to the desktop terminal, or the Android logcat.

    FILE* file = fopen("ReadMe.txt", "r"); 
    if(!file) printf(" File not found. ReadMe.txt\n");
    if(file) { 
        char line[1024] {}; 
        while(fgets(line, 1024, file)) { 
            printf("%s", line);
        } 
        fclose(file); 
    } 
    //------------------------------------------------------------------- 

    uint32_t image[256*256] = {};

    //CInstance instance(true);                                 // Create a Vulkan Instance
    //instance.DebugReport.SetFlags(14);                        // Select validation-message types (see: VkDebugReportFlagsEXT)

    MainWindow window;                                        // Create a window
    window.SetTitle("Window : 01_KeyEvents");                 // Set the window title
    window.SetWinSize(640, 480);                              // Set the window size (Desktop)
    window.SetWinPos(0, 0);                                   // Set the window position to top-left
    window.ShowKeyboard(true);                                // Show soft-keyboard (Android)

    window.ShowImage(image,256,256);

    //VkSurfaceKHR surface = window.CreateVkSurface(instance);  // Create the Vulkan surface (requires vkUtils)

    while(window.PollEvents()) {                              // Main event loop, runs until window is closed.
        bool key_pressed = window.GetKeyState(KEY_LeftShift);
        if (key_pressed) printf("LEFT SHIFT PRESSED\r");


        for(int y=0;y<256;++y) for(int x=0;x<256;++x) {
            uint32_t* pix = &image[x + y*256];
            *pix = (x<<16) + (y<<8) + (rand()&255);
        }
        if (!key_pressed) window.ShowImage(image,256,256);
    }
    return 0;
}
