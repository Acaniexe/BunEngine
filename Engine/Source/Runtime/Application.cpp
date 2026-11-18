#include "Application.h"
#include <Core/Logger.h>
#include <Core/Input.h>
#include <filesystem>
#include <thread>
#include <chrono>

namespace Bun
{
    std::string Application::s_ExecutablePath;

    void Application::SetExecutablePath(const char* path)
    {
        if (path == nullptr || path[0] == '\0')
        {
            s_ExecutablePath.clear();
            BUN_WARN("Executable path was empty; Shader path resolution may fail.");
            return;
        }

        s_ExecutablePath = std::filesystem::absolute(path).lexically_normal().string();
        BUN_INFO("Executable path: " + s_ExecutablePath);
    }

    Application::Application()
    {
        WindowDesc desc;
        desc.Title = "Bun Engine";
        desc.Width = 1280;
        desc.Height = 720;

        m_Window = Window::Create(desc);
        m_Window->SetEventCallback([this](Event& e) { HandleEvent(e); });

        m_Renderer.Init(*m_Window);

        Logger::Get().Info("Application started.");
    }

    void Application::Run()
    {
        OnInit(); 

        while (m_Running)
        {
            m_Timer.Tick();
            m_Window->OnUpdate();

            OnUpdate((f32)m_Timer.DeltaTime());

            const bool minimised = m_Window->GetWidth() == 0 || m_Window->GetHeight() == 0;
            
            if (minimised)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
            else if (m_Renderer.BeginFrame())
            {
                OnRender();
                m_Renderer.EndFrame();
            }
            

            if (m_Window->ShouldClose())
                m_Running = false;

            Input::Update();
        }

        m_Renderer.WaitIdle();
        OnShutdown();
        m_Renderer.Shutdown();
    }

    void Application::HandleEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(
            [this](WindowCloseEvent& e) { return OnWindowClose(e); });

        OnEvent(e);
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        BUN_UNUSED(e);
        m_Running = false;
        return true;
    }

    void Application::Close()
    {
        m_Running = false;
    }
}