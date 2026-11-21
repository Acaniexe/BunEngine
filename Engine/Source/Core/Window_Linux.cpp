#include "Core.h"

#if defined(BUN_PLATFORM_LINUX)

#include "Window.h"
#include "Input.h"
#include "Logger.h"
#include <X11/Xlib.h>
#include <cstdlib>

namespace Bun
{
    class LinuxWindow : public Window
    {
    public:
        explicit LinuxWindow(const WindowDesc& desc)
            : m_Width(desc.Width), m_Height(desc.Height), m_VSync(desc.VSync)
        {
            if (getenv("WAYLAND_DISPLAY"))
                Logger::Get().Warn("Native Wayland not yet supported - using X11 (via XWayland)");

            m_Display = XOpenDisplay(nullptr);
            i32 screen = DefaultScreen(m_Display);

            m_Window = XCreateSimpleWindow(
                m_Display, RootWindow(m_Display, screen),
                0, 0, desc.Width, desc.Height, 1,
                BlackPixel(m_Display, screen), BlackPixel(m_Display, screen)
            );

            m_NativeHandle.display = m_Display;
            m_NativeHandle.window = m_Window;

            XStoreName(m_Display, m_Window, desc.Title.c_str());

            XSelectInput(m_Display, m_Window,
                ExposureMask | KeyPressMask | KeyReleaseMask |
                ButtonPressMask | ButtonReleaseMask |
                PointerMotionMask | StructureNotifyMask);

            m_WmDeleteMessage = XInternAtom(m_Display, "WM_DELETE_WINDOW", False);
            XSetWMProtocols(m_Display, m_Window, &m_WmDeleteMessage, 1);

            XMapWindow(m_Display, m_Window);
        }

        ~LinuxWindow() override
        {
            XDestroyWindow(m_Display, m_Window);
            XCloseDisplay(m_Display);
        }

        void OnUpdate() override 
        {
            while (XPending(m_Display))
            {
                XEvent ev;
                XNextEvent(m_Display, &ev);
                HandleEvent(ev);
            }
            if (m_CursorWrap) PollCursorWrap();
        }

        void SetCursorWrap(bool enabled) override
        {
            if (enabled == m_CursorWrap) return;
            m_CursorWrap = enabled;
            if (enabled)
            {
                ::Window root, child; int rx, ry, wx, wy; unsigned int mask;
                XQueryPointer(m_Displaym m_Window, &root, &child, &rx, &ry, &wx, &wy, &mask);
                m_LastX = wx; m_LastY = wy;
                m_VirtualX = (f32)wx; m_VirtualY = (f32)wy;
            }
        }

        u32 GetWidth() const override { return m_Width; }
        u32 GetHeight() const override { return m_Height; }
        bool ShouldClose() const override { return m_ShouldClose; }
        void SetEventCallback(const EventCallbackFn& cb) override { m_EventCallback = cb; }
        void SetVSync(bool enabled) override { m_VSync = enabled; }
        bool IsVSync() const override { return m_VSync; }
        void* GetNativeHandle() const override { return (void*)&m_NativeHandle; }

