//==========================Win32===============================

//#define VK_USE_PLATFORM_WIN32_KHR
//#define WINDOW_IMPLEMENTATION
//#define ENABLE_MULTITOUCH

#ifdef VK_USE_PLATFORM_WIN32_KHR

#ifndef WINDOW_WIN32
#define WINDOW_WIN32

#include "WindowBase.h"
#include <windowsx.h>  // Mouse
#include <assert.h>
#include <ShellScalingApi.h> // for GetDpiForWindow

#define MIN(a,b) ((a<b)?(a):(b))
#define MAX(a,b) ((a>b)?(a):(b))


// Convert native Win32 keyboard scancode to cross-platform USB HID code.
const unsigned char WIN32_TO_HID[256] = {
      0,  0,  0,  0,  0,  0,  0,  0, 42, 43,  0,  0,  0, 40,  0,  0,    // 16
    225,224,226, 72, 57,  0,  0,  0,  0,  0,  0, 41,  0,  0,  0,  0,    // 32
     44, 75, 78, 77, 74, 80, 82, 79, 81,  0,  0,  0, 70, 73, 76,  0,    // 48
     39, 30, 31, 32, 33, 34, 35, 36, 37, 38,  0,  0,  0,  0,  0,  0,    // 64
      0,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,    // 80
     19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,  0,  0,  0,  0,  0,    // 96
     98, 89, 90, 91, 92, 93, 94, 95, 96, 97, 85, 87,  0, 86, 99, 84,    //112
     58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,104,105,106,107,    //128
    108,109,110,111,112,113,114,115,  0,  0,  0,  0,  0,  0,  0,  0,    //144
     83, 71,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,    //160
    225,229,224,228,226,230,  0,  0,  0,  0,  0,  0,  0,127,128,129,    //176    L/R shift/ctrl/alt  mute/vol+/vol-
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 51, 46, 54, 45, 55, 56,    //192
     53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,    //208
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 47, 49, 48, 52,  0,    //224
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,    //240
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0     //256
};
//=============================Win32============================
class Window_win32 : public WindowBase {
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HBITMAP m_DIB = 0;  // For ShowImage().  Holds image to display.

    CMTouch MTouch;  // Multi-Touch device
    void Create(const char* title="Window", uint width=640, uint height=480);
public:
    void SetTitle(const char* title);
    void SetWinPos (uint x, uint y);
    void SetWinSize(uint w, uint h);

public:
    Window_win32(){Create();};
    Window_win32(const char* title, uint width, uint height);
    virtual ~Window_win32();
    EventType GetEvent(bool wait_for_event = false);
    const void* GetNativeHandle() const {return &m_hInstance;};
    float GetDisplayScale();
    void ShowImage(uint32_t* buf, uint32_t width, uint32_t height);
};
//==============================================================
#endif

#ifdef WINDOW_IMPLEMENTATION

