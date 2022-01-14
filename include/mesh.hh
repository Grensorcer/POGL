#pragma once
#include <vector>
#include <string>
#include <memory>
#include <set>
#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <assimp/scene.h>
#include "texture.hh"
#include "program.hh"

namespace mygl
{
    class Mesh
    {
    public:
        Mesh(const char *name)
            : name_{ name }
        {}
        ~Mesh() = default;
        virtual bool load() = 0;
        virtual void render(const mygl::program &program) = 0;
        void compute(const mygl::program &compute_program);

    protected:
        virtual void mesh_init(unsigned int idx, const aiMesh *mesh) = 0;
        bool scene_init(const aiScene *scene);
        bool mat_init(const aiScene *scene);
        bool load_texture_entry(size_t idx, const aiMaterial *const material,
                                aiTextureType type);

#define INVALID_MATERIAL 0xFFFFFFFF

        struct MeshEntry
        {
            struct Compute_Info
            {
                glm::vec3 speed = glm::vec3(0.);
                float pinned = false;
            };

            MeshEntry()
            {
                glGenVertexArrays(1, &VAO);
            }
            ~MeshEntry()
            {
                glDeleteVertexArrays(1, &VAO);
            }

            void init(const std::vector<glm::vec3> &vertices,
                      const std::vector<glm::vec3> &normals,
                      const std::vector<glm::vec3> &tangents,
                      const std::vector<glm::vec2> &uvs,
                      const std::vector<unsigned int> &indices);

            std::vector<std::set<int>>
            init_neighbours(const std::vector<glm::vec3> &vertices,
                            const std::vector<unsigned int> &indices);
            void init_compute(const std::vector<std::set<int>> &vertices);

            GLuint VAO;
            GLuint vertex_VBO;
            GLuint normal_VBO;
            GLuint SSBO;

            unsigned int material_index;
            unsigned int num_indices;
            unsigned int num_vertices;
        };

        std::string name_;
        std::vector<std::unique_ptr<Texture>> texture_entries_;
        std::vector<MeshEntry> mesh_entries_;
    };

    class TriangleMesh : public Mesh
    {
    public:
        using Mesh::Mesh;
        bool load();
        void render(const mygl::program &program);

    protected:
        void mesh_init(unsigned int idx, const aiMesh *mesh);
    };

    class QuadMesh : public Mesh
    {
    public:
        using Mesh::Mesh;
        bool load();
        void render(const mygl::program &program);

    protected:
        void mesh_init(unsigned int idx, const aiMesh *mesh);
    };
} // namespace mygl