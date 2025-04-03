//#define VK_USE_PLATFORM_XCB_KHR
//#define WINDOW_IMPLEMENTATION

//============================XCB===============================
#ifdef VK_USE_PLATFORM_XCB_KHR

#ifndef WINDOW_XCB
#define WINDOW_XCB

#define ENABLE_MULTITOUCH
#define ENABLE_XCB_IMAGE   // requires libxcb-image0-dev
#define ENABLE_XCB_CURSOR  // requires libxcb-cursor-dev
#define ENABLE_GAMEPAD     // requires libevdev-dev

//-------------------------------------------------
#include "WindowBase.h"
//#include <xcb/xcb.h>            // XCB only
//#include <X11/Xlib.h>           // XLib only
#include <X11/Xlib-xcb.h>         // Xlib + XCB
#include <xkbcommon/xkbcommon.h>  // Keyboard
#include <X11/Xresource.h>        // DPI scale
#include <stdlib.h>               // atof
#include <assert.h>
#ifdef ENABLE_XCB_IMAGE
#include <xcb/xcb_image.h>        // ShowImage  libxcb-image0-dev
#endif
#ifdef ENABLE_XCB_CURSOR
#include <xcb/xcb_cursor.h>       // mouse cursor icons
#endif
#ifdef ENABLE_GAMEPAD
#include <libevdev/libevdev.h>    // libevdev-dev
#include <fcntl.h>                // gamepad open
#include <unistd.h>               // gamepad read
#include <sys/inotify.h>          // gamepad inotify
#include <dirent.h>               // For scanning the /dev/input/ directory

#include <sys/ioctl.h>
#include <linux/input.h>

#endif
//-------------------------------------------------

#ifdef ENABLE_MULTITOUCH
#include <X11/extensions/XInput2.h>  // MultiTouch
typedef uint16_t xcb_input_device_id_t;
typedef uint32_t xcb_input_fp1616_t;
// clang-format off
typedef struct xcb_input_touch_begin_event_t {  // from xinput.h in XCB 1.12 (current version is 1.11)
    uint8_t                   response_type;
    uint8_t                   extension;
    uint16_t                  sequence;
    uint32_t                  length;
    uint16_t                  event_type;
    xcb_input_device_id_t     deviceid;
    xcb_timestamp_t           time;
    uint32_t                  detail;
    xcb_window_t              root;
    xcb_window_t              event;
    xcb_window_t              child;
    uint32_t                  full_sequence;
    xcb_input_fp1616_t        root_x;
    xcb_input_fp1616_t        root_y;
    xcb_input_fp1616_t        event_x;
    xcb_input_fp1616_t        event_y;
    uint16_t                  buttons_len;
    uint16_t                  valuators_len;
    xcb_input_device_id_t     sourceid;
    // uint8_t                   pad0[2];
    // uint32_t                  flags;
    // xcb_input_modifier_info_t mods;
    // xcb_input_group_info_t    group;
} xcb_input_touch_begin_event_t;
#endif

