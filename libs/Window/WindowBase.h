/*
*--------------------------------------------------------------------------
* FIFO Buffer is used in the few cases where event messages need to be buffered or swapped.
* EventType contains a union struct of all possible message types that may be returned by GetEvent.
* WindowBase is the abstract base class for all the platform-specific window classes.
*--------------------------------------------------------------------------
*/


#ifndef WINDOWBASE_H
#define WINDOWBASE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <string>

#include "Logging.h"
#include "keycodes.h"

// clang-format off
typedef unsigned int uint;

enum eAction { eUP, eDOWN, eMOVE };  // keyboard / mouse / touchscreen actions

//========================Event Message=========================
struct EventType {
    enum Tag{NONE, MOUSE, KEY, TEXT, MOVE, RESIZE, FOCUS, TOUCH, CLOSE, UNKNOWN} tag; // event type
    union {
        struct {eAction action; int16_t x; int16_t y; uint8_t btn;} mouse;     // mouse move/click
        struct {eAction action; eKeycode keycode;                 } key;       // Keyboard key state
        struct {const char* str;                                  } text;      // Text entered
        struct {int16_t x; int16_t y;                             } move;      // Window move
        struct {uint16_t width; uint16_t height;                  } resize;    // Window resize
        struct {bool has_focus;                                   } focus;     // Window gained/lost focus
        struct {eAction action; float x; float y; uint8_t id;     } touch;     // multi-touch display
        struct {                                                  } close;     // Window is closing
    };
    void Clear() { tag = NONE; }
};
//==============================================================
//======================== FIFO Buffer =========================  // Used for event message queue
class EventFIFO {
    static const char SIZE = 32;
    int head, tail;
    EventType buf[SIZE] = {};

  public:
    EventFIFO() : head(0), tail(0) {}
    bool isEmpty() { return head == tail; }                                                    // Check if queue is empty.
    void push(EventType const& item) { ++head; buf[head %= SIZE] = item; }                     // Add item to queue
    EventType* pop() { if (head == tail) return nullptr; ++tail; return &buf[tail %= SIZE]; }  // Returns item ptr, or 0 if queue is empty
};
//==============================================================
//=========================MULTI-TOUCH==========================
class CMTouch {
    struct CPointer{bool active; float x; float y;};
    static const int  MAX_POINTERS = 10;  // Max 10 fingers
    uint32_t touchID [MAX_POINTERS]{};    // finger-id lookup table (PC)
    CPointer Pointers[MAX_POINTERS]{};

  public:
    int count;  // number of active touch-id's (Android only)
    void Clear() { memset(this, 0, sizeof(*this)); }

    // Convert desktop-style touch-id's to an android-style finger-id.
    EventType Event_by_ID(eAction action, float x, float y, uint32_t findval, uint32_t setval) {
        for (uint32_t i = 0; i < MAX_POINTERS; ++i) {  // lookup finger-id
            if (touchID[i] == findval) {
                touchID[i] = setval;
                return Event(action, x, y, i);
            }
        }
        return {EventType::UNKNOWN};
    }

    EventType Event(eAction action, float x, float y, uint8_t id) {
        if (id >= MAX_POINTERS) return {};  // Exit if too many fingers
        CPointer& P                   = Pointers[id];
        if (action != eMOVE) P.active = (action == eDOWN);
        P.x                           = x;
        P.y                           = y;
        EventType e                   = {EventType::TOUCH};
        e.touch                       = {action, x, y, id};
        return e;
    }
};
//==============================================================

//======================Window base class=======================
class WindowBase {
    struct {int16_t x; int16_t y;}mousepos = {};                               // mouse position
    bool m_btnstate[6]   = {};                                                 // mouse btn state
    bool m_keystate[256] = {};                                                 // keyboard state

  protected:
    EventFIFO eventFIFO;                                                       // Event message queue buffer
    EventType MouseEvent (eAction action, int16_t x, int16_t y, uint8_t btn);  // Mouse event
    EventType KeyEvent   (eAction action, uint8_t key);                        // Keyboard event
    EventType TextEvent  (const char* str);                                    // Text event
    EventType MoveEvent  (int16_t x, int16_t y);                               // Window moved
    EventType ResizeEvent(uint16_t width, uint16_t height);                    // Window resized
    EventType FocusEvent (bool has_focus);                                     // Window gained/lost focus
    EventType CloseEvent ();                                                   // Window closing

