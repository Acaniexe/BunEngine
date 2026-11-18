#include <Runtime/Application.h>
#include <Core/Input.h>
#include <Renderer/Camera.h>

#include <Assets/ObjLoader.h>
#include <Assets/ImportedScene.h>
#include <Assets/MeshData.h>
#include <Renderer/Mesh.h>
#include <cstdio>

using namespace Bun;

class SandboxApp : public Application
{
protected:

    void OnInit() override
    {
        Assets::ImportedScene scene;
        if (Assets::ObjLoader::Load("Sandbox/Content/Models/PlayArea.obj", scene))
        {
            for (auto& m : scene.meshes)
            {
                Math::Vec3 color(0.6f);
                if (m.materialIndex >= 0)
                    color = scene.materials[m.materialIndex].baseColor;

                m_Meshes.push_back({ GetRenderer().CreateMesh(m.data), color });
            }
            std::printf("Imported %zu meshes, %zu materials\n", scene.meshes.size(), scene.materials.size());

            for (auto& mat : scene.materials)
                std::printf("Material %s: colour %.2f %.2f %.2f, texture '%s'\n", 
                    mat.name.c_str(), mat.baseColor.x, mat.baseColor.y, mat.baseColor.z, mat.diffuseTexture.c_str());
        }
    }

    void OnShutdown() override{
        for (auto& m : m_Meshes) m.mesh->Shutdown();
    }

    void OnUpdate(f32 dt) override
    {
        m_Time += dt;
        const f32 radius = 2.0f;
        const f32 height = 1.5f;
        const f32 angularSpeed = 1.0f;
        const f32 angle = m_Time * angularSpeed;

        GetRenderer().SetLightPosition({radius * Math::Cos(angle), height, radius * Math::Sin(angle)});

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
        //Math::Mat4 model = Math::Mat4::RotationY(m_Time) * Math::Mat4::RotationX(m_Time * 0.5f);
        Math::Mat4 model;
        GetRenderer().SetCamera(m_Camera.GetView(), m_Camera.GetProjection());
        /*GetRenderer().DrawTestCube(model);
        GetRenderer().DrawLightMarker();*/

        if (m_Meshes.empty()) GetRenderer().DrawTestCube(model);
        for (auto& m : m_Meshes) GetRenderer().DrawMesh(*m.mesh, model, m.color);
        GetRenderer().DrawLightMarker();
    }

private:
    Renderer::Camera m_Camera;
    f32 m_Time = 0.f;
    struct SceneMesh
    {
        Scope<Renderer::Mesh> mesh;
        Math::Vec3 color;
    };

    std::vector<SceneMesh> m_Meshes;
};

int main(int argc, char* argv[])
{
    const char* executablePath = (argc > 0 && argv != nullptr) ? argv[0] : nullptr;
    Application::SetExecutablePath(executablePath);

    SandboxApp app;
    app.Run();
    return 0;
}