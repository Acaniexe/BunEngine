#include <Runtime/Application.h>
#include <Core/Input.h>
#include <Renderer/Camera.h>
#include <Assets/ObjLoader.h>
#include <Assets/ImportedScene.h>
#include <Assets/MeshData.h>
#include <Renderer/Mesh.h>
#include <Assets/ImageLoader.h>
#include <cstdio>
#include <filesystem>

using namespace Bun;

class SandboxApp : public Application
{
protected:

    void OnInit() override
    {
        const std::filesystem::path modelDir = "Sandbox/Content/Models";
        Assets::ImportedScene scene;
        if (Assets::ObjLoader::Load((modelDir / "PlayArea.obj").string(), scene))
        {
            m_MaterialTextures.resize(scene.materials.size());
            for (usize i = 0; i < scene.materials.size(); i++)
            {
                const auto& mat = scene.materials[i];
                if (mat.diffuseTexture.empty()) continue;

                Assets::ImageData img;
                if (Assets::ImageLoader::Load((modelDir / mat.diffuseTexture).string(), img))
                    m_MaterialTextures[i] = GetRenderer().CreateTexture(img);
                else
                    std::printf("Texture load FAILED: %s\n", mat.diffuseTexture.c_str());
            }

            for (auto& m : scene.meshes)
            {
                Math::Vec3 color(0.6f);
                Renderer::Texture* texture = nullptr;

                if (m.materialIndex >= 0)
                {
                    color = scene.materials[m.materialIndex].baseColor;
                    texture = m_MaterialTextures[m.materialIndex].get();
                    if (texture) color = Math::Vec3(1.0f);
                }
            
                m_Meshes.push_back({ GetRenderer().CreateMesh(m.data), color, texture });
            }
            std::printf("Imported %zu meshes, %zu materials\n", scene.meshes.size(), scene.materials.size());

            for (auto& mat : scene.materials)
                std::printf("Material %s: colour %.2f %.2f %.2f, texture '%s'\n", 
                    mat.name.c_str(), mat.baseColor.x, mat.baseColor.y, mat.baseColor.z, mat.diffuseTexture.c_str());

            for (auto& m : scene.meshes)
            {
                Math::Vec3 mn(1e9f), mx(-1e9f);
                for (auto& v : m.data.vertices)
                {
                    mn.x = Math::Min(mn.x, v.position.x); mx.x = Math::Max(mx.x, v.position.x);
                    mn.y = Math::Min(mn.y, v.position.y); mx.y = Math::Max(mx.y, v.position.y);
                    mn.z = Math::Min(mn.z, v.position.z); mx.z = Math::Max(mx.z, v.position.z);
                }
                std::printf("  %-26s centre %.2f %.2f %.2f\n", m.name.c_str(),
                    (mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f, (mn.z + mx.z) * 0.5f);
            }
        }
    }

    void OnShutdown() override{
        for (auto& t : m_MaterialTextures) if (t) t->Shutdown();
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

        if (Input::IsMouseButtonPressed(Mouse::Right)) GetWindow().SetCursorWrap(true);
        if (Input::IsMouseButtonReleased(Mouse::Right)) GetWindow().SetCursorWrap(false);


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
        for (auto& m : m_Meshes) GetRenderer().DrawMesh(*m.mesh, model, m.color, m.texture);
        GetRenderer().DrawLightMarker();
        DrawAxisGizmo();
    }

    void DrawAxisGizmo()
    {
        Math::Vec3 fwd = m_Camera.GetForward();
        Math::Vec3 right = m_Camera.GetRight();
        Math::Vec3 up = Math::Vec3::Cross(right, fwd);

        Math::Vec3 o = m_Camera.GetPosition() + fwd * 3.0f + right * -1.4f + up * -0.8f;

        const f32 len = -.4f, t = 0.03f;
        auto& r = GetRenderer();
        r.DrawDebugBox(o + Math::Vec3(len * 0.5f, 0.f, 0.f), Math::Vec3(len, t, t), Math::Vec3(1.0f, 0.15f, 0.15f));
        r.DrawDebugBox(o + Math::Vec3(0.f, len * 0.5, 0.f), Math::Vec3(t, len, t), Math::Vec3(0.2f, 1.0f, 0.2f));
        r.DrawDebugBox(o + Math::Vec3(0.f, 0.f, len * 0.5f), Math::Vec3(t, t, len), Math::Vec3(0.2f, 1.0f, 1.0f));
        r.DrawDebugBox(o, Math::Vec3(t * 1.6f), Math::Vec3(1.f));
    }

private:
    Renderer::Camera m_Camera;
    f32 m_Time = 0.f;
    struct SceneMesh
    {
        Scope<Renderer::Mesh> mesh;
        Math::Vec3 color;
        Renderer::Texture* texture = nullptr;
    };

    std::vector<SceneMesh> m_Meshes;
    std::vector<Scope<Renderer::Texture>> m_MaterialTextures;
};

int main(int argc, char* argv[])
{
    const char* executablePath = (argc > 0 && argv != nullptr) ? argv[0] : nullptr;
    Application::SetExecutablePath(executablePath);

    SandboxApp app;
    app.Run();
    return 0;
}