#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

namespace Bun::Assets
{
    static int Resolve(int idx, usize count)
    {
        if (idx > 0) return idx - 1;
        if (idx < 0) return (int)count + idx;
        return -1;
    }

    static void ParseCounter(const std::string& s, int& p, int& t, int& n)
    {
        p = t = n = 0;
        size_t a = s.find('/');
        p = std::stoi(s.substr(0, a));
        if (a == std::string::npos) return;

        size_t b = s.find('/', a + 1);
        std::string tt = s.substr(a + 1, b == std::string::npos ? std::string::npos : b - a - 1);
        if (!tt.empty()) t = std::stoi(tt);
        if (b != std::string::npos)
        {
            std::string nn = s.substr(b + 1);
            if (!nn.empty()) n = std::stoi(nn);
        }
    }

    static void ParseMtl(const std::string& path, ImportedScene& scene)
    {
        std::ifstream file(path);
        if (!file.is_open()) return;

        ImportedMaterial* cur = nullptr;
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream ss(line);
            std::string tag;
            ss >> tag;

            if (tag == "newmtl")
            {
                std::string name;
                ss >> name;
                cur = nullptr;
                for (auto& m : scene.materials)
                    if (m.name == name) { cur = &m; break; }
            }
            else if (cur && tag == "Kd")
                ss >> cur->baseColor.x >> cur->baseColor.y >> cur->baseColor.z;
            else if (cur && tag == "map_Kd")
            {
                std::getline(ss >> std::ws, cur->diffuseTexture);
            }
        }
    }

    bool ObjLoader::Load(const std::string& path, ImportedScene& out)
    {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        out.meshes.clear();
        out.materials.clear();

        std::vector<Math::Vec3> positions, normals;
        std::vector<Math::Vec2> uvs;

        std::unordered_map<std::string, u32> unique;

        std::unordered_map<std::string, int> materialLookup;

        std::string mtlFile;
        int cur = -1;

        auto startMesh = [&](const std::string& name)
        {
            out.meshes.emplace_back();
            cur = (int)out.meshes.size() - 1;
            out.meshes[cur].name = name;
            unique.clear();
        };

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream ss(line);
            std::string tag;
            ss >> tag;

            if (tag == "v")       { Math::Vec3 p; ss >> p.x >> p.y >> p.z; positions.push_back(p); }
            else if (tag == "vn") { Math::Vec3 n; ss >> n.x >> n.y >> n.z; normals.push_back(n); }
            else if (tag == "vt") { Math::Vec2 t; ss >> t.x >> t.y; t.y = 1.0f - t.y; uvs.push_back(t); }
            else if (tag == "o")
            {
                std::string name;
                ss >> name;
                startMesh(name);
            }
            else if (tag == "usemtl")
            {
                std::string name;
                ss >> name;

                auto it = materialLookup.find(name);
                if (it == materialLookup.end())
                {
                    out.materials.push_back({ name });
                    it = materialLookup.emplace(name, (int)out.materials.size() - 1).first;
                }

                if (cur >= 0 && out.meshes[cur].data.indices.empty())
                    out.meshes[cur].materialIndex = it->second;
            }
            else if (tag == "mtllib") { ss >> mtlFile; }
            else if (tag == "f")
            {
                if (cur < 0) startMesh("default");
                MeshData& mesh = out.meshes[cur].data;

                std::vector<u32> face;
                std::string corner;
                while (ss >> corner)
                {
                    auto it = unique.find(corner);
                    if (it == unique.end())
                    {
                        int pi, ti, ni;
                        ParseCounter(corner, pi, ti, ni);

                        int p = Resolve(pi, positions.size());
                        if (p < 0 || p >= (int)positions.size()) return false;

                        RHI::Vertex v{};
                        v.position = positions[p];

                        int t = ti ? Resolve(ti, uvs.size()) : -1;
                        if (t >= 0 && t < (int)uvs.size()) v.uv = uvs[t];

                        int n = ni ? Resolve(ni, normals.size()) : -1;
                        if (n >= 0 && n < (int)normals.size()) v.normal = normals[n];

                        u32 index = (u32)mesh.vertices.size();
                        mesh.vertices.push_back(v);
                        it = unique.emplace(corner, index).first;
                    }
                    face.push_back(it->second);
                }

                for (usize i = 1; i + 1 < face.size(); i++)
                {
                    mesh.indices.push_back(face[0]);
                    mesh.indices.push_back(face[i]);
                    mesh.indices.push_back(face[i + 1]);
                }
            }
        }
        out.meshes.erase(
            std::remove_if(out.meshes.begin(), out.meshes.end(),
                [](const ImportedMesh& m) { return m.data.indices.empty(); }),
            out.meshes.end());

        if (!mtlFile.empty())
            ParseMtl((std::filesystem::path(path).parent_path() / mtlFile).string(), out);
        return !out.meshes.empty();
    }
}