//=====================Win32 IMPLEMENTATION=====================
LRESULT CALLBACK WndProc(HWND m_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Window_win32::Window_win32(const char* title, uint width, uint height) {
    Create(title, width, height);
}

void Window_win32::Create(const char* title, uint width, uint height) {
    shape.width  = width;
    shape.height = height;
    m_running      = true;
    LOGI("Creating Win32 Window...\n");

    m_hInstance = GetModuleHandle(NULL);

    // Initialize the window class structure:
    WNDCLASSEX win_class;
    win_class.cbSize        = sizeof(WNDCLASSEX);
    win_class.style         = CS_HREDRAW | CS_VREDRAW;
    win_class.lpfnWndProc   = WndProc;
    win_class.cbClsExtra    = 0;
    win_class.cbWndExtra    = 0;
    win_class.hInstance     = m_hInstance;
    win_class.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    win_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    win_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    win_class.lpszMenuName  = NULL;
    win_class.lpszClassName = title;
    win_class.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);
    // Register window class:
    ATOM atom = RegisterClassEx(&win_class);
    assert(atom && "Failed to register the window class.");

    // Create window with the registered class:
    RECT wr = {0, 0, (LONG)width, (LONG)height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    m_hWnd = CreateWindowEx(0,
                          title,                                          // class name
                          title,                                          // app name
                          WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPEDWINDOW,  // window style
                          100, 100,                                       // x/y coords
                          wr.right - wr.left,                             // width
                          wr.bottom - wr.top,                             // height
                          NULL,                                           // handle to parent
                          NULL,                                           // handle to menu
                          m_hInstance,                                    // hInstance
                          NULL);                                          // no extra parameters
    assert(m_hWnd && "Failed to create a window.");

    eventFIFO.push(ResizeEvent(width, height));
}

Window_win32::~Window_win32() { DestroyWindow(m_hWnd); }

void Window_win32::SetTitle(const char* title) { SetWindowText(m_hWnd, title); }

void Window_win32::SetWinPos(uint x, uint y) {
    SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
    if (x != shape.x || y != shape.y) eventFIFO.push(MoveEvent(x, y));  // Trigger window moved event
}

void Window_win32::SetWinSize(uint w, uint h) {
    RECT wr = {0, 0, (LONG)w, (LONG)h};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);  // Add border size to create desired client area size
    int total_width = wr.right - wr.left;
    int total_height = wr.bottom - wr.top;
    SetWindowPos(m_hWnd, NULL, 0, 0, total_width, total_height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
    if ((w != shape.width) | (h != shape.height)) eventFIFO.push(ResizeEvent(w, h));  // Trigger resize event
}

#define WM_RESHAPE (WM_USER + 0)
#define WM_ACTIVE  (WM_USER + 1)

EventType Window_win32::GetEvent(bool wait_for_event) {
    // EventType event;
    if (!eventFIFO.isEmpty()) return *eventFIFO.pop();

    if (m_running) {
        MSG msg = {};
        if(wait_for_event) m_running = (GetMessage(&msg, NULL, 16, 0) > 0);           // Blocking mode
        else if(!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) return {EventType::NONE};  // Non-blocking mode

        TranslateMessage(&msg);
        int16_t x = GET_X_LPARAM(msg.lParam);
        int16_t y = GET_Y_LPARAM(msg.lParam);

        //--Convert Shift / Ctrl / Alt key messages to LeftShift / RightShift / LeftCtrl / RightCtrl / LeftAlt / RightAlt--
        if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) {
            if (msg.wParam == VK_CONTROL) msg.wParam = (msg.lParam & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL;
            if (msg.wParam == VK_SHIFT) {
                if (!!(::GetKeyState(VK_LSHIFT) & 128) != GetKeyState(KEY_LeftShift )) PostMessage(m_hWnd, msg.message, VK_LSHIFT, 0);
                if (!!(::GetKeyState(VK_RSHIFT) & 128) != GetKeyState(KEY_RightShift)) PostMessage(m_hWnd, msg.message, VK_RSHIFT, 0);
                return {EventType::NONE};
            }
        } else if (msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP) {
            if (msg.wParam == VK_MENU) msg.wParam = (msg.lParam & (1 << 24)) ? VK_RMENU : VK_LMENU;
        }
        //-----------------------------------------------------------------------------------------------------------------

        static char buf[4] = {};
        uint8_t bestBtn = GetBtnState(1) ? 1 : GetBtnState(2) ? 2 : GetBtnState(3) ? 3 : 0;
        switch (msg.message) {
            //--Mouse events--
            case WM_MOUSEMOVE  : return MouseEvent(eMOVE, x, y, bestBtn);
            case WM_LBUTTONDOWN: return MouseEvent(eDOWN, x, y, 1);
            case WM_MBUTTONDOWN: return MouseEvent(eDOWN, x, y, 2);
            case WM_RBUTTONDOWN: return MouseEvent(eDOWN, x, y, 3);
            case WM_LBUTTONUP  : return MouseEvent(eUP  , x, y, 1);
            case WM_MBUTTONUP  : return MouseEvent(eUP  , x, y, 2);
            case WM_RBUTTONUP  : return MouseEvent(eUP  , x, y, 3);
            //--Mouse wheel events--
            case WM_MOUSEWHEEL: {
                uint8_t wheel = (GET_WHEEL_DELTA_WPARAM(msg.wParam) > 0) ? 4 : 5;
                POINT point = {x, y};
                ScreenToClient(msg.hwnd, &point);
                return {EventType::MOUSE, {eDOWN, (int16_t)point.x, (int16_t)point.y, wheel}};
            }
            //--Keyboard events--
            case WM_KEYDOWN   : return KeyEvent(eDOWN, WIN32_TO_HID[msg.wParam]);
            case WM_KEYUP     : return KeyEvent(eUP  , WIN32_TO_HID[msg.wParam]);
            case WM_SYSKEYDOWN: {MSG discard; GetMessage(&discard, NULL, 0, 0);     // Alt-key triggers a WM_MOUSEMOVE message... Discard it.
                                return KeyEvent(eDOWN, WIN32_TO_HID[msg.wParam]); } // +alt key
            case WM_SYSKEYUP  : return KeyEvent(eUP  , WIN32_TO_HID[msg.wParam]);   // +alt key

            //--Char event--
            case WM_CHAR: { strncpy_s(buf, (const char*)&msg.wParam, 4);  return TextEvent(buf); }  // return UTF8 code of key pressed
            //--Window events--
            case WM_ACTIVE: { return FocusEvent(msg.wParam != WA_INACTIVE); }

            case WM_RESHAPE: {
                if (!m_has_focus) {
                    PostMessage(m_hWnd, WM_RESHAPE, msg.wParam, msg.lParam);  // Repost this event to the queue
                    return FocusEvent(true);                                // Activate window before reshape
                }

                RECT r;
                GetClientRect(m_hWnd, &r);
                uint16_t w = (uint16_t)(r.right - r.left);
                uint16_t h = (uint16_t)(r.bottom - r.top);
                if (w != shape.width || h != shape.height) return ResizeEvent(w, h);  // window resized

                GetWindowRect(m_hWnd, &r);
                int16_t x = (int16_t)r.left;
                int16_t y = (int16_t)r.top;
                if (x != shape.x || y != shape.y) return MoveEvent(x, y);  // window moved
            }
            case WM_CLOSE: {
                if(msg.hwnd == m_hWnd) {
                    LOGI("WM_CLOSE\n");
                    if(m_DIB) {DeleteObject(m_DIB); m_DIB=0;}
                    return CloseEvent();
                }
                break;
            }
            case WM_PAINT: {
                if(m_DIB) {
                    PAINTSTRUCT ps{};
                    HDC hDC = BeginPaint(m_hWnd, &ps);
                    HDC hMemDC = CreateCompatibleDC(hDC);
                    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, m_DIB);
                    BitBlt(hDC, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, hMemDC, 0, 0, SRCCOPY);
                    SelectObject(hMemDC, hOldBitmap);
                    DeleteDC(hMemDC);
                    EndPaint(m_hWnd, &ps);
                }
                break;
            }

#ifdef ENABLE_MULTITOUCH

#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN   0x0246
#define WM_POINTERUP     0x0247

            case WM_POINTERUPDATE:
            case WM_POINTERDOWN:
            case WM_POINTERUP: {
                POINTER_INFO pointerInfo;
                if (GetPointerInfo(GET_POINTERID_WPARAM(msg.wParam), &pointerInfo)) {
                    uint  id = pointerInfo.pointerId;
                    POINT pt = pointerInfo.ptPixelLocation;
                    ScreenToClient(m_hWnd, &pt);
                    switch (msg.message) {
                        case WM_POINTERDOWN  : return MTouch.Event_by_ID(eDOWN, x, y,  0, id);  // touch down event
                        case WM_POINTERUPDATE: return MTouch.Event_by_ID(eMOVE, x, y, id, id);  // touch move event
                        case WM_POINTERUP    : return MTouch.Event_by_ID(eUP  , x, y, id,  0);  // touch up event
                    }
                }
            }
#endif
        }
        DispatchMessage(&msg);
    }
    return {EventType::NONE};
}

