#pragma once
#include <vector>
#include <string>
#include <memory>
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
        bool load();
        void render(const mygl::program &program);
        void compute(const mygl::program &compute_program);

    private:
        bool scene_init(const aiScene *scene);
        void mesh_init(unsigned int idx, const aiMesh *mesh);
        bool mat_init(const aiScene *scene);
        bool load_texture_entry(size_t idx, const aiMaterial *const material,
                                aiTextureType type);

#define INVALID_MATERIAL 0xFFFFFFFF

        struct MeshEntry
        {
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

            GLuint VAO;
            GLuint vertex_VBO;
            GLuint normal_VBO;
            GLuint SSBO;

            unsigned int material_index;
            unsigned int num_indices;
        };

        std::string name_;
        std::vector<std::unique_ptr<Texture>> texture_entries_;
        std::vector<MeshEntry> mesh_entries_;
    };
} // namespace mygl