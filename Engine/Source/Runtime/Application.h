#pragma once
#include <Renderer/Renderer.h>
#include <Core/Core.h>
#include <Core/Window.h>
#include <Core/Timer.h>
#include <Core/Event.h>

namespace Bun
{
    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void Run();
        void Close();

        static void SetExecutablePath(const char* path);
        static std::string s_ExecutablePath;

    protected:
        virtual void OnInit() {}
        virtual void OnUpdate(f32 dt) { BUN_UNUSED(dt); }
        virtual void OnRender() {}
        virtual void OnShutdown() {}
        virtual void OnEvent(Event& e) { BUN_UNUSED(e); }

        Renderer::Renderer& GetRenderer() { return m_Renderer; }
        Window& GetWindow() { return *m_Window; }
        
    private:
        void HandleEvent(Event& e);
        bool OnWindowClose(WindowCloseEvent& e);

        Scope<Window> m_Window;
        Timer m_Timer;
        bool m_Running = true;
        Renderer::Renderer m_Renderer;
    };
}