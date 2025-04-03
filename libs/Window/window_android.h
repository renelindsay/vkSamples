
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

#include <iostream>
#include <string>
#include <cstring>

#undef  repeat
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

//------------------------ JNI Wrappers ------------------------
#define CALL_OBJ_METHOD( OBJ,METHOD,SIGNATURE, ...) env->CallObjectMethod (OBJ, env->GetMethodID(env->GetObjectClass(OBJ),METHOD,SIGNATURE), ##__VA_ARGS__)
//#define CALL_BOOL_METHOD(OBJ,METHOD,SIGNATURE, ...) env->CallBooleanMethod(OBJ, env->GetMethodID(env->GetObjectClass(OBJ),METHOD,SIGNATURE), __VA_ARGS__)
#define GET_STATIC_OBJ_FIELD(CLASS,FIELD,SIGNATURE) env->GetStaticObjectField(CLASS, env->GetStaticFieldID(CLASS, FIELD, SIGNATURE))

static std::vector<int> AInputQueue_getDeviceIds() {
    JavaVM* jvm = Android_App->activity->vm;
    JNIEnv* env = Android_App->activity->env;
    jobject obj = Android_App->activity->clazz;  // native activity
    jvm->AttachCurrentThread(&env, nullptr);     // Attach current thread to the JVM.

    // Get InputManager from Context and call getInputDeviceIds()
    jclass contextClass = env->FindClass("android/content/Context");
    jstring inputService = (jstring)GET_STATIC_OBJ_FIELD(contextClass, "INPUT_SERVICE", "Ljava/lang/String;");
    jobject inputManager = CALL_OBJ_METHOD(obj, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", inputService);
    jintArray deviceIdsArray = (jintArray)CALL_OBJ_METHOD(inputManager, "getInputDeviceIds", "()[I");

    // Convert Java int[] to std::vector<int>
    jsize length = env->GetArrayLength(deviceIdsArray);
    std::vector<int> gamepadIds(length);
    jint *elements = env->GetIntArrayElements(deviceIdsArray, nullptr);
    for (jsize i=0; i<length; ++i) gamepadIds[i] = elements[i];
    env->ReleaseIntArrayElements(deviceIdsArray, elements, JNI_ABORT);

    jvm->DetachCurrentThread();  // Finished with the JVM.
    return gamepadIds;
}

struct GamepadInfo {
    std::string name;
    bool hasHatAxes;
};

static std::string jstringToStdString(JNIEnv* env, jstring jStr) {
    if (!jStr) return "";
    const char* chars = env->GetStringUTFChars(jStr, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(jStr, chars);
    return result;
}

static GamepadInfo GetGamepadInfo(int deviceId) {
    JavaVM* jvm = Android_App->activity->vm;
    JNIEnv* env = Android_App->activity->env;
    jobject obj = Android_App->activity->clazz;  // native activity
    jvm->AttachCurrentThread(&env, nullptr);
    GamepadInfo info;
    //get gamepad hame
    jclass inputDeviceClass = env->FindClass("android/view/InputDevice");
    jmethodID getDeviceMethod = env->GetStaticMethodID(inputDeviceClass, "getDevice", "(I)Landroid/view/InputDevice;");
    jobject inputDevice = env->CallStaticObjectMethod(inputDeviceClass, getDeviceMethod, deviceId);
    jmethodID getNameMethod = env->GetMethodID(inputDeviceClass, "getName", "()Ljava/lang/String;");
    jstring jName = (jstring)env->CallObjectMethod(inputDevice, getNameMethod);
    info.name = jstringToStdString(env, jName);
    env->DeleteLocalRef(jName);
    //gamepad has hat?
    jmethodID getMotionRangeMethod = env->GetMethodID(inputDeviceClass, "getMotionRange", "(I)Landroid/view/InputDevice$MotionRange;");
    jobject hatXRange = env->CallObjectMethod(inputDevice, getMotionRangeMethod, AMOTION_EVENT_AXIS_HAT_X);
    jobject hatYRange = env->CallObjectMethod(inputDevice, getMotionRangeMethod, AMOTION_EVENT_AXIS_HAT_Y);
    info.hasHatAxes = (hatXRange && hatYRange);
    // Cleanup
    env->DeleteLocalRef(inputDevice);
    env->DeleteLocalRef(inputDeviceClass);
    jvm->DetachCurrentThread();
    return info;
}

//--------------------------------------------------------------

class Window_android : public WindowBase {
    android_app* m_app = 0;
    CMTouch MTouch;

    //---- Gamepad ----
    struct GPadSlots {
        int32_t deviceID=0;
        char name[256] = {};
        bool SwapAB =false;       // Nintendo gamepads swap A and B buttons
        bool useDPad=false;       // Sony Dualshock hat emits dpad button events instead of HAT axis
    }gpads[MAX_GAMEPADS];
    //-----------------

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

    //-------------------- GAMEPAD ---------------------
    int Gamepad_keymap(uint keycode, bool useDPAD=false) {
        switch (keycode) {
            case AKEYCODE_BUTTON_A:      return eBTN_A;      // 96
            case AKEYCODE_BUTTON_B:      return eBTN_B;      // 97
            case AKEYCODE_BUTTON_X:      return eBTN_X;      // 99
            case AKEYCODE_BUTTON_Y:      return eBTN_Y;      // 100
            case AKEYCODE_BUTTON_L1:     return eBTN_TL;     // 102
            case AKEYCODE_BUTTON_R1:     return eBTN_TR;     // 103
            case AKEYCODE_BUTTON_SELECT: return eBTN_SELECT; // 109
            case AKEYCODE_BUTTON_START:  return eBTN_START;  // 108
            case AKEYCODE_BUTTON_MODE:   return eBTN_MODE;   // 110
            case AKEYCODE_BUTTON_THUMBL: return eBTN_THUMBL; // 106
            case AKEYCODE_BUTTON_THUMBR: return eBTN_THUMBR; // 107
            //default: return 0;
        }
        if(useDPAD)
        switch (keycode) {
            case AKEYCODE_DPAD_UP:    return eDPAD_UP;      // 19
            case AKEYCODE_DPAD_DOWN:  return eDPAD_DOWN;    // 20
            case AKEYCODE_DPAD_LEFT:  return eDPAD_LEFT;    // 21
            case AKEYCODE_DPAD_RIGHT: return eDPAD_RIGHT;   // 22
            //default: return 0;
        }
        return 0;
    }

    int8_t FindGamepad(AInputEvent* a_event) {  // returns gamepad slot id or -1 if failed
        uint32_t deviceID = AInputEvent_getDeviceId(a_event);
        repeat(MAX_GAMEPADS) if (gpads[i].deviceID == deviceID) return i;  // find gamepad by deviceId
        return -1;
    }

    EventType ConnectGamepad(AInputEvent* a_event) {  // tries to connect to gamepad
        uint32_t deviceID = AInputEvent_getDeviceId(a_event);
        repeat(MAX_GAMEPADS) if (gpads[i].deviceID == 0) {
            auto& pad = gpads[i];
            pad.deviceID = deviceID;
            GamepadInfo info = GetGamepadInfo(deviceID);
            std::string name = info.name;
            strncpy(pad.name, name.c_str(), sizeof(pad.name)-1);
            printf("Gamepad %d found: %s\n", i, pad.name);
            // NINTENDO
            pad.SwapAB=false;  // Nintendo swaps the A and B buttons
            if((name.find("Nintendo")>-1) || (name.find("Switch")>-1))         {pad.SwapAB = true;}
            if((name.find("Joy-Con")>-1)  || (name.find("Pro Controller")>-1)) {pad.SwapAB = true;}
            // SONY
            pad.useDPad = !info.hasHatAxes;
            return GPadConnect(i, true);
        }
        return {EventType::NONE};
    }

    void MonitorGamepads() {
        // run only once per second
        static clock_t last_time = clock();
        clock_t curr_time = clock();
        if (curr_time - last_time < CLOCKS_PER_SEC) return;
        last_time = curr_time;

        auto list = AInputQueue_getDeviceIds();
        //for(int item : list) printf("%d ", item); printf("\n");
        for(int i=0; i<MAX_GAMEPADS; ++i) {
            int32_t id = gpads[i].deviceID;
            if(id) if (std::find(list.begin(), list.end(), id) == list.end()) {
                gpads[i].deviceID = 0;
                eventFIFO.push(GPadConnect(i,false));
            }
        }
    }

    EventType GetGPadButtonEvent(AInputEvent* a_event) {
        //ASSERT(AInputEvent_getType(a_event)==AINPUT_EVENT_TYPE_KEY, "Not a key press event.");
        int8_t id = FindGamepad(a_event);            // Get gamepad ID for this event
        if(id==-1) return ConnectGamepad(a_event);   // If not found, connect

        int32_t keycode  = AKeyEvent_getKeyCode(a_event);
        if (keycode >= AKEYCODE_BUTTON_A && keycode <= AKEYCODE_BUTTON_MODE) {
            //printf("Gamepad %d: ", id);
            if (AKeyEvent_getRepeatCount(a_event) == 0) {  // ignore keyboard repeat events
                auto& pad = gpads[id];
                uint8_t btn = Gamepad_keymap(keycode, pad.useDPad);
                if(pad.SwapAB && btn<5){static int ABtoBA[]={0,2,1,4,3}; btn=ABtoBA[btn];}  // Nintendo: Swap A/B
                bool down = (AKeyEvent_getAction(a_event) == AKEY_EVENT_ACTION_DOWN);
                return GPadButton(id, btn, down);
            }
        }
        return {};
    }

    EventType GetGPadAxisEvent(AInputEvent* a_event) {
        //ASSERT(AInputEvent_getType(a_event)==AINPUT_EVENT_TYPE_MOTION, "Not a motion event.");
        int8_t id = FindGamepad(a_event);            // Get gamepad ID for this event
        if(id==-1) return ConnectGamepad(a_event);   // If not found, connect
        Gamepad& pad = gamepad[id];

        auto axisCheck = [&](eGamepadAxis axis, float val) {
            if(pad.axes[axis] == val) return;
            eventFIFO.push(GPadAxis(id, axis, val));
        };

        auto Hat = [&](int val, int btnNeg, int btnPos) { // convert hat axis values to button events
            if((val!=-1) && ( pad.buttons[btnNeg])) eventFIFO.push(GPadButton(id, btnNeg, 0));
            if((val!= 1) && ( pad.buttons[btnPos])) eventFIFO.push(GPadButton(id, btnPos, 0));
            if((val==-1) && (!pad.buttons[btnNeg])) eventFIFO.push(GPadButton(id, btnNeg, 1));
            if((val== 1) && (!pad.buttons[btnPos])) eventFIFO.push(GPadButton(id, btnPos, 1));
        };

        float lx = AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_X, 0);
        float ly =-AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_Y, 0);
        float rx = AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_Z, 0);
        float ry =-AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_RZ, 0);
              rx+= AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_RX, 0);
              ry+=-AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_RY, 0);
        float tl = AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_GAS, 0);
        float tr = AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_BRAKE, 0);
        float hatx = AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_HAT_X, 0);
        float haty = AMotionEvent_getAxisValue(a_event,AMOTION_EVENT_AXIS_HAT_Y, 0);

        axisCheck(eAXIS_LX, lx);
        axisCheck(eAXIS_LY, ly);
        axisCheck(eAXIS_RX, rx);
        axisCheck(eAXIS_RY, ry);
        axisCheck(eAXIS_TL, tl);
        axisCheck(eAXIS_TR, tr);
        Hat(hatx, eDPAD_LEFT, eDPAD_RIGHT);
        Hat(haty, eDPAD_UP,   eDPAD_DOWN);

        if(!eventFIFO.isEmpty()) return *eventFIFO.pop();
        return {};
    }
    //--------------------------------------------------
    //-------------------- KEYBOARD --------------------
    EventType GetKeyboardEvent(AInputEvent* a_event) {  // KEYBOARD
        int32_t a_action = AKeyEvent_getAction(a_event);
        int32_t keycode  = AKeyEvent_getKeyCode(a_event);
        uint8_t hidcode  = ANDROID_TO_HID[keycode];
        // printf("key action:%d keycode=%d",a_action,keycode);
        if(!hidcode) return {};  // unknown key

        switch (a_action) {
            case AKEY_EVENT_ACTION_DOWN: {
                static char buf[5] = {};
                int metaState = AKeyEvent_getMetaState(a_event);
                int unicode = GetUnicodeChar(AKEY_EVENT_ACTION_DOWN, keycode, metaState);
                if(unicode) {
                    std::string utf8text = UnicodeToUTF8(unicode);
                    memcpy(buf, utf8text.c_str(), 4);  // copy to static buf so it doesn't go out of scope
                    eventFIFO.push(TextEvent(buf));    // text typed event  (store in FIFO for next run)
                }
                return KeyEvent(eDOWN, hidcode);       // key pressed event (returned on this run)
            }
            case AKEY_EVENT_ACTION_UP: {
                return KeyEvent(eUP, hidcode); // key released event
            }
            case AKEY_EVENT_ACTION_MULTIPLE: {
                // TODO: Implement IME and auto-correct string input,
                //  (When google fixes the getCharacters bug.)
                //return TextEvent("IME/AutoCorrect not supported");
            }
        }
        return {};
    }
    //--------------------------------------------------
    //------------------ TOUCHSCREEN -------------------
    EventType GetTouchscreenEvent(AInputEvent* a_event) {
        EventType event = {};
        int32_t a_action = AMotionEvent_getAction(a_event);
        int action       = (a_action & 255); // get action-code from bottom 8 bits
        MTouch.count     = (int)AMotionEvent_getPointerCount(a_event);
        if (action == AMOTION_EVENT_ACTION_MOVE) {  //touch drag events
            for(uint i = 0; i<MTouch.count; ++i) {
                uint8_t finger_id = (uint8_t)AMotionEvent_getPointerId(a_event, i);
                float x           = AMotionEvent_getX(a_event, i);
                float y           = AMotionEvent_getY(a_event, i);
                if(i==0) event    = MTouch.Event(eMOVE, x, y, finger_id);   // return first finger directly
                else eventFIFO.push(MTouch.Event(eMOVE, x, y, finger_id));  // queue additional finger events
            }
        } else {  // touch up/down events
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
        return event;
    }
    //--------------------------------------------------
    //--------------------- MOUSE ----------------------
    EventType GetMouseEvent(AInputEvent* a_event) {
        EventType event = {};
        int32_t a_action = AMotionEvent_getAction(a_event);
        int action = (a_action & 255); // get action-code from bottom 8 bits

        int16_t x = (int16_t)AMotionEvent_getX(a_event, 0);
        int16_t y = (int16_t)AMotionEvent_getY(a_event, 0);
        bool moved = (x!=mousepos.x || y!=mousepos.y);
        if(!moved && action==AMOTION_EVENT_ACTION_HOVER_MOVE) return event;  // eliminate fake move events

        // Get button state (bitmask: 0x1 = left, 0x2 = right, 0x4 = middle)
        int32_t buttons = AMotionEvent_getButtonState(a_event);
        uint8_t bestBtn = GetBtnState(1) ? 1 : GetBtnState(2) ? 2 : GetBtnState(3) ? 3 : 0;

        uint8_t btn = 0;  // get button that changed
        if(m_btnstate[3] != (buttons & AMOTION_EVENT_BUTTON_SECONDARY)) btn = 3;
        if(m_btnstate[2] != (buttons & AMOTION_EVENT_BUTTON_TERTIARY)) btn = 2;
        if(m_btnstate[1] != (buttons & AMOTION_EVENT_BUTTON_PRIMARY)) btn = 1;

        switch (action) {
            case AMOTION_EVENT_ACTION_BUTTON_PRESS   : event = MouseEvent(eDOWN, x, y, btn);     break;
            case AMOTION_EVENT_ACTION_MOVE           : event = MouseEvent(eMOVE, x, y, bestBtn); break;
            case AMOTION_EVENT_ACTION_HOVER_MOVE     : event = MouseEvent(eMOVE, x, y, 0  );     break;
            case AMOTION_EVENT_ACTION_BUTTON_RELEASE : event = MouseEvent(eUP,   x, y, btn);     break;
            case AMOTION_EVENT_ACTION_SCROLL: {
                float vscroll = AMotionEvent_getAxisValue(a_event, AMOTION_EVENT_AXIS_VSCROLL, 0);
                uint8_t wheel = (vscroll > 0) ? 4 : 5;
                event = MouseEvent(eDOWN, x, y, wheel);
                break;
            }
            default: break;
        }
        return event;
    }
    //--------------------------------------------------
    //--------------- Main event handler ---------------
    EventType GetEvent(bool wait_for_event = false) {
        EventType event = {};
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
                //LOGV("Event: source=0x%4x type=%d\n", AInputEvent_getSource(a_event),AInputEvent_getType(a_event));
                if (AInputQueue_preDispatchEvent(m_app->inputQueue, a_event)) { continue; }
                int32_t handled = 0;
                if (m_app->onInputEvent) handled = m_app->onInputEvent(m_app, a_event);

                int32_t source = AInputEvent_getSource(a_event);
                //bool isClassButton   = (source & AINPUT_SOURCE_CLASS_BUTTON);
                //bool isClassPointer  = (source & AINPUT_SOURCE_CLASS_POINTER);
                //bool isClassJoystick = (source & AINPUT_SOURCE_CLASS_JOYSTICK);

                source&=AINPUT_SOURCE_ANY;
                bool isKeyboard  = (source & AINPUT_SOURCE_KEYBOARD);     // class button
                bool isGamepad   = (source & AINPUT_SOURCE_GAMEPAD);      // class button
                bool isJoystick  = (source & AINPUT_SOURCE_JOYSTICK);     // class joystick
                bool isTouch     = (source & AINPUT_SOURCE_TOUCHSCREEN);  // class pointer
                bool isMouse     = (source & AINPUT_SOURCE_MOUSE);        // class pointer
                bool isStylus    = (source & AINPUT_SOURCE_STYLUS);       // class pointer
                if(isStylus) isMouse = true;  // treat stylus as mouse. TODO: Add Stylus support
                //printf("source=0x%4x %s%s%s%s%s\n", source, isTouch?"T":".", isMouse?"M":".", isKeyboard?"K":".", isGamepad?"G":".", isJoystick?"J":".");

                if(!event && isKeyboard) {event = GetKeyboardEvent   (a_event);}
                if(!event && isGamepad)  {event = GetGPadButtonEvent (a_event);}
                if(!event && isJoystick) {event = GetGPadAxisEvent   (a_event);}
                if(!event && isTouch)    {event = GetTouchscreenEvent(a_event);}
                if(!event && isMouse)    {event = GetMouseEvent      (a_event); handled=1;}
                handled |= event;  // if an event was created, mark it as handled
handled = 1;
                if(!event)
                  if (!eventFIFO.isEmpty()) event = *eventFIFO.pop();
                AInputQueue_finishEvent(m_app->inputQueue, a_event, handled);
                return event;
            }
        }  // else if (id == LOOPER_ID_USER) { printf("LOOPER_ID_USER\n");}

        MonitorGamepads();
        if (m_app->destroyRequested) return CloseEvent();  // Check if we are exiting.
        return {};
    };
    //--------------------------------------------------

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
        uint min_w = MIN(w, width);  // draw min of window-w and image-w
        uint min_h = MIN(h, height); // draw min of window-h and image-h
        int stride = outbuf.stride;  // Actual buffer stride for memory alignment

        for(int y = 0; y<min_h; ++y) {
            uint32_t* src = buf + y * width;
            uint32_t* dst = ((uint32_t*)outbuf.bits) + y*stride;
            memcpy(dst, src, min_w*4);
        }
        ANativeWindow_unlockAndPost(wnd);
        ANativeWindow_release(wnd);
    }
};

#endif

#endif  // VK_USE_PLATFORM_ANDROID_KHR
//==============================================================
