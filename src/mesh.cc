#include "mesh.hh"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "utils.hh"

namespace mygl
{
    bool Mesh::load()
    {
        bool res = false;
        auto importer = Assimp::Importer();
        const auto scene = importer.ReadFile(
            name_,
            aiProcess_Triangulate | aiProcess_GenSmoothNormals
                | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        if (scene)
            res = scene_init(scene);
        else
            std::cout << "Could not initialize scene from " << name_ << '\n';

        return res;
    }

    void Mesh::render()
    {
        std::cout << mesh_entries_.size() << '\n';
        for (size_t i = 0; i < mesh_entries_.size(); ++i)
        {
            glBindVertexArray(mesh_entries_[i].VAO);

            glDrawElements(GL_TRIANGLES, mesh_entries_[i].num_indices,
                           GL_UNSIGNED_INT, (void *)0);
            // glDrawElements(GL_TRIANGLES, 3 * 4, GL_UNSIGNED_SHORT, (void
            // *)0);
        }
        glBindVertexArray(0);
    }
    bool Mesh::scene_init(const aiScene *scene)
    {
        mesh_entries_.resize(scene->mNumMeshes);

        for (size_t i = 0; i < mesh_entries_.size(); ++i)
        {
            const auto mesh = scene->mMeshes[i];
            mesh_init(i, mesh);
        }

        return true;
    }
    void Mesh::mesh_init(unsigned int idx, const aiMesh *mesh)
    {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;
        const aiVector3D zero(0.f, 0.f, 0.f);

        vertices.reserve(mesh->mNumVertices);
        normals.reserve(mesh->mNumVertices);
        uvs.reserve(mesh->mNumVertices);
        indices.reserve(mesh->mNumFaces * 3);

        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            const auto pos = mesh->mVertices + i;
            const auto normal = mesh->mNormals + i;
            const auto tex_coord =
                mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0] + i : &zero;

            vertices.push_back(glm::vec3(pos->x, pos->y, pos->z));
            normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
            uvs.push_back(glm::vec2(tex_coord->x, tex_coord->y));
        }

        for (size_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const auto face = mesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        mesh_entries_[idx].init(vertices, normals, uvs, indices);
    }

    void Mesh::MeshEntry::init(const std::vector<glm::vec3> &vertices,
                               const std::vector<glm::vec3> &normals,
                               const std::vector<glm::vec2> &uvs,
                               const std::vector<unsigned int> &indices)
    {
        num_indices = indices.size();
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLuint vertex_VBO;
        GLuint normal_VBO;
        GLuint uv_VBO;
        GLuint IBO;

        const auto s = sizeof(glm::vec3);

        glGenBuffers(1, &vertex_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * vertices.size(), &(vertices.front()),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &normal_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * normals.size(), &(normals.front()),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &uv_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, uv_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * uvs.size(), &(uvs.front()),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(unsigned int) * indices.size(), &(indices.front()),
                     GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
} // namespace mygl