    float m_display_scale = 1.f;
    bool m_running;
    bool m_textinput;
    bool m_has_focus;                                                          // true if window has focus
    bool m_resized;
    struct shape_t { int16_t x; int16_t y; uint16_t width; uint16_t height; } shape = {};  // window shape
    std::string clipboard;                                                     // fake clipboard

  public:
    WindowBase() : m_running(false), m_textinput(false), m_has_focus(false), m_resized(false){}
    virtual ~WindowBase() {}
    virtual void Close() { eventFIFO.push(CloseEvent()); }

    //--State query functions--
    void GetWinPos  (int16_t& x, int16_t& y) { x = shape.x; y = shape.y; }
    void GetWinSize (int16_t& width, int16_t& height) { width = shape.width; height = shape.height; }
    void GetWinSize (int32_t& width, int32_t& height) { width = shape.width; height = shape.height; }
    bool GetKeyState(eKeycode key) { return m_keystate[key]; }                      // returns true if key is pressed
    bool GetBtnState(uint8_t  btn) { return (btn < 6) ? m_btnstate[btn] : 0; }      // returns true if mouse btn is pressed
    void GetMousePos(int16_t& x, int16_t& y) {x = mousepos.x; y = mousepos.y;}    // returns mouse x,y position
    bool IsRunning() { return m_running; }
    uint Width() {return shape.width;}
    uint Height(){return shape.height;}
    bool Resized() { bool resized = m_resized; m_resized = false; return resized; }
    virtual float GetDisplayScale() {return 1.f;}

    virtual const char* GetClipboardText() {return clipboard.c_str(); }  // Fake clipboard. Works only locally.
    virtual void SetClipboardText(const char* str) { clipboard = str; }  // TODO: Override this with platform implementation.

    //--Control functions--
    virtual void ShowKeyboard(bool enabled);                      // Shows the Android soft-keyboard. //TODO: Enable TextEvent?
    virtual bool TextInput() { return m_textinput; }              // Returns true if text input is enabled TODO: Fix this
    virtual void SetTitle(const char* title) {}
    virtual void SetWinPos (uint x, uint y) {}
    virtual void SetWinSize(uint w, uint h) {}
    virtual const void* GetNativeHandle() const = 0;
    virtual void ShowImage(uint32_t* buf, uint32_t width, uint32_t height) {}

    //--Event loop--
    virtual EventType GetEvent(bool wait_for_event = false) = 0;  // Fetch one event from the queue.
    bool ProcessEvents(bool wait_for_event = false);              // Dispatch all waiting events to event handlers. Returns false if window is closing.
    bool ProcessEvent (EventType e);                              // Dispatch/inject the given event to event handlers.
    bool PollEvents() { return ProcessEvents(false); }            // Run continuously
    bool WaitEvents() { return ProcessEvents(true ); }            // Pause app when there are no events to process
    // void Run(){ while(ProcessEvents()){} }                              // Run message loop until window is closed.

    //-- Virtual Functions as event handlers --
    virtual void OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {}  // Callback for mouse events
    virtual void OnKeyEvent(eAction action, eKeycode keycode) {}                     // Callback for keyboard events (keycodes)
    virtual void OnTextEvent(const char *str) {}                                     // Callback for text typed events (text)
    virtual void OnMoveEvent(int16_t x, int16_t y) {}                                // Callback for window move events
    virtual void OnResizeEvent(uint16_t width, uint16_t height) {}                   // Callback for window resize events
    virtual void OnFocusEvent(bool hasFocus) {}                                      // Callback for window gain/lose focus events
    virtual void OnTouchEvent(eAction action, float x, float y, uint8_t id) {}       // Callback for Multi-touch events
    virtual void OnCloseEvent() {}                                                   // Callback for window closing event
};
//==============================================================

#endif