// clang-format off
// Convert native EVDEV key-code to cross-platform USB HID code.
const unsigned char EVDEV_TO_HID[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0, 41, 30, 31, 32, 33, 34, 35,
 36, 37, 38, 39, 45, 46, 42, 43, 20, 26,  8, 21, 23, 28, 24, 12,
 18, 19, 47, 48, 40,224,  4, 22,  7,  9, 10, 11, 13, 14, 15, 51,
 52, 53,225, 49, 29, 27,  6, 25,  5, 17, 16, 54, 55, 56,229, 85,
226, 44, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 83, 71, 95,
 96, 97, 86, 92, 93, 94, 87, 89, 90, 91, 98, 99,  0,  0,100, 68,
 69,  0,  0,  0,  0,  0,  0,  0, 88,228, 84, 70,230,  0, 74, 82,
 75, 80, 79, 77, 81, 78, 73, 76,  0,127,128,129,  0,103,  0, 72,
  0,  0,  0,  0,  0,227,231,118,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,104,
105,106,107,108,109,110,111,112,113,114,115,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// clang-format on
//=============================XCB==============================
class Window_xcb : public WindowBase {
    Display* display;                  // for XLib
    xcb_connection_t* xcb_connection;  // for XCB
    xcb_screen_t* xcb_screen;
    xcb_window_t xcb_window;
    xcb_intern_atom_reply_t* atom_wm_delete_window;
    //----xcb_image ----
    xcb_gcontext_t gc=0;
    xcb_pixmap_t   pixmap=0;
    //------------------
    //---xkb Keyboard---
    xkb_context* k_ctx;  // context for xkbcommon keyboard input
    xkb_keymap* k_keymap;
    xkb_state* k_state;
    //------------------
    //---Touch Device---
    CMTouch MTouch;
    int xi_opcode;  // 131
    int xi_devid;   // 2
    //------------------
    //----- Cursor -----
#ifdef ENABLE_XCB_CURSOR
    xcb_cursor_context_t *cursor_ctx;
    xcb_cursor_t cursors[12];
#endif
    //------------------
    //---- Gamepad ----
#ifdef ENABLE_GAMEPAD
    int inotify_fd = -1;          // gamepad inotify descriptor
    int watch_fd   = -1;          // gamepad watch descriptor
    struct Evdev {                // gamepad handle and axis ranges
        int fd = -1;
        libevdev* dev = nullptr;
        char name[256] = {};      // gamepad model name
        char path[256] = {};      // eg. /dev/input/event240
        struct Limits {
            int min =0;           // Axis range min value
            int max =0;           // Axis range max value
            int fuzz=0;           // Noise level
            int flat=0;           // Dead zone
            int prev=0;           // previous value
        }axis[11];
        bool useDPad=false;       // Sony Dualshock hat emits dpad button events instead of HAT axis
        bool SwapAB=false;        // Nintendo gamepads swap A and B buttons
    } evdev[MAX_GAMEPADS];

    void DetectGamepads();                           // Detect connected gamepads
    bool ConnectGamepad(const char* path);           // eg. /dev/input/event240
    void DisconnectGamepad(uint8_t id);              // Disconnect gamepad by id (0-3)
    void SetGamepadLEDs(uint8_t id, uint8_t state);  // pad-id(0-3), led-bitmask(0-15)
    void ReadGamepadEvents();                        // Process all gamepad events
#endif
    //------------------

    bool InitTouch();                                        // Returns false if no touch-device was found.
    EventType TranslateEvent(xcb_generic_event_t* x_event);  // Convert x_event to WSIWindow event
    void Create(const char* title="Window", uint width=640, uint height=480);

  public:
    void SetTitle(const char* title);
    void SetWinPos (uint x, uint y);
    void SetWinSize(uint w, uint h);
    //void CreateSurface(VkInstance instance);

    Window_xcb() {Create();}
    Window_xcb(const char* title, uint width, uint height);
    virtual ~Window_xcb();
    EventType GetEvent(bool wait_for_event = false);
    //bool CanPresent(VkPhysicalDevice phy, uint32_t queue_family);  // check if this window can present this queue type
    const void* GetNativeHandle() const {return &xcb_connection;}
    float GetDisplayScale();
    void ShowImage(uint32_t* buf, uint32_t width, uint32_t height);
    void SetCursor(eCursor id);
};
//==============================================================
#endif

//#define WINDOW_IMPLEMENTATION
#ifdef WINDOW_IMPLEMENTATION

//=======================XCB IMPLEMENTATION=====================

Window_xcb::Window_xcb(const char* title, uint width, uint height) {
    Create(title, width, height);
}

void Window_xcb::Create(const char* title, uint width, uint height) {
    shape.width  = width;
    shape.height = height;
    m_running      = true;

    LOGI("Creating XCB-Window...\n");

    // --Init Connection-- XCB only
    // int scr;
    // xcb_connection = xcb_connect(NULL, &scr);
    // assert(xcb_connection && "XCB failed to connect to the X server.");
    // const xcb_setup_t*   setup = xcb_get_setup(xcb_connection);
    // xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    // while(scr-- > 0) xcb_screen_next(&iter);
    // xcb_screen = iter.data;
    //-------------------

    //----XLib + XCB----
    XInitThreads(); // Required by Vulkan, when using XLib. (Vulkan spec section: 30.2.6 Xlib Platform)
    display = XOpenDisplay(NULL);                 assert(display && "Failed to open Display");        // for XLIB functions
    xcb_connection = XGetXCBConnection(display);  assert(display && "Failed to open XCB connection"); // for XCB functions
    const xcb_setup_t* setup = xcb_get_setup(xcb_connection);
    setup = xcb_get_setup(xcb_connection);
    xcb_screen = (xcb_setup_roots_iterator(setup)).data;
    XSetEventQueueOwner(display, XCBOwnsEventQueue);
    //------------------

    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[2];
    value_list[0] = xcb_screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_PRESS |          // 1
                    XCB_EVENT_MASK_KEY_RELEASE |        // 2
                    XCB_EVENT_MASK_BUTTON_PRESS |       // 4
                    XCB_EVENT_MASK_BUTTON_RELEASE |     // 8
                    XCB_EVENT_MASK_POINTER_MOTION |     // 64       motion with no mouse button held
                    XCB_EVENT_MASK_BUTTON_MOTION  |     // 8192     motion with one or more mouse buttons held
                  //XCB_EVENT_MASK_KEYMAP_STATE |       // 16384
                  //XCB_EVENT_MASK_EXPOSURE |           // 32768    Make ShowImage persistant
                  //XCB_EVENT_MASK_VISIBILITY_CHANGE,   // 65536,
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY |   // 131072   Window move/resize events
                  //XCB_EVENT_MASK_RESIZE_REDIRECT |    // 262144
                    XCB_EVENT_MASK_FOCUS_CHANGE;        // 2097152  Window focus

#ifdef ENABLE_XCB_IMAGE
    gc     = xcb_generate_id(xcb_connection);
    pixmap = xcb_generate_id(xcb_connection);
#endif

    xcb_window = xcb_generate_id(xcb_connection);
    xcb_create_window(xcb_connection, XCB_COPY_FROM_PARENT, xcb_window, xcb_screen->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, xcb_screen->root_visual, value_mask, value_list);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(xcb_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply  = xcb_intern_atom_reply(xcb_connection, cookie, 0);

    //--
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(xcb_connection, 0, 16, "WM_DELETE_WINDOW");
    atom_wm_delete_window            = xcb_intern_atom_reply(xcb_connection, cookie2, 0);
    xcb_change_property(xcb_connection, XCB_PROP_MODE_REPLACE, xcb_window, (*reply).atom, 4, 32, 1, &(*atom_wm_delete_window).atom);
    //--
    XFree(reply);

    //---Keyboard input---
    k_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    // xkb_rule_names names = {NULL,"pc105","is","dvorak","terminate:ctrl_alt_bksp"};
    // keymap = xkb_keymap_new_from_names(k_ctx, &names,XKB_KEYMAP_COMPILE_NO_FLAGS);
    k_keymap = xkb_keymap_new_from_names(k_ctx, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);  // use current keyboard settings
    k_state  = xkb_state_new(k_keymap);
    //--------------------
    InitTouch();
    //--------------------
    SetTitle(title);
    eventFIFO.push(ResizeEvent(width, height));  // ResizeEvent BEFORE focus, for consistency with win32 and android

    //---- Mouse Cursor ----
#ifdef ENABLE_XCB_CURSOR
    xcb_cursor_context_new(xcb_connection, xcb_setup_roots_iterator(setup).data, &cursor_ctx);
    cursors[eCursor::eArrow]      = xcb_cursor_load_cursor(cursor_ctx, "left_ptr");
    cursors[eCursor::eCaret]      = xcb_cursor_load_cursor(cursor_ctx, "xterm");
    cursors[eCursor::eResizeAll]  = xcb_cursor_load_cursor(cursor_ctx, "fleur");
    cursors[eCursor::eResizeNS]   = xcb_cursor_load_cursor(cursor_ctx, "sb_v_double_arrow");
    cursors[eCursor::eResizeEW]   = xcb_cursor_load_cursor(cursor_ctx, "sb_h_double_arrow");
    cursors[eCursor::eResizeNESW] = xcb_cursor_load_cursor(cursor_ctx, "top_right_corner");
    cursors[eCursor::eResizeNWSE] = xcb_cursor_load_cursor(cursor_ctx, "top_left_corner");
    cursors[eCursor::eHand]       = xcb_cursor_load_cursor(cursor_ctx, "hand2");
    cursors[eCursor::eWait]       = xcb_cursor_load_cursor(cursor_ctx, "wait");
    cursors[eCursor::eProgress]   = xcb_cursor_load_cursor(cursor_ctx, "progress");
    cursors[eCursor::eNotAllowed] = xcb_cursor_load_cursor(cursor_ctx, "circle");
#endif
    //----------------------

    //----Map the window----
    xcb_map_window(xcb_connection, xcb_window);
    xcb_flush(xcb_connection);

    // Wait for the window to be mapped (so resize works correctly)
    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(xcb_connection))) {
        bool mapped = ((event->response_type & ~0x80) == XCB_MAP_NOTIFY);
        free(event);
        if(mapped) break;
    }
    //---------------------- 
}

