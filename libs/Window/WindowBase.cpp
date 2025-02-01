/*
*--------------------------------------------------------------------------
* Platform-specific event handlers call these functions to store input-device state,
* and package the event parameters into a platform-independent "EventType" struct.
*--------------------------------------------------------------------------
*/

#include "WindowBase.h"

//--Events--
EventType WindowBase::MouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) {
    mousepos = {x, y};
    if (action != eMOVE) m_btnstate[btn] = (action == eDOWN);  // Keep track of button state
    EventType e = {EventType::MOUSE, {action, x, y, btn}};
    return e;
}

EventType WindowBase::KeyEvent(eAction action, uint8_t key) {
    m_keystate[key] = (action == eDOWN);
    EventType e   = {EventType::KEY};
    e.key         = {action, (eKeycode)key};
    return e;
}

EventType WindowBase::TextEvent(const char* str) {
    EventType e = {EventType::TEXT};
    e.text.str  = str;
    return e;
}

EventType WindowBase::MoveEvent(int16_t x, int16_t y) {
    shape.x     = x;
    shape.y     = y;
    EventType e = {EventType::MOVE};
    e.move      = {x, y};
    return e;
}

EventType WindowBase::ResizeEvent(uint16_t width, uint16_t height) {
    this->m_resized = true;
    shape.width  = width;
    shape.height = height;
    EventType e  = {EventType::RESIZE};
    e.resize     = {width, height};
    return e;
}

EventType WindowBase::FocusEvent(bool has_focus) {
    this->m_has_focus   = has_focus;
    EventType e       = {EventType::FOCUS};
    e.focus.has_focus = has_focus;
    return e;
}

EventType WindowBase::CloseEvent() {
    m_running = false;
    return {EventType::CLOSE};
}
//----------

void WindowBase::ShowKeyboard(bool enabled) { m_textinput = enabled; }

bool WindowBase::ProcessEvents(bool wait_for_event) {
    EventType e = GetEvent(wait_for_event);
    while (e.tag != EventType::NONE) {
        m_running = ProcessEvent(e);  // Call event handlers
        if(!m_running) return false;
        e = GetEvent();
    }
    return m_running;
}

bool WindowBase::ProcessEvent(EventType e) {
    switch (e.tag) {
       case EventType::MOUSE : OnMouseEvent (e.mouse.action, e.mouse.x, e.mouse.y, e.mouse.btn);  break;
       case EventType::KEY   : OnKeyEvent   (e.key.action, e.key.keycode);                        break;
       case EventType::TEXT  : OnTextEvent  (e.text.str);                                         break;
       case EventType::MOVE  : OnMoveEvent  (e.move.x, e.move.y);                                 break;
       case EventType::RESIZE: OnResizeEvent(e.resize.width, e.resize.height);                    break;
       case EventType::FOCUS : OnFocusEvent (e.focus.has_focus);                                  break;
       case EventType::TOUCH : OnTouchEvent (e.touch.action, e.touch.x, e.touch.y, e.touch.id);   break;
       case EventType::CLOSE : OnCloseEvent (); return false;
       default: break;
    }
    return true;
}


