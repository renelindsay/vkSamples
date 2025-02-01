
//#ifdef ANDROID
//#define VK_USE_PLATFORM_ANDROID_KHR
//#endif

//==========================ANDROID=============================
#ifdef VK_USE_PLATFORM_ANDROID_KHR

#ifndef WINDOW_ANDROID
#define WINDOW_ANDROID

#include "WindowBase.h"
#include "native.h"  // for Android_App
#include <cmath>

#define repeat(COUNT) for (uint32_t i = 0; i < COUNT; ++i)
#define MIN(A,B) (((A)<(B))?(A):(B));
#define MAX(A,B) (((A)>(B))?(A):(B));

//========================================================
// clang-format off
// Convert native Android key-code to cross-platform USB HID code.
const unsigned char ANDROID_TO_HID[256] = {
  0,227,231,  0,  0,  0,  0, 39, 30, 31, 32, 33, 34, 35, 36, 37,
 38,  0,  0, 82, 81, 80, 79,  0,  0,  0,  0,  0,  0,  4,  5,  6,
  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
 23, 24, 25, 26, 27, 28, 29, 54, 55,226,230,225,229, 43, 44,  0,
  0,  0, 40,  0, 53, 45, 46, 47, 48, 49, 51, 52, 56,  0,  0,  0,
  0,  0,118,  0,  0,  0,  0,  0,  0,  0,  0,  0, 75, 78,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 57, 71,  0,  0,  0,  0, 72, 74, 77, 73,  0,  0,  0,
 24, 25,  0, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 83,
 98, 89, 90, 91, 92, 93, 94, 95, 96, 97, 84, 85, 86, 87, 99,  0,
 88,103,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};
// clang-format on
//==========================Android=============================
class Window_android : public WindowBase {
    android_app* m_app = 0;
    CMTouch MTouch;
  public:
    void SetTitle(const char* title){};  // TODO : Set window title?
    void SetWinPos (uint x, uint y){};
    void SetWinSize(uint w, uint h){};

  private:
    void Create(const char* title="", uint width=640, uint height=480) {
        shape.width  = 0;  // width;
        shape.height = 0;  // height;
        m_running    = true;
        LOGI("Creating Android-Window...\n");
        m_app = Android_App;

        //---Wait for window to be created AND gain focus---
        while (!m_has_focus) {
            int events = 0;
            struct android_poll_source* source;
            int id = ALooper_pollOnce(100, NULL, &events, (void**)&source);
            if (id == LOOPER_ID_MAIN) {
                int8_t cmd = android_app_read_cmd(m_app);
                android_app_pre_exec_cmd(m_app, cmd);
                if (m_app->onAppCmd != NULL) m_app->onAppCmd(m_app, cmd);
                if (cmd == APP_CMD_INIT_WINDOW) {
                    shape.width  = (uint16_t)ANativeWindow_getWidth (m_app->window);
                    shape.height = (uint16_t)ANativeWindow_getHeight(m_app->window);
                    eventFIFO.push(ResizeEvent(shape.width, shape.height));        // post window-resize event

                    //Get device configuration for dp scaling
                    AConfiguration* config = AConfiguration_new();
                    AConfiguration_fromAssetManager(config, m_app->activity->assetManager);
                    int32_t dpi = AConfiguration_getDensity(config);
                    m_display_scale = dpi / 160.0;
                    AConfiguration_delete(config);
                }
                if (cmd == APP_CMD_GAINED_FOCUS) eventFIFO.push(FocusEvent(true)); // post focus-event
                android_app_post_exec_cmd(m_app, cmd);
            }
        }
        ALooper_pollAll(10, NULL, NULL, NULL);  // for keyboard
        //--------------------------------------------------
    };

  public:
    Window_android(){Create();}

    Window_android(const char* title, uint width, uint height) {
        Create(title, width, height);
    }

    virtual ~Window_android(){}

