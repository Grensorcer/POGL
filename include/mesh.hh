#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <assimp/scene.h>

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
        void render();

    private:
        bool scene_init(const aiScene *scene);
        void mesh_init(unsigned int idx, const aiMesh *mesh);

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
                      const std::vector<glm::vec2> &uvs,
                      const std::vector<unsigned int> &indices);

            GLuint VAO;
            unsigned int num_indices;
        };

        const char *name_;
        std::vector<MeshEntry> mesh_entries_;
    };
} // namespace mygl