#include "mesh.hh"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "utils.hh"

namespace mygl
{
    void Mesh::set_shader(const std::shared_ptr<program> &program)
    {
        shader_ = program;
    }

    std::shared_ptr<program> &Mesh::get_shader()
    {
        return shader_;
    }

    void Mesh::compute(const program &compute_program,
                       std::optional<size_t> dispatch_size)
    {
        if (!compute_)
            return;
        compute_program.use();
        for (const auto &mesh_entry : mesh_entries_)
        {
            compute_program.set_uint("nb_vertices", mesh_entry.num_vertices);
            glBindVertexArray(mesh_entry.VAO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1,
                             mesh_entry.vertex_VBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2,
                             mesh_entry.normal_VBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3,
                             mesh_entry.neighbour_SSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4,
                             mesh_entry.neighbour_vertex_SSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5,
                             mesh_entry.neighbour_distance_SSBO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, mesh_entry.info_SSBO);
            size_t dispatch = mesh_entry.num_vertices;
            if (dispatch_size.has_value())
                dispatch = dispatch_size.value() > dispatch
                    ? dispatch_size.value()
                    : dispatch;
            glDispatchCompute(dispatch / 1024 + 1, 1, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Mesh::render(const std::shared_ptr<program> &program)
    {
        auto tmp = this->shader_;
        this->set_shader(program);
        this->render();
        this->set_shader(tmp);
    }

    glm::mat4 &Mesh::get_world()
    {
        return world_;
    }

    bool Mesh::is_compute()
    {
        return compute_;
    }

    const std::vector<Mesh::MeshEntry> &Mesh::get_entries()
    {
        return mesh_entries_;
    }

    void Mesh::set_world(glm::mat4 world)
    {
        world_ = world;
    }

    bool Mesh::scene_init(const aiScene *scene)
    {
        mesh_entries_.resize(scene->mNumMeshes);
        texture_entries_.resize(scene->mNumMaterials * 3);

        for (size_t i = 0; i < mesh_entries_.size(); ++i)
        {
            const auto mesh = scene->mMeshes[i];
            mesh_init(i, mesh);
        }

        return mat_init(scene);
    }

    bool Mesh::load_texture_entry(size_t idx, const aiMaterial *const material,
                                  aiTextureType type)
    {
        aiString path;
        bool res = true;
        if (material->GetTexture(type, 0, &path, NULL, NULL, NULL, NULL, NULL)
            == AI_SUCCESS)
        {
            texture_entries_[idx] = std::make_unique<Texture>(path.data);

            if (!texture_entries_[idx]->load())
            {
                std::cerr << "Error loading texture " << path.data << '\n';
                texture_entries_[idx].reset(nullptr);
                res = false;
            }
        }
        else
        {
            std::cerr << "Could not GetTexture for idx " << idx << '\n';
            res = false;
        }

        return res;
    }

    bool Mesh::mat_init(const aiScene *scene)
    {
        bool res = true;

        for (size_t i = 0; i < scene->mNumMaterials; ++i)
        {
            const auto material = scene->mMaterials[i];
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
                res &=
                    load_texture_entry(i * 3, material, aiTextureType_DIFFUSE);
            else
            {
                texture_entries_[i * 3] =
                    std::make_unique<Texture>("../data/texture/white.png");
                texture_entries_[i * 3]->load();
            }

            if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
                res &= load_texture_entry(i * 3 + 1, material,
                                          aiTextureType_HEIGHT);
            else
            {
                texture_entries_[i * 3 + 1] = std::make_unique<Texture>(
                    "../data/texture/normal_white.png");
                texture_entries_[i * 3 + 1]->load();
            }

            if (material->GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
                res &= load_texture_entry(i * 3 + 2, material,
                                          aiTextureType_DISPLACEMENT);
            else
            {
                texture_entries_[i * 3 + 2] =
                    std::make_unique<Texture>("../data/texture/white.png");
                texture_entries_[i * 3 + 2]->load();
            }
        }

        return res;
    }

    void Mesh::MeshEntry::init(const std::vector<glm::vec3> &vertices,
                               const std::vector<glm::vec3> &normals,
                               const std::vector<glm::vec3> &tangents,
                               const std::vector<glm::vec2> &uvs,
                               const std::vector<unsigned int> &indices)
    {
        num_indices = indices.size();
        num_vertices = vertices.size();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        GLuint tangent_VBO;
        GLuint uv_VBO;
        GLuint IBO;

        const auto s = sizeof(glm::vec3);

        glGenBuffers(1, &vertex_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * vertices.size(), &(vertices.front()),
                     GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &normal_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * normals.size(), &(normals.front()),
                     GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &tangent_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, tangent_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * tangents.size(), &(tangents.front()),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &uv_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, uv_VBO);
        glBufferData(GL_ARRAY_BUFFER, s * uvs.size(), &(uvs.front()),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(unsigned int) * indices.size(), &(indices.front()),
                     GL_STATIC_DRAW);

        // glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        // glBufferData(GL_SHADER_STORAGE_BUFFER, s * vertices.size(),
        //              &(vertices.front()), GL_STATIC_DRAW);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertex_VBO);

        // glBufferData(GL_SHADER_STORAGE_BUFFER, s * normals.size(),
        //              &(normals.front()), GL_STATIC_DRAW);
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, normal_VBO);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }

    std::vector<std::array<int, 8>>
    Mesh::MeshEntry::init_neighbours(const std::vector<glm::vec3> &vertices,
                                     const std::vector<unsigned int> &indices)
    {
        glBindVertexArray(VAO);
        std::vector<int> neighbour_indices(8 * vertices.size(), -1);
        std::vector<float> neighbour_distances(8 * vertices.size(), 0);
        std::vector<std::array<int, 8>> neighbour_indices_;

        neighbour_indices_.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i)
            for (short j = 0; j < 8; ++j)
                neighbour_indices_[i][j] = -1;

        if (indices.back() == indices.size() - 1)
            std::cout << "WARNING: Seems like this quad mesh has no "
                         "repeated index\n";
        // std::cout << "TOTAL SIZE: " << indices.size() << '\n';
        for (size_t i = 0; i < indices.size(); i += 4)
        {
            auto i0 = indices[i];
            auto i1 = indices[i + 1];
            auto i2 = indices[i + 2];
            auto i3 = indices[i + 3];

            neighbour_indices_[i0][3] = i1;
            neighbour_indices_[i0][4] = i2;
            neighbour_indices_[i0][5] = i3;

            neighbour_indices_[i1][7] = i0;
            neighbour_indices_[i1][5] = i2;
            neighbour_indices_[i1][6] = i3;

            neighbour_indices_[i2][0] = i0;
            neighbour_indices_[i2][1] = i1;
            neighbour_indices_[i2][7] = i3;

            neighbour_indices_[i3][1] = i0;
            neighbour_indices_[i3][2] = i1;
            neighbour_indices_[i3][3] = i2;
        }

        for (size_t i = 0; i < neighbour_indices_.size(); ++i)
        {
            for (short j = 0; j < 8; ++j)
            {
                auto idx = neighbour_indices_[i][j];
                neighbour_indices[i * 8 + j] = idx;
                if (idx != -1)
                    neighbour_distances[i * 8 + j] =
                        glm::distance(vertices[i], vertices[idx]);
            }
        }

        glGenBuffers(1, &neighbour_SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbour_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     sizeof(int) * neighbour_indices.size(),
                     neighbour_indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        glGenBuffers(1, &neighbour_vertex_SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbour_vertex_SSBO);
        std::vector<glm::vec3> dummy{ neighbour_distances.size(),
                                      glm::vec3(0) };
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     sizeof(glm::vec3) * neighbour_distances.size(),
                     dummy.data(), GL_DYNAMIC_DRAW);

        glGenBuffers(1, &neighbour_distance_SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, neighbour_distance_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     sizeof(float) * neighbour_distances.size(),
                     neighbour_distances.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);

        return neighbour_indices_;
    }

    void Mesh::MeshEntry::init_compute(
        const std::vector<std::array<int, 8>> &neighbour_indices,
        const std::vector<glm::vec3> &vertices)
    {
        glBindVertexArray(VAO);
        glGenBuffers(1, &info_SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, info_SSBO);

        auto info = std::vector<Compute_Info>(neighbour_indices.size());
        for (size_t i = 0; i < neighbour_indices.size(); ++i)
        {
            info[i].position = vertices[i];
            info[i].pinned = false;
            float pin_limit = 4.7;
            if (vertices[i].z < -pin_limit &&
                (vertices[i].x > pin_limit || vertices[i].x < -pin_limit))
                info[i].pinned = true;
        }

        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     sizeof(Compute_Info) * info.size(), &(info.front()),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }

    bool TriangleMesh::load()
    {
        bool res = false;
        auto importer = Assimp::Importer();
        const auto scene = importer.ReadFile(
            name_,
            aiProcess_Triangulate | aiProcess_GenSmoothNormals
                | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices
                | aiProcess_CalcTangentSpace);

        if (scene)
            res = scene_init(scene);
        else
            std::cerr << "Could not initialize scene from " << name_ << '\n';

        return res;
    }

    void TriangleMesh::render()
    {
        shader_->use();
        for (const auto &mesh_entry : mesh_entries_)
        {
            glBindVertexArray(mesh_entry.VAO);
            texture_entries_[mesh_entry.material_index * 3]->bind(GL_TEXTURE0);
            texture_entries_[mesh_entry.material_index * 3 + 1]->bind(
                GL_TEXTURE1);
            texture_entries_[mesh_entry.material_index * 3 + 2]->bind(
                GL_TEXTURE2);

            glDrawElements(GL_TRIANGLES, mesh_entry.num_indices,
                           GL_UNSIGNED_INT, (void *)0);
        }
        glBindVertexArray(0);
    }

    void TriangleMesh::mesh_init(unsigned int idx, const aiMesh *mesh)
    {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;
        const aiVector3D zero(0.f, 0.f, 0.f);

        vertices.reserve(mesh->mNumVertices);
        normals.reserve(mesh->mNumVertices);
        tangents.reserve(mesh->mNumVertices);
        uvs.reserve(mesh->mNumVertices);
        indices.reserve(mesh->mNumFaces * 3);

        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            const auto pos = mesh->mVertices + i;
            const auto normal = mesh->mNormals + i;
            const auto tangent = mesh->mTangents + i;
            const auto tex_coord =
                mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0] + i : &zero;

            vertices.push_back(glm::vec3(pos->x, pos->y, pos->z));
            normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
            tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));
            uvs.push_back(glm::vec2(tex_coord->x, tex_coord->y));
        }

        for (size_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const auto face = mesh->mFaces[i];
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        mesh_entries_[idx].material_index = mesh->mMaterialIndex;
        mesh_entries_[idx].init(vertices, normals, tangents, uvs, indices);
    }

    bool QuadMesh::load()
    {
        compute_ = true;

        bool res = false;
        auto importer = Assimp::Importer();
        const auto scene = importer.ReadFile(
            name_,
            aiProcess_GenSmoothNormals | aiProcess_FlipUVs
                | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

        if (scene)
            res = scene_init(scene);
        else
            std::cerr << "Could not initialize scene from " << name_ << '\n';

        return res;
    }

    void QuadMesh::render()
    {
        shader_->use();
        for (const auto &mesh_entry : mesh_entries_)
        {
            glBindVertexArray(mesh_entry.VAO);
            texture_entries_[mesh_entry.material_index * 3]->bind(GL_TEXTURE0);
            texture_entries_[mesh_entry.material_index * 3 + 1]->bind(
                GL_TEXTURE1);
            texture_entries_[mesh_entry.material_index * 3 + 2]->bind(
                GL_TEXTURE2);

            glPatchParameteri(GL_PATCH_VERTICES, 4);
            glDrawElements(GL_PATCHES, mesh_entry.num_indices, GL_UNSIGNED_INT,
                           (void *)0);
        }
        glBindVertexArray(0);
    }

    void QuadMesh::mesh_init(unsigned int idx, const aiMesh *mesh)
    {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;
        const aiVector3D zero(0.f, 0.f, 0.f);

        vertices.reserve(mesh->mNumVertices);
        normals.reserve(mesh->mNumVertices);
        tangents.reserve(mesh->mNumVertices);
        uvs.reserve(mesh->mNumVertices);
        indices.reserve(mesh->mNumFaces * 4);

        for (size_t i = 0; i < mesh->mNumVertices; ++i)
        {
            const auto pos = mesh->mVertices + i;
            const auto normal = mesh->mNormals + i;
            const auto tangent = mesh->mTangents + i;
            const auto tex_coord =
                mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0] + i : &zero;

            vertices.push_back(glm::vec3(pos->x, pos->y, pos->z));
            normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
            tangents.push_back(glm::vec3(tangent->x, tangent->y, tangent->z));
            uvs.push_back(glm::vec2(tex_coord->x, tex_coord->y));
        }

        for (size_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const auto face = mesh->mFaces[i];
            if (face.mNumIndices != 4)
                std::cerr << "Quad mesh has wrong number of face indices: "
                          << face.mNumIndices << '\n';

            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
            indices.push_back(face.mIndices[3]);
            // std::cout << face.mIndices[0] << " " << face.mIndices[1] << " "
            //          << face.mIndices[2] << " " << face.mIndices[3] << "\n";
        }

        mesh_entries_[idx].material_index = mesh->mMaterialIndex;
        mesh_entries_[idx].init(vertices, normals, tangents, uvs, indices);
        auto neighbour_sets =
            mesh_entries_[idx].init_neighbours(vertices, indices);
        mesh_entries_[idx].init_compute(neighbour_sets, vertices);
    }

} // namespace mygl
