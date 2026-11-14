#include <Runtime/Application.h>
#include <Core/Input.h>
#include <Renderer/Camera.h>

using namespace C4ndy;

class SandboxApp : public Application
{
protected:
    void OnUpdate(f32 dt) override
    {
        m_Time += dt;

        if (Input::IsMouseButtonDown(Mouse::Right))
        {
            const f32 sens = 0.003f;
            m_Camera.Rotate(Input::GetMouseDeltaX() * sens, -Input::GetMouseDeltaY() * sens);
        }

        const f32 speed = 3.0f * dt;
        Math::Vec3 fwd = m_Camera.GetForward();
        Math::Vec3 right = m_Camera.GetRight();
        if (Input::IsKeyDown(Key::W)) m_Camera.Move(fwd * speed);
        if (Input::IsKeyDown(Key::S)) m_Camera.Move(fwd * -speed);
        if (Input::IsKeyDown(Key::D)) m_Camera.Move(right * speed);
        if (Input::IsKeyDown(Key::A)) m_Camera.Move(right * -speed);
        if (Input::IsKeyDown(Key::E)) m_Camera.Move(Math::Vec3(0.f, speed, 0.f));
        if (Input::IsKeyDown(Key::Q)) m_Camera.Move(Math::Vec3(0.f, -speed, 0.f));

        if (GetWindow().GetHeight() > 0)
            m_Camera.SetAspect((f32)GetWindow().GetWidth() / (f32)GetWindow().GetHeight());
    }

    void OnRender() override
    {
        Math::Mat4 model = Math::Mat4::RotationY(m_Time) * Math::Mat4::RotationX(m_Time * 0.5f);
        GetRenderer().SetCamera(m_Camera.GetView(), m_Camera.GetProjection());
        GetRenderer().DrawTestCube(model);
        GetRenderer().DrawLightMarker();
    }

private:
    Renderer::Camera m_Camera;
    f32 m_Time = 0.f;
};

int main(int argc, char* argv[])
{
    const char* executablePath = (argc > 0 && argv != nullptr) ? argv[0] : nullptr;
    Application::SetExecutablePath(executablePath);

    SandboxApp app;
    app.Run();
    return 0;
}