    private:
        void HandleEvent(const XEvent& ev)
        {
            switch (ev.type)
            {
                case ClientMessage:
                {
                    if ((Atom)ev.xclient.data.l[0] == m_WmDeleteMessage)
                    {
                        m_ShouldClose = true;
                        WindowCloseEvent e;
                        if (m_EventCallback) m_EventCallback(e);
                    }
                    break;
                }

                case ConfigureNotify:
                {
                    m_Width = ev.xconfigure.width;
                    m_Height = ev.xconfigure.height;
                    WindowResizeEvent e(m_Width, m_Height);
                    if (m_EventCallback) m_EventCallback(e);
                    break;
                }

                case KeyPress:
                {
                    KeySym keysym = XLookupKeysym((XKeyEvent*)&ev.xkey, 0);
                    i32 key = (i32)keysym;
                    if (key >= 'a' && key <= 'z') key -= 32;

                    Input::OnKeyDown(key);
                    KeyPressedEvent e(key, false);
                    if (m_EventCallback) m_EventCallback(e);
                    break;
                }

                case KeyRelease:
                {
                    KeySym keysym = XLookupKeysym((XKeyEvent*)&ev.xkey, 0);
                    i32 key = (i32)keysym;
                    if (key >= 'a' && key <= 'z') key -= 32;

                    Input::OnKeyUp(key);
                    KeyReleasedEvent e(key);
                    if (m_EventCallback) m_EventCallback(e);
                    break;
                }

                case ButtonPress:
                {
                    u32 btn = ev.xbutton.button;
                    if (btn == 4 || btn == 5)
                    {
                        f32 delta = (btn == 4) ? 1.0f : -1.0f;
                        Input::OnMouseScroll(delta);
                        MouseScrolledEvent e(0.0f, delta);
                        if (m_EventCallback) m_EventCallback(e);
                    }
                    else
                    {
                        i32 mouseBtn = (btn == 1) ? Mouse::Left : (btn == 3) ? Mouse::Right : Mouse::Middle;
                        Input::OnMouseButtonDown(mouseBtn);
                        MouseButtonPressedEvent e(mouseBtn);
                        if (m_EventCallback) m_EventCallback(e);
                    }
                    break;
                }

                case ButtonRelease:
                {
                    u32 btn = ev.xbutton.button;
                    
                    i32 mouseBtn = (btn == 1) ? Mouse::Left : (btn == 3) ? Mouse::Right : Mouse::Middle;
                    Input::OnMouseButtonUp(mouseBtn);
                    MouseButtonReleasedEvent e(mouseBtn);
                    if (m_EventCallback) m_EventCallback(e);
                    
                    break;
                }

                case MotionNotify:
                {
                    if (m_CursorWrap) break;
                    f32 x = (f32)ev.xmotion.x;
                    f32 y = (f32)ev.xmotion.y;
                    Input::OnMouseMove(x, y);
                    MouseMovedEvent e(x, y);
                    if (m_EventCallback) m_EventCallback(e);
                    break;
                }
            }
        }

        void PollCursorWrap()
        {
            ::Window root, child; int rx, ry, wx, wy; unsigned int mask;
            if (!XQueryPointer(m_Display, m_Window, &root, &child, &rx, &ry, &wx, &wy, &mask)) return;

            m_VirtualX += (f32)(wx - m_LastX);
            m_VirtualY += (f32)(wy - m_LastY);

            const int margin = 8;
            int tx = wx, ty = wy;
            if (wx <= margin) tx = (int)m_Width - margin - 2;
            else if (wx >= (int)m_Width - margin) tx = margin + 2;
            if (wy <= margin) ty = (int)m_Height - margin - 2;
            else if (wy >= (int)m_Height - margin) ty = margin + 2;

            if (tx != wx || ty != wy)
            {
                XWarpPointer(m_Display, None, m_Window, 0, 0, 0, 0, tx, ty);
                XFlush(m_Display);
                wx = tx; wy = ty;
            }
            m_LastX = wxl m_LastY = wy;

            Input::OnMouseMove(m_VirtualX, m_VirtualY);
            MouseMovedEvent e(m_VirtualX, m_VirtualY);
            if (m_EventCallback) m_EventCallback(e);
        }

        Display* m_Display = nullptr;
        ::Window m_Window;
        Atom m_WmDeleteMessage;
        u32 m_Width, m_Height;
        bool m_VSync;
        bool m_ShouldClose = false;
        EventCallbackFn m_EventCallback;
        bool m_CursorWrap = false;
        f32 m_VirtualX = 0.f, m_VirtualY = 0.f;
        int m_LastX = 0, m_LastY = 0;

        struct NativeHandle
        {
            Display* display;
            ::Window window;
        } m_NativeHandle;
    };

    Scope<Window> Window::Create(const WindowDesc& desc)
    {
        return MakeScope<LinuxWindow>(desc);
    }
}

#endif