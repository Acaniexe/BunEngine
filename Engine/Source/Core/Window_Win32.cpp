#include "Core.h"

#if defined(BUN_PLATFORM_WINDOWS)

#include "Window.h"
#include "Input.h"
#include "Logger.h"
#include <Windows.h>
#include <windowsx.h>
#include <cstdio>

namespace Bun
{
    class WindowsWindow : public Window
    {
    public:
        explicit WindowsWindow(const WindowDesc& desc)
            : m_Width(desc.Width), m_Height(desc.Height), m_VSync(desc.VSync)
        {
            WNDCLASSEX wc = {};
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.lpfnWndProc = WindowsWindow::WndProcStatic;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = L"BunWindowClass";
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

            std::wstring title(desc.Title.begin(), desc.Title.end());

            ATOM classAtom = RegisterClassEx(&wc);
            if (classAtom == 0) 
                Logger::Get().Error("RegisterClass failed: " + std::to_string(GetLastError()));

            m_Hwnd = CreateWindowEx(
                0, L"BunWindowClass", title.c_str(),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                (int)desc.Width, (int)desc.Height,
                nullptr, nullptr, GetModuleHandle(nullptr), this
            );

            if (m_Hwnd == nullptr)
                Logger::Get().Error("CreatedWindowEx failed: " + std::to_string(GetLastError()));
            else
                Logger::Get().Info("Window Created! HWND valid.");


            ShowWindow(m_Hwnd, SW_SHOW);
        }

        ~WindowsWindow() override
        {
            DestroyWindow(m_Hwnd);
        }

        void OnUpdate() override
        {
            MSG msg;
            while (PeekMessage(&msg, m_Hwnd, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (m_CursorWrap) PollCursorWrap();
        }

        void SetCursorWrap(bool enabled) override
        {
            if (enabled == m_CursorWrap) return;
            m_CursorWrap = enabled;

            if (enabled)
            {
                POINT p; GetCursorPos(&p); ScreenToClient(m_Hwnd, &p);
                m_LastScreen = p;
                ScreenToClient(m_Hwnd, &p);
                m_VirtualX = (f32)p.x; m_VirtualY = (f32)p.y;
                SetCapture(m_Hwnd);
            }
            else
                ReleaseCapture();
        }

        u32 GetWidth() const override { return m_Width; }
        u32 GetHeight() const override { return m_Height; }
        bool ShouldClose() const override { return m_ShouldClose; }
        void SetEventCallback(const EventCallbackFn& cb) override { m_EventCallback = cb; }
        void SetVSync(bool enabled) override { m_VSync = enabled; }
        bool IsVSync() const override { return m_VSync; }
        void* GetNativeHandle() const override { return (void*)m_Hwnd; }

    private:
        static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            WindowsWindow* window;
            if (msg == WM_NCCREATE)
            {
                auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
                window = reinterpret_cast<WindowsWindow*>(cs->lpCreateParams);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
            }
            else
            {
                window = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            }

            if (window)
                return window->HandleMessage(hwnd, msg, wParam, lParam);
            
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        void PollCursorWrap()
        {
            POINT now; GetCursorPos(&now);

            m_VirtualX += (f32)(now.x - m_LastScreen.x);
            m_VirtualY += (f32)(now.y - m_LastScreen.y);

            RECT rc; GetClientRect(m_Hwnd, &rc);
            POINT tl = { 0, 0 }, br = { rc.right, rc.bottom };
            ClientToScreen(m_Hwnd, &tl);
            ClientToScreen(m_Hwnd, &br);

            const int margin = 8;
            POINT target = now;
            if (now.x <= tl.x + margin) target.x = br.x - margin - 2;
            else if (now.x >= br.x - margin) target.x = tl.x + margin + 2;
            if (now.y <= tl.y + margin) target.y = br.y - margin - 2;
            else if (now.y >= br.y - margin) target.y = tl.y + margin + 2;

            if ((target.x != now.x || target.y != now.y) && SetCursorPos(target.x, target.y))
                now = target;
            m_LastScreen = now;

            Input::OnMouseMove(m_VirtualX, m_VirtualY);
            MouseMovedEvent e(m_VirtualX, m_VirtualY);
            if (m_EventCallback) m_EventCallback(e);
        }

        LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
                case WM_CLOSE:
                {
                       m_ShouldClose = true;
                    WindowCloseEvent e;
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }
                
                case WM_SIZE:
                {
                    m_Width = LOWORD(lParam);
                    m_Height = HIWORD(lParam);
                    WindowResizeEvent e(m_Width, m_Height);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_KEYDOWN:
                {
                    Input::OnKeyDown((i32)wParam);
                    KeyPressedEvent e((i32)wParam, false);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_KEYUP:
                {
                    Input::OnKeyUp((i32)wParam);
                    KeyReleasedEvent e((i32)wParam);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_LBUTTONDOWN:
                {
                    Input::OnMouseButtonDown(Mouse::Left);
                    MouseButtonPressedEvent e(Mouse::Left);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_LBUTTONUP:
                {
                    Input::OnMouseButtonUp(Mouse::Left);
                    MouseButtonReleasedEvent e(Mouse::Left);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_RBUTTONDOWN:
                {
                    Input::OnMouseButtonDown(Mouse::Right);
                    MouseButtonPressedEvent e(Mouse::Right);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_RBUTTONUP:
                {
                    Input::OnMouseButtonUp(Mouse::Right);
                    MouseButtonReleasedEvent e(Mouse::Right);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_MOUSEMOVE:
                {
                    if (m_CursorWrap) return 0;
                    f32 x = (f32)GET_X_LPARAM(lParam);
                    f32 y = (f32)GET_Y_LPARAM(lParam);
                    Input::OnMouseMove(x, y);
                    MouseMovedEvent e(x, y);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }

                case WM_MOUSEWHEEL:
                {
                    f32 delta = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
                    Input::OnMouseScroll(delta);
                    MouseScrolledEvent e(0.0f, delta);
                    if (m_EventCallback) m_EventCallback(e);
                    return 0;
                }
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        HWND m_Hwnd = nullptr;
        u32 m_Width, m_Height;
        bool m_VSync;
        bool m_ShouldClose = false;
        EventCallbackFn m_EventCallback;     
        bool m_CursorWrap = false;
        f32 m_VirtualX = 0.f, m_VirtualY = 0.f; 
        POINT m_LastScreen{};
    };

    Scope<Window> Window::Create(const WindowDesc& desc)
    {
        return MakeScope<WindowsWindow>(desc);
    }
}

#endif