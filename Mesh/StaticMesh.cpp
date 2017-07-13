//
// Created by cilan on 7/12/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#include "StaticMesh.h"

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <glm/gtx/transform.hpp>
#include <assimp/postprocess.h>
#include <chrono>

StaticMesh::MeshEntry::MeshEntry()
{
    numIndices = 0;
    materialIndex = INVALID_MATERIAL;
};

StaticMesh::StaticMesh() {
    m_VAO = 0;
    memset(&m_Buffers, 0, sizeof(m_Buffers)); // m'buffers *tips hat*
}

bool StaticMesh::loadMesh(const std::string& path) {
    bool ret = false;

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(sizeof(m_Buffers) / sizeof(*m_Buffers), m_Buffers);

    m_pScene = m_importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (m_pScene) {
        m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform.Inverse();
        ret = initFromScene(m_pScene);
    } else {
        std::cout << m_importer.GetErrorString() << std::endl;
    }

    glBindVertexArray(0);

    return ret;
}

bool StaticMesh::initMaterials(const aiScene* pScene) {
    for (unsigned int i = 0; i < pScene->mNumMaterials; ++i) {
        aiString texturePath;
        pScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL);
        if (std::string(texturePath.C_Str()).empty()) {
            texturePath = aiString("white.png");
        }

        Texture texture = Texture(GL_TEXTURE_2D, std::string("assets/") + texturePath.C_Str());
        texture.load();

        m_Textures[i] = std::move(texture);
    }
    return true;
}

bool StaticMesh::initFromScene(const aiScene* pScene) {
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;

    unsigned int numVertices = 0,
                 numIndices = 0;

    for (unsigned int i = 0; i < m_Entries.size(); ++i) {
        MeshEntry entry;
        entry.materialIndex = pScene->mMeshes[i]->mMaterialIndex;
        entry.numIndices = pScene->mMeshes[i]->mNumFaces * 3;

        numVertices += pScene->mMeshes[i]->mNumVertices;
        numIndices += entry.numIndices;

        m_Entries[i] = std::move(entry);
    }

    positions.reserve(numVertices);
    normals.reserve(numVertices);
    texCoords.reserve(numVertices);
    normals.reserve(numVertices);
    indices.reserve(numIndices);

    const aiVector3D zero3D(0.0f, 0.0f, 0.0f);
    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        const aiMesh* meshy = pScene->mMeshes[i];
        unsigned int vertices = meshy->mNumVertices;
        for (unsigned int j = 0; j < vertices; ++j) {
            const aiVector3D &pos = meshy->mVertices[j];
            const aiVector3D &tc = meshy->HasTextureCoords(0) ? meshy->mTextureCoords[0][j] : zero3D;
            const aiVector3D &n = meshy->mNormals[j];
            positions.push_back(glm::vec3(pos.x, pos.y, pos.z));
            texCoords.push_back(glm::vec2(tc.x, tc.y));
            normals.push_back(glm::vec3(n.x, n.y, n.z));
        }

        for (unsigned int j = 0; j < meshy->mNumFaces; ++j) {
            const aiFace& fase = meshy->mFaces[j];
            assert(fase.mNumIndices == 3);
            indices.push_back(fase.mIndices[0]);
            indices.push_back(fase.mIndices[1]);
            indices.push_back(fase.mIndices[2]);
        }
    }

    if (!initMaterials(pScene)) {
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    return glGetError() == GL_NO_ERROR;
}

void StaticMesh::draw() {
    glBindVertexArray(m_VAO);

    for (const auto &mesh : m_Entries) {
        m_Textures[mesh.materialIndex].bind(GL_TEXTURE0);
        glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}