Window_xcb::~Window_xcb() {

#ifdef ENABLE_XCB_IMAGE
    if(gc)     xcb_free_gc    (xcb_connection, gc);
    if(pixmap) xcb_free_pixmap(xcb_connection, pixmap);
#endif
#ifdef ENABLE_XCB_CURSOR
    int cnt = sizeof(cursors) / sizeof(cursors[0]);
    for(int i=0; i<cnt; ++i) xcb_free_cursor(xcb_connection, cursors[i]);
    xcb_cursor_context_free(cursor_ctx);
#endif
#ifdef ENABLE_GAMEPAD
    for (int i = 0; i < MAX_GAMEPADS; ++i) { DisconnectGamepad(i); }
    if (watch_fd   != -1) { inotify_rm_watch(inotify_fd, watch_fd); watch_fd=-1;}
    if (inotify_fd != -1) { close(inotify_fd); inotify_fd=-1; }
#endif
    XFree(atom_wm_delete_window);
    xcb_disconnect(xcb_connection);
    XFree(k_ctx);  // xkb keyboard
}

void Window_xcb::SetTitle(const char* title) {
    xcb_change_property(xcb_connection, XCB_PROP_MODE_REPLACE, xcb_window, XCB_ATOM_WM_NAME,  // set window title
                        XCB_ATOM_STRING, 8, strlen(title), title);
    xcb_change_property(xcb_connection, XCB_PROP_MODE_REPLACE, xcb_window, XCB_ATOM_WM_ICON_NAME,  // set icon title
                        XCB_ATOM_STRING, 8, strlen(title), title);
    xcb_map_window(xcb_connection, xcb_window);
    xcb_flush(xcb_connection);
}

