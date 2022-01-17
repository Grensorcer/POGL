#pragma once
#include <vector>
#include <string>
#include <memory>
#include <array>
#include <optional>
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
        struct MeshEntry
        {
            struct Compute_Info
            {
                glm::vec3 position = glm::vec3(0.);
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

            std::vector<std::array<int, 8>>
            init_neighbours(const std::vector<glm::vec3> &vertices,
                            const std::vector<unsigned int> &indices);
            void
            init_compute(const std::vector<std::array<int, 8>> &neighbour_sets,
                         const std::vector<glm::vec3> &vertices);

            GLuint VAO;
            GLuint vertex_VBO;
            GLuint normal_VBO;
            GLuint neighbour_SSBO;
            GLuint neighbour_distance_SSBO;
            GLuint info_SSBO;

            unsigned int material_index;
            unsigned int num_indices;
            unsigned int num_vertices;
        };

    public:
        Mesh(const char *name)
            : name_{ name }
        {}
        ~Mesh() = default;
        virtual bool load() = 0;
        virtual void render() = 0;
        void render(const std::shared_ptr<program> &program);
        void set_shader(const std::shared_ptr<program> &program);
        std::shared_ptr<program> &get_shader();
        void compute(const program &compute_program,
                     std::optional<size_t> dispatch_size = std::nullopt);
        const std::vector<MeshEntry> &get_entries();
        glm::mat4 &get_world();
        bool is_compute();
        void set_world(glm::mat4 world);

    protected:
        virtual void mesh_init(unsigned int idx, const aiMesh *mesh) = 0;
        bool scene_init(const aiScene *scene);
        bool mat_init(const aiScene *scene);
        bool load_texture_entry(size_t idx, const aiMaterial *const material,
                                aiTextureType type);

#define INVALID_MATERIAL 0xFFFFFFFF

        std::string name_;
        std::vector<std::unique_ptr<Texture>> texture_entries_;
        std::vector<MeshEntry> mesh_entries_;
        std::shared_ptr<program> shader_;
        glm::mat4 world_ = glm::mat4(1.0f);

        bool compute_ = false;
    };

    class TriangleMesh : public Mesh
    {
    public:
        using Mesh::Mesh;
        bool load();
        void render();
        void render(const mygl::program &program);

    protected:
        void mesh_init(unsigned int idx, const aiMesh *mesh);
    };

    class QuadMesh : public Mesh
    {
    public:
        using Mesh::Mesh;
        bool load();
        void render();
        void render(const mygl::program &program);

    protected:
        void mesh_init(unsigned int idx, const aiMesh *mesh);
    };
} // namespace mygl