    EventType GetEvent(bool wait_for_event = false) {
        EventType event    = {};
        static char buf[4] = {};                            // store char for text event
        if (!eventFIFO.isEmpty()) return *eventFIFO.pop();  // pop message from message queue buffer

        int events = 0;
        struct android_poll_source* source;
        int timeoutMillis = wait_for_event ? -1 : 0; // Blocking or non-blocking mode
        int id = ALooper_pollOnce(timeoutMillis, NULL, &events, (void**)&source);
        // ALooper_pollAll(0, NULL,&events,(void**)&source);

        // if(id>=0) printf("id=%d events=%d, source=%d",id,(int)events, source[0]);
        // if(source) source->process(app, source);

        if (id == LOOPER_ID_MAIN) {
            int8_t cmd = android_app_read_cmd(m_app);
            android_app_pre_exec_cmd(m_app, cmd);
            if (m_app->onAppCmd != NULL) m_app->onAppCmd(m_app, cmd);
            switch (cmd) {
                case APP_CMD_GAINED_FOCUS: event = FocusEvent(true);  break;
                case APP_CMD_LOST_FOCUS  : event = FocusEvent(false); break;
                default: break;
            }
            android_app_post_exec_cmd(m_app, cmd);
            return event;
        } else if (id == LOOPER_ID_INPUT) {
            AInputEvent* a_event = NULL;
            while (AInputQueue_getEvent(m_app->inputQueue, &a_event) >= 0) {
                // LOGV("New input event: type=%d\n", AInputEvent_getType(event));
                if (AInputQueue_preDispatchEvent(m_app->inputQueue, a_event)) { continue; }
                int32_t handled                        = 0;
                if (m_app->onInputEvent != NULL) handled = m_app->onInputEvent(m_app, a_event);

                int32_t type = AInputEvent_getType(a_event);
                if (type == AINPUT_EVENT_TYPE_KEY) {  // KEYBOARD
                    int32_t a_action = AKeyEvent_getAction(a_event);
                    int32_t keycode  = AKeyEvent_getKeyCode(a_event);
                    uint8_t hidcode  = ANDROID_TO_HID[keycode];
                    // printf("key action:%d keycode=%d",a_action,keycode);
                    switch (a_action) {
                    case AKEY_EVENT_ACTION_DOWN: {
                        int metaState = AKeyEvent_getMetaState(a_event);
                        int unicode   = GetUnicodeChar(AKEY_EVENT_ACTION_DOWN, keycode, metaState);
                        (int&)buf     = unicode;
                        event = KeyEvent(eDOWN, hidcode);            // key pressed event (returned on this run)
                        if (buf[0]) eventFIFO.push(TextEvent(buf));  // text typed event  (store in FIFO for next run)
                        break;
                    }
                    case AKEY_EVENT_ACTION_UP: {
                        event = KeyEvent(eUP, hidcode);              // key released event
                        break;
                    }
                    default: break;
                    }

                } else if (type == AINPUT_EVENT_TYPE_MOTION) { // TOUCH-SCREEN
                    int32_t a_action = AMotionEvent_getAction(a_event);
                    int action       = (a_action & 255); // get action-code from bottom 8 bits
                    MTouch.count     = (int)AMotionEvent_getPointerCount(a_event);
                    if (action == AMOTION_EVENT_ACTION_MOVE) {

                        if (MTouch.count>1)
                        for(uint i = 1; i<MTouch.count; ++i) {
                            uint8_t finger_id = (uint8_t)AMotionEvent_getPointerId(a_event, i);
                            float x           = AMotionEvent_getX(a_event, i);
                            float y           = AMotionEvent_getY(a_event, i);
                            event             = MTouch.Event(eMOVE, x, y, finger_id);
                            eventFIFO.push(event);
                        }
                        if (MTouch.count>0) {
                            uint8_t finger_id = (uint8_t)AMotionEvent_getPointerId(a_event, 0);
                            float x           = AMotionEvent_getX(a_event, 0);
                            float y           = AMotionEvent_getY(a_event, 0);
                            event             = MTouch.Event(eMOVE, x, y, finger_id);
                        }


                    } else {
                        size_t inx        = (size_t)(a_action >> 8); // get index from top 24 bits
                        uint8_t finger_id = (uint8_t)AMotionEvent_getPointerId(a_event, inx);
                        float x           = AMotionEvent_getX(a_event, inx);
                        float y           = AMotionEvent_getY(a_event, inx);
                        switch (action) {
                            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                            case AMOTION_EVENT_ACTION_DOWN      :  event = MTouch.Event(eDOWN, x, y, finger_id);  break;
                            case AMOTION_EVENT_ACTION_POINTER_UP:
                            case AMOTION_EVENT_ACTION_UP        :  event = MTouch.Event(eUP  , x, y, finger_id);  break;
                            case AMOTION_EVENT_ACTION_CANCEL    :  MTouch.Clear();                                break;
                            default:break;
                        }
                    }
                    //-------------------------Emulate mouse from touch events--------------------------
                    // if(event.tag==EventType::TOUCH && event.touch.id==0){  //if one-finger touch
                    //     eventFIFO.push(MouseEvent(event.touch.action, event.touch.x, event.touch.y, 1));
                    // }
                    //----------------------------------------------------------------------------------
                    handled = 0;
                }
                AInputQueue_finishEvent(m_app->inputQueue, a_event, handled);
                return event;
            }

        }  // else if (id == LOOPER_ID_USER) { }

        // Check if we are exiting.
        if (m_app->destroyRequested) {
            LOGI("destroyRequested");
            return CloseEvent();
        }
        return {EventType::NONE};
    };

    //--Show / Hide keyboard--
    void ShowKeyboard(bool enabled) {
        m_textinput = enabled;
        ::ShowKeyboard(enabled);
        LOGI("%s keyboard", enabled ? "Show" : "Hide");
    }

    virtual const void* GetNativeHandle() const {return m_app->window;};

    float GetDisplayScale() { return m_display_scale; }

    virtual void ShowImage(uint32_t* buf, uint32_t width, uint32_t height) {
        auto& wnd = m_app->window;
        int w = ANativeWindow_getWidth(wnd);
        int h = ANativeWindow_getHeight(wnd);
        int s = GetDisplayScale();
        //printf("w=%d h=%d  w2=%d h2=%d\n", w,h, width, height);

        ANativeWindow_Buffer outbuf;
        //ANativeWindow_setBuffersGeometry(wnd, width, height, WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_setBuffersGeometry(wnd, w/s, h/s, WINDOW_FORMAT_RGBA_8888);

        ANativeWindow_acquire(wnd);
        ARect bounds{0, 0, (int)w, (int)h};
        ANativeWindow_lock(wnd, &outbuf, &bounds);
        //printf("bounds: left=%d top=%d right=%d bottom=%d\n", bounds.left, bounds.top, bounds.right, bounds.bottom);
        w = bounds.right - bounds.left;
        h = bounds.bottom - bounds.top;
        uint min_w = MIN(w, width);
        uint min_h = MIN(h, height);
        int step = std::ceil((float)w/64)*64;  // round up to nearest 64 pixels
        for(int y = 0; y<min_h; ++y) {
            uint32_t* src = buf + y * width;
            uint32_t* dst = ((uint32_t*)outbuf.bits) + y*step;
            memcpy(dst, src, min_w*4);
        }
        ANativeWindow_unlockAndPost(wnd);
        ANativeWindow_release(wnd);
    }
};

#endif

#endif  // VK_USE_PLATFORM_ANDROID_KHR
//==============================================================