void Window_xcb::SetWinPos(uint x, uint y) {
    float scale = GetDisplayScale();
    x = (uint) (x * scale);
    y = (uint) (y * scale);
    uint values[] = {x, y};
    xcb_configure_window(xcb_connection, xcb_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
    xcb_flush(xcb_connection);
}

void Window_xcb::SetWinSize(uint w, uint h) {
    float scale = GetDisplayScale();
    w = (uint) (w * scale);
    h = (uint) (h * scale);
    uint values[] = {w, h};
    xcb_configure_window(xcb_connection, xcb_window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(xcb_connection);
}
/*
void Window_xcb::CreateSurface(VkInstance instance) {
    if (surface) return;
    this->instance = instance;
    VkXcbSurfaceCreateInfoKHR xcb_createInfo;
    xcb_createInfo.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcb_createInfo.pNext      = NULL;
    xcb_createInfo.flags      = 0;
    xcb_createInfo.connection = xcb_connection;
    xcb_createInfo.window     = xcb_window;
    VKERRCHECK(vkCreateXcbSurfaceKHR(instance, &xcb_createInfo, NULL, &surface));
    LOGI("Vulkan Surface created\n");
}
*/
//---------------------------------------------------------------------------
bool Window_xcb::InitTouch() {
#ifdef ENABLE_MULTITOUCH
    int ev, err;
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &ev, &err)) {
        LOGW("XInputExtension not available.\n");
        return false;
    }

    // check the version of XInput
    int major = 2;
    int minor = 3;
    if (XIQueryVersion(display, &major, &minor) != Success) {
        LOGW("No XI2 support. (%d.%d only)\n", major, minor);
        return false;
    }

    {  // select device
        int cnt;
        XIDeviceInfo* di = XIQueryDevice(display, XIAllDevices, &cnt);
        for (int i = 0; i < cnt; ++i) {
            XIDeviceInfo* dev = &di[i];
            for (int j = 0; j < dev->num_classes; ++j) {
                XITouchClassInfo* tcinfo = (XITouchClassInfo*)(dev->classes[j]);
                if (tcinfo->type != XITouchClass) {
                    xi_devid = dev->deviceid;
                    goto endloop;
                }
            }
        }
    endloop:
        XIFreeDeviceInfo(di);
    }

    {  // select which events to listen to
        unsigned char buf[3] = {};
        XIEventMask mask     = {};
        mask.deviceid        = xi_devid;
        mask.mask_len        = XIMaskLen(XI_TouchEnd);
        mask.mask            = buf;
        XISetMask(mask.mask, XI_TouchBegin);
        XISetMask(mask.mask, XI_TouchUpdate);
        XISetMask(mask.mask, XI_TouchEnd);
        XISelectEvents(display, xcb_window, &mask, 1);
    }
    return true;
#else
    return false;
#endif
}
//---------------------------------------------------------------------------

