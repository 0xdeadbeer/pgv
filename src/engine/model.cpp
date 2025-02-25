#include <engine/model.hpp>
#include <assimp/mesh.h>
#include <assimp/vector3.h>
#include <glm/fwd.hpp>
#include <bgfx/bgfx.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ModelComponent::ModelComponent() {
    this->layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .end();
}

int ModelComponent::LoadNode(aiScene *scene, aiNode *node) {
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh *mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            this->vertices.push_back(mesh->mVertices[j].x);
            this->vertices.push_back(mesh->mVertices[j].y);
            this->vertices.push_back(mesh->mVertices[j].z);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
            aiFace *face = &(mesh->mFaces[j]);
            this->indices.insert(this->indices.end(), &face->mIndices[0], &face->mIndices[face->mNumIndices]);
        }
    }

    for (int i = 0; i < node->mNumChildren; ++i) {
        this->LoadNode(scene, node->mChildren[i]);
    }

    return 0;
}

int ModelComponent::LoadModel(std::string filename) {
    Assimp::Importer importer;
    aiScene *scene = (aiScene *) importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "--error: failed loading model from file-system" << std::endl;
        return -1;
    }

    aiNode *root = scene->mRootNode;
    int ret = this->LoadNode(scene, root);
    if (ret < 0) {
        std::cout << "-error: failed loading node tree" << std::endl;
        return -1;
    }

    this->vbh = bgfx::createVertexBuffer(bgfx::makeRef(
                this->vertices.data(),
                this->vertices.size()*sizeof(float)), 
            this->layout);

    this->ibh = bgfx::createIndexBuffer(bgfx::makeRef(
                this->indices.data(),
                this->indices.size()*sizeof(unsigned int)),
            BGFX_BUFFER_INDEX32);

    return 0;
}
