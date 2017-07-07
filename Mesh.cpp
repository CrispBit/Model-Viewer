//
// Created by cilan on 7/1/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#include "Mesh.h"

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <glm/gtx/transform.hpp>
#include <assimp/postprocess.h>
#include <chrono>

Mesh::MeshEntry::MeshEntry()
{
    VB = INVALID_OGL_VALUE;
    IB = INVALID_OGL_VALUE;
    numIndices  = 0;
    materialIndex = INVALID_MATERIAL;
};

Mesh::MeshEntry::MeshEntry(MeshEntry&& m) : VB(m.VB), IB(m.IB),
                    numIndices(m.numIndices), materialIndex(m.materialIndex){
    m.VB = INVALID_OGL_VALUE;
    m.IB = INVALID_OGL_VALUE;
    m.numIndices  = 0;
    m.materialIndex = INVALID_MATERIAL;
}

Mesh::MeshEntry::~MeshEntry()
{
    if (VB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &VB);
    }

    if (IB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &IB);
    }
}

bool Mesh::MeshEntry::Init(const std::vector<Vertex>& Vertices,
                           const std::vector<GLuint>& Indices)
{
    numIndices = Indices.size();

    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, &Indices[0], GL_STATIC_DRAW);

    return true;
}

Mesh::Mesh() {
    // do nothing
}

bool Mesh::loadMesh(const std::string& path) {
    bool ret = false;

    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (pScene) {
        ret = initFromScene(pScene);
    } else {
        std::cout << "SOMETHING JBVADD HAPPENED!@!!@!!!!" << std::endl;
    }

    return ret;
}

bool Mesh::initMaterials(const aiScene* pScene) {
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
        aiString texturePath;
        pScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL);
        if (std::string(texturePath.C_Str()).empty()) {
            texturePath = aiString("assets/white.png");
        }

        std::unique_ptr<Texture> texture = std::make_unique<Texture>(GL_TEXTURE_2D, texturePath.C_Str());
        texture->load();

        m_Textures.push_back(std::move(texture));
    }
    return false;
}

bool Mesh::initFromScene(const aiScene* pScene) {
    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        aiMesh* meshy = pScene->mMeshes[i];
        std::unique_ptr<MeshEntry> entry = std::make_unique<MeshEntry>();
        std::vector<Vertex> vaortishes;
        std::vector<GLuint> indexs;
        unsigned int vertices = meshy->mNumVertices;
        for (unsigned int j = 0; j < vertices; ++j) {
            const aiVector3D &potato = meshy->mVertices[j];
            //const aiVector3D &n = meshy->mNormals[j];
            const aiVector3D &tc = meshy->HasTextureCoords(0) ? meshy->mTextureCoords[0][j] : aiVector3D(0,0,0);
            vaortishes.push_back(Vertex(glm::vec3(potato.x, potato.y, potato.z), glm::vec2(tc.x, tc.y)));
        }

        for (unsigned int j = 0; j < meshy->mNumFaces; ++j) {
            const aiFace& fase = meshy->mFaces[j];
            assert(fase.mNumIndices == 3);
            indexs.push_back(fase.mIndices[0]);
            indexs.push_back(fase.mIndices[1]);
            indexs.push_back(fase.mIndices[2]);
        }

        entry->materialIndex = meshy->mMaterialIndex;

        entry->Init(vaortishes, indexs);

        m_Entries.push_back(std::move(entry));
    }

    /* this->meshy = pScene->mMeshes[0];
    pScene->mMaterials[this->meshy->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL);

    sf::Image img_data;
    if (!img_data.loadFromFile(texturePath.C_Str())) {
        std::cout << "Could not load " <<  texturePath.C_Str() << std::endl;
    }
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA,
            img_data.getSize().x, img_data.getSize().y,
            0,
            GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr()
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vaortishes.size(), &vaortishes[0], GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, vaortishes.size() * sizeof(glm::vec3), &vaortishes[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);                 // position
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); // texture coordinate
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    GLuint indexbufferthingo;
    glGenBuffers(1, &indexbufferthingo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferthingo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(unsigned int), &indexs[0], GL_STATIC_DRAW);
    numFaces = meshy->mNumFaces; */

    return initMaterials(pScene);
}

void Mesh::draw() {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    for (const auto &mesh : m_Entries) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VB);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);                 // position
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); // texture coordinate

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IB);

        m_Textures[mesh->materialIndex]->bind(GL_TEXTURE0);
        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
    }

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    // check OpenGL error
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}