EventType Window_xcb::TranslateEvent(xcb_generic_event_t* x_event) {
    static char buf[4] = {};                                            // store char for text event
    xcb_button_press_event_t& e = *(xcb_button_press_event_t*)x_event;  // xcb_motion_notify_event_t
    int16_t mx = e.event_x;
    int16_t my = e.event_y;
    uint8_t btn= e.detail;
    uint8_t bestBtn = GetBtnState(1) ? 1 : GetBtnState(2) ? 2 : GetBtnState(3) ? 3 : 0;  // If multiple buttons pressed, pick left one.
    switch(x_event->response_type & ~0x80) {
        case XCB_MOTION_NOTIFY : return MouseEvent(eMOVE, mx, my, bestBtn); // mouse move
        case XCB_BUTTON_PRESS  : return MouseEvent(eDOWN, mx, my, btn);     // mouse btn press
        case XCB_BUTTON_RELEASE: return MouseEvent(eUP  , mx, my, btn);     // mouse btn release
        case XCB_KEY_PRESS:{
            //printf("btn %d", btn);
            uint8_t keycode = EVDEV_TO_HID[btn];                    // On Stratus XL gamepad, 2 buttons trigger keyboard events
            if(!keycode) {                                          // remap key to gamepad btn
                if(btn==166) return GPadButton(0, eBTN_SELECT, 1);  // Steelseries Stratus XL: select button (XF86Back)
                if(btn==180) return GPadButton(0, eBTN_MODE, 1);    // Steelseries Stratus XL: mode button   (XF86HomePage)
            }
            xkb_state_key_get_utf8(k_state,btn,buf,sizeof(buf));
            xkb_state_update_key(k_state,btn,XKB_KEY_DOWN);
            if(buf[0]) eventFIFO.push(TextEvent(buf));              // text typed event (store in FIFO for next run)
            return KeyEvent(eDOWN, keycode);                        // key pressed event
        }
        case XCB_KEY_RELEASE: {
            xkb_state_update_key(k_state, btn, XKB_KEY_UP);
            uint8_t keycode = EVDEV_TO_HID[btn];
            if(!keycode) {                                          // remap key to gamepad btn
                if(btn==166) return GPadButton(0, eBTN_SELECT, 0);  // Steelseries Stratus XL
                if(btn==180) return GPadButton(0, eBTN_MODE, 0);    // Steelseries Stratus XL
            }
            return KeyEvent(eUP, keycode);                          // key released event
        }
        case XCB_CLIENT_MESSAGE: {                                  // window close event
            if ((*(xcb_client_message_event_t*)x_event).data.data32[0] == (*atom_wm_delete_window).atom) {
                LOGI("Closing Window\n");
                return CloseEvent();
            }
            break;
        }
        case XCB_CONFIGURE_NOTIFY: {                                // Window Reshape (move or resize)
            auto& e = *(xcb_configure_notify_event_t*)x_event;
            //bool se = (e.response_type & 128);                    // True if message was sent with "SendEvent"
            if (e.width != shape.width || e.height != shape.height) return ResizeEvent(e.width, e.height); // window resized
            else if (e.x != shape.x || e.y != shape.y)              return MoveEvent(e.x, e.y);            // window moved
            break;
        }
        case XCB_FOCUS_IN  : if (!m_has_focus) return FocusEvent(true);   // window gained focus
        case XCB_FOCUS_OUT : if ( m_has_focus) return FocusEvent(false);  // window lost focus

        case XCB_GE_GENERIC: {                                            // Multi touch screen events
#ifdef ENABLE_MULTITOUCH
            xcb_input_touch_begin_event_t& te = *(xcb_input_touch_begin_event_t*)x_event;
            if(te.extension == xi_opcode) {  // check if this event is from the touch device
                float x = te.event_x / 65536.f;
                float y = te.event_y / 65536.f;
                uint id = te.detail;

                switch(te.event_type){
                    case XI_TouchBegin : return MTouch.Event_by_ID(eDOWN, x, y,  0, id); // touch down event
                    case XI_TouchUpdate: return MTouch.Event_by_ID(eMOVE, x, y, id, id); // touch move event
                    case XI_TouchEnd   : return MTouch.Event_by_ID(eUP  , x, y, id,  0); // touch up event
                    default : break;
                }
            }
#endif
            return {EventType::UNKNOWN};
        }  // XCB_GE_GENERIC

#ifdef ENABLE_XCB_IMAGE
        case XCB_EXPOSE: {  // for ShowImage
             xcb_expose_event_t& e = *(xcb_expose_event_t*)x_event;
             xcb_copy_area(xcb_connection,pixmap,xcb_window,gc,e.x,e.y,e.x,e.y,e.width,e.height);
             xcb_flush(xcb_connection);
        }break;
#endif
        default:
            // printf("EVENT: %d\n",(x_event->response_type & ~0x80));  //get event numerical value
            break;
    }  // switch
    return {EventType::NONE};
}

EventType Window_xcb::GetEvent(bool wait_for_event) {
    ReadGamepadEvents();
    if (!eventFIFO.isEmpty()) return *eventFIFO.pop();  // pop message from message queue buffer
    xcb_generic_event_t* x_event;
    if (wait_for_event) x_event = xcb_wait_for_event(xcb_connection);  // Blocking mode
    else                x_event = xcb_poll_for_event(xcb_connection);  // Non-blocking mode
    while(x_event) {
        EventType event = TranslateEvent(x_event);
        XFree(x_event);
        if (event.tag == EventType::UNKNOWN) {
            x_event = xcb_poll_for_event(xcb_connection);  // Discard unknown events (Intel Mesa drivers spams event 35)
        } else return event;
    }
    ReadGamepadEvents();
    return {EventType::NONE};
}