// MS-Windows event handling function:
LRESULT CALLBACK WndProc(HWND m_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostMessage(m_hWnd, WM_CLOSE, 0, 0);  // for OnCloseEvent
            return 0;
        case WM_DESTROY:
            LOGI("WM_DESTROY\n");
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
             //LOGI("WM_PAINT\n");
            return 0;
        case WM_GETMINMAXINFO:  // set window's minimum size
            // ((MINMAXINFO*)lParam)->ptMinTrackSize = demo.minsize;
            return 0;

//        case WM_IME_CHAR: 
//            wprintf(L"WM_IME_CHAR : %c\n", (wchar_t)wParam);
//            return 0;

        case WM_EXITSIZEMOVE : { PostMessage(m_hWnd, WM_RESHAPE, 0, 0);          break; }
        case WM_ACTIVATE     : { PostMessage(m_hWnd, WM_ACTIVE, wParam, lParam); break; }
        default: break;
    }
    return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

float Window_win32::GetDisplayScale() {
    int dpi = GetDpiForWindow(m_hWnd);
    //printf("dpi = %d\n", dpi);
    return dpi/96.f;
}

void Window_win32::ShowImage(uint32_t* buf, uint32_t width, uint32_t height) {  // using GDI only
    if(m_DIB) {DeleteObject(m_DIB); m_DIB=0;}  // delete previous bitmap
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  // Negative height for top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;     // RGBA format
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(0);
    m_DIB = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT, buf, &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);
    InvalidateRect(m_hWnd, NULL, false);
}

#endif  // WINDOW_IMPLEMENTATION

#endif  // VK_USE_PLATFORM_WIN32_KHR
//==============================================================