float Window_xcb::GetDisplayScale() {
    float dpi = 0.0;
    XrmValue value;
    char *type = NULL;
    char *resourceString = XResourceManagerString(display);
    XrmInitialize();
    XrmDatabase db = XrmGetStringDatabase(resourceString);
    if (resourceString) {
        //printf("Entire DB:\n%s\n", resourceString);
        if (XrmGetResource(db, "Xft.dpi", "String", &type, &value) == True) {
            if (value.addr) {
                dpi = atof(value.addr);
            }
        }
    }
    //printf("Monitor DPI: %f\n", dpi);
    m_display_scale = dpi / 96.f;
    return m_display_scale;
}

#ifdef ENABLE_XCB_IMAGE
void Window_xcb::ShowImage(uint32_t* buf, uint32_t width, uint32_t height) {  // Shows image for 1 frame.
    xcb_connection_t* c = xcb_connection;
    xcb_image_format_t format = XCB_IMAGE_FORMAT_Z_PIXMAP;
    int depth  = xcb_screen->root_depth;

    xcb_create_pixmap(c,depth,pixmap,xcb_window,width,height);
    xcb_create_gc(c,gc,pixmap,0,NULL);

    xcb_image_t* image = xcb_image_create_native(c,width,height,format,depth,0,0,(uint8_t*)buf);
    xcb_image_put(c, pixmap, gc, image, 0, 0, 0);
    xcb_image_destroy(image);
    xcb_copy_area(c,pixmap,xcb_window,gc,0,0,0,0,width,height);
    xcb_flush(xcb_connection);

    xcb_free_gc(c, gc);
    xcb_free_pixmap(c, pixmap);
}
#endif


void Window_xcb::SetCursor(eCursor id) {
#ifdef ENABLE_XCB_CURSOR
    xcb_change_window_attributes(xcb_connection, xcb_window, XCB_CW_CURSOR, &cursors[id]);
#endif
}

//---Gamepad---
#ifdef ENABLE_GAMEPAD
void Window_xcb::DetectGamepads() {
    if (inotify_fd == -1) {
        // Scan for already connected gamepads
        DIR* dir = opendir("/dev/input/");
        if(dir) {
            dirent* entry;
            while ((entry = readdir(dir))) {
                if (strncmp(entry->d_name, "event", 5) == 0) { // Look for "event#"
                    char path[512]{};
                    snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
                    ConnectGamepad(path);
                }
            }
        }
        // Watch for new device connections using inotify
        inotify_fd = inotify_init1(IN_NONBLOCK);
        if (inotify_fd < 0) { perror("inotify_init1"); return; }
        watch_fd = inotify_add_watch(inotify_fd, "/dev/input/", IN_CREATE);
    }
    // Process inotify events
    char buffer[1024];
    int len = read(inotify_fd, buffer, sizeof(buffer));
    if (len > 0) {
        for (char* ptr = buffer; ptr < buffer + len;) {
            struct inotify_event* event = (struct inotify_event*)ptr;
            ptr += sizeof(struct inotify_event) + event->len;
            if (event->mask & (IN_CREATE)) {
                if (strncmp(event->name, "event", 5) == 0) {  // Look for "eventX"
                    char path[256]{};
                    snprintf(path, sizeof(path), "/dev/input/%s", event->name);
                    usleep(60000);         // Allow time for the device to appear
                    ConnectGamepad(path);  // Try to connect as Gamepad
                }
            }
        }
    }
}

void setGamepadLED(const char* devicePath, int LED_id, bool state) {
    int fd = open(devicePath, O_WRONLY);
    if(fd<0) return;
    struct input_event event;
    event.type = EV_LED;
    event.code = LED_id;
    event.value = state ? 1 : 0;
    write(fd, &event, sizeof(event));
    close(fd);
}

void Window_xcb::SetGamepadLEDs(uint8_t id, uint8_t state) {
    setGamepadLED(evdev[id].path, 0, !!(state&1));
    setGamepadLED(evdev[id].path, 1, !!(state&2));
    setGamepadLED(evdev[id].path, 2, !!(state&4));
    setGamepadLED(evdev[id].path, 3, !!(state&8));
}

bool Window_xcb::ConnectGamepad(const char* path) {
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if(fd<0) return false;
    struct libevdev* dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) >= 0) {
        if(libevdev_has_event_type(dev, EV_ABS)
        && libevdev_has_event_type(dev, EV_KEY)
        && libevdev_has_event_code(dev, EV_KEY, BTN_SOUTH)
        && libevdev_has_event_code(dev, EV_KEY, BTN_NORTH)
        && libevdev_has_event_code(dev, EV_KEY, BTN_EAST )
        && libevdev_has_event_code(dev, EV_KEY, BTN_WEST )
        && libevdev_has_event_code(dev, EV_ABS, ABS_X)
        && libevdev_has_event_code(dev, EV_ABS, ABS_Y)) {  // make sure this is a gamepad
            for (int i = 0; i < MAX_GAMEPADS; ++i) {  // find a free slot
                Evdev& ev = evdev[i];
                ev.useDPad=(!libevdev_has_event_code(dev, EV_ABS, ABS_HAT0X));  // If HAT is missing, use DPad. (for Sony Dualshock)
                if(ev.fd < 0) {
                    ev.fd = fd;
                    ev.dev= dev;
                    strncpy(ev.name, libevdev_get_name(dev), sizeof(ev.name)-1);               
                    strncpy(ev.path, path, sizeof(ev.path)-1);
                    //printf("Gamepad %d found: %s at %s\n", i, ev.name, path);

                    ev.SwapAB=false;  // Nintendo swaps the A and B buttons
                    //if (ev.name && (strstr(ev.name, "Nintendo") || strstr(ev.name, "Switch")))         {ev.SwapAB = true;}
                    //if (ev.name && (strstr(ev.name, "Joy-Con")  || strstr(ev.name, "Pro Controller"))) {ev.SwapAB = true;}
                    //printf("NINTENDO\n");

                    for(uint axis=0; axis<11; ++axis) {
                        auto& ax = ev.axis[axis];
                        ax.min  = libevdev_get_abs_minimum(dev,axis); // axis min value
                        ax.max  = libevdev_get_abs_maximum(dev,axis); // axis max value
                        ax.fuzz = libevdev_get_abs_fuzz   (dev,axis); // noise level
                        ax.flat = libevdev_get_abs_flat   (dev,axis); // dead zone
                        printf("axis:%d min=%d max=%d fuzz=%d flat=%d\n", axis, ax.min, ax.max, ax.fuzz, ax.flat);
                    }
                    SetGamepadLEDs(i,1<<i);  // Set Gamepad LEDs to indicate which slot its in.
                    eventFIFO.push(GPadConnect(i, true));
                    return true;
                }
            }
        } else {libevdev_free(dev); close(fd);}
    } else close(fd);
    return false;
}

void Window_xcb::DisconnectGamepad(uint8_t id) {
    Evdev& ev = evdev[id];
    if(ev.fd==-1) return;
    eventFIFO.push(GPadConnect(id, false));
    //SetGamepadLEDs(id, 0);  // Does not restore blinking :(
    libevdev_free(ev.dev);
    close(ev.fd);
    ev.fd = -1;
    ev.dev = 0;
    ev.name[0] = '\0';
    ev.path[0] = '\0';
    memset(ev.axis, 0, sizeof(ev.axis));
    //printf("Gamepad %d disconnected.\n", id);
}

int Gamepad_keymap(uint keycode) {
    switch (keycode) {
        case BTN_A:      return eBTN_A;      // 304
        case BTN_B:      return eBTN_B;      // 305
        case BTN_X:      return eBTN_X;      // 307
        case BTN_Y:      return eBTN_Y;      // 308
        case BTN_TL:     return eBTN_TL;     // 310
        case BTN_TR:     return eBTN_TR;     // 311
        case BTN_SELECT: return eBTN_SELECT; // 314
        case BTN_START:  return eBTN_START;  // 315
        case BTN_MODE:   return eBTN_MODE;   // 316
        case BTN_THUMBL: return eBTN_THUMBL; // 317
        case BTN_THUMBR: return eBTN_THUMBR; // 318

        // Sony Dualshock uses 4 DPad buttons, instead of HAT0X/Y axes:
        case BTN_DPAD_UP   : return eDPAD_UP;    //0x220  for Dualshock
        case BTN_DPAD_DOWN : return eDPAD_DOWN;  //0x221  for Dualshock
        case BTN_DPAD_LEFT : return eDPAD_LEFT;  //0x222  for Dualshock
        case BTN_DPAD_RIGHT: return eDPAD_RIGHT; //0x223  for Dualshock
        default: return 0;
    }
}

int Gamepad_axismap(uint axiscode) {
    switch(axiscode) {
        case ABS_X    : return eAXIS_LX;  //  0
        case ABS_Y    : return eAXIS_LY;  //  1
        case ABS_RX   : return eAXIS_RX;  //  3
        case ABS_RY   : return eAXIS_RY;  //  4
        case ABS_Z    : return eAXIS_RX;  //  2  // alternative mapping
        case ABS_RZ   : return eAXIS_RY;  //  5  // alternative mapping
        case ABS_GAS  : return eAXIS_TR;  //  9
        case ABS_BRAKE: return eAXIS_TL;  // 10
        default: return 0;
    }
}

void Window_xcb::ReadGamepadEvents() {
    DetectGamepads();
    for (int i=0; i<MAX_GAMEPADS; ++i) {
        Evdev&   ev  = evdev[i];
        Gamepad& pad = gamepad[i];
        if (!pad.active) continue;

        auto Hat = [&](int val, int btnNeg, int btnPos) { // convert hat axis values to button events
            //if(!ev.hasHat) return;
            if((val!=-1) && ( pad.buttons[btnNeg])) eventFIFO.push(GPadButton(i, btnNeg, 0));
            if((val!= 1) && ( pad.buttons[btnPos])) eventFIFO.push(GPadButton(i, btnPos, 0));
            if((val==-1) && (!pad.buttons[btnNeg])) eventFIFO.push(GPadButton(i, btnNeg, 1));
            if((val== 1) && (!pad.buttons[btnPos])) eventFIFO.push(GPadButton(i, btnPos, 1));
        };

        auto isFuzz = [](int value, Evdev::Limits& a) -> bool {
            int delta = abs(a.prev - value);
            if(delta<a.fuzz) return true;
            a.prev = value;
            return false;
        };

        auto Trigger = [](int value, Evdev::Limits& a) -> float { // Apply dead-zone, normalize
            int val   = std::max(value-a.min-a.flat,0);
            int range = std::max(a.max-a.min-a.flat,1);
            return val / (float)range;
        };

        auto Thumb = [](int value, Evdev::Limits& a) -> float { // Apply dead-zone, normalize
            int center = (a.min + a.max) / 2;
            int centered = value - center;
            int sign  = centered<0 ? -1:1;
            int val   = std::max(std::abs(centered) - a.flat, 0);
            int range = a.max - center - a.flat;
            return (val / (float)range) * sign;
        };

        int rc=0;
        struct input_event event;
        while ((rc=libevdev_next_event(ev.dev, LIBEVDEV_READ_FLAG_NORMAL, &event)) == 0) {
            if (event.type == EV_KEY) {  // Button press/release
                int btn = Gamepad_keymap(event.code);
                if(ev.SwapAB && btn<5){static int ABtoBA[]={0,2,1,4,3}; btn=ABtoBA[btn];}  // Swap A/B and X/Y (Nintendo)
                if((event.code < BTN_DPAD_UP) || ev.useDPad)                               // Only use DPad if missing HAT (Sony)
                eventFIFO.push(GPadButton(i, btn, event.value));
            }
            else if (event.type == EV_ABS) {  // Analog axes and hat buttons
                if(event.code == ABS_HAT0X) {Hat(event.value, eDPAD_LEFT, eDPAD_RIGHT); continue;}
                if(event.code == ABS_HAT0Y) {Hat(event.value, eDPAD_UP,   eDPAD_DOWN);  continue;}
                if(event.code > 10) continue;

                auto& limits = ev.axis[event.code];  // min,max,fuzz,flat
                if(isFuzz(event.value, limits)) continue;  // defuzz
                bool isTrigger = (event.code==ABS_GAS || event.code==ABS_BRAKE);
                float fval = isTrigger? Trigger(event.value, limits)
                                      : Thumb  (event.value, limits);
                int axis = Gamepad_axismap(event.code);
                if(pad.axes[axis] == fval) continue;             // deadzone
                if(axis==eAXIS_LY || axis==eAXIS_RY) fval=-fval; // flip y axis
                eventFIFO.push(GPadAxis(i, axis, fval));
            }
        }
        if(rc==-ENODEV) DisconnectGamepad(i);
    }
}

/*
// eg. SetGamepadRumble(0, 20000, 0);
void Window_xcb::SetGamepadRumble(int index, uint16_t weak, uint16_t strong) {  // TODO
    if (index < 0 || index >= MAX_GAMEPADS || gamepads[index].fd < 0) return;

    struct ff_effect effect = {};
    effect.type = FF_RUMBLE;
    effect.id = -1;
    effect.u.rumble.strong_magnitude = strong;
    effect.u.rumble.weak_magnitude = weak;
    if (ioctl(gamepads[index].fd, EVIOCSFF, &effect) < 0) return;

    struct input_event play = {};
    play.type = EV_FF;
    play.code = effect.id;
    play.value = 1;

    write(gamepads[index].fd, &play, sizeof(play));  // Start rumble
    usleep(500000);                                  // Let it run for 500ms
    ioctl(gamepads[index].fd, EVIOCRMFF, effect.id); // Remove the effect
}
*/

#else
    void Window_xcb::DetectGamepads(){};
    bool Window_xcb::ConnectGamepad(const char* path){};
    void Window_xcb::DisconnectGamepad(uint8_t id){};
    void Window_xcb::SetGamepadLEDs(uint8_t id, uint8_t state){};
    void Window_xcb::ReadGamepadEvents(){};
#endif


//-------------

#endif  // WINDOW_IMPLEMENTATION

#endif  // VK_USE_PLATFORM_XCB_KHR
//==============================================================
