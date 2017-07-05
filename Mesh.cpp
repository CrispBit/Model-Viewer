//
// Created by cilan on 7/1/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#include "Mesh.h"
#include <GL/glew.h>

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <chrono>

Mesh::Mesh() {
    // do nothing
}

bool Mesh::loadMesh(const std::string& path) {
    bool ret = false;

    const aiScene* pScene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (pScene) {
        ret = initFromScene(pScene);
    } else {
        std::cout << "SOMETHING JBVADD HAPPENED!@!!@!!!!" << std::endl;
    }

    return ret;
}

bool Mesh::initFromScene(const aiScene* pScene) {
    this->meshy = pScene->mMeshes[0];
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

    std::vector<Vertex> vaortishes;
    std::vector<unsigned int> indexs;
    unsigned int vertices = this->meshy->mNumVertices;
    for (unsigned int i = 0; i < vertices; ++i) {
        const aiVector3D &potato = this->meshy->mVertices[i];
        //const aiVector3D &n = meshy->mNormals[i];
        const aiVector3D &tc = this->meshy->HasTextureCoords(0) ? this->meshy->mTextureCoords[0][i] : aiVector3D(0,0,0);
        vaortishes.push_back(Vertex(glm::vec3(potato.x, potato.y, potato.z), glm::vec2(tc.x, tc.y)));
    }

    for (unsigned int i = 0; i < meshy->mNumFaces; ++i) {
        const aiFace & fase = meshy->mFaces[i];
        assert(fase.mNumIndices == 3);
        indexs.push_back(fase.mIndices[0]);
        indexs.push_back(fase.mIndices[1]);
        indexs.push_back(fase.mIndices[2]);
    }

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
    numFaces = meshy->mNumFaces;


    return true;
}

void Mesh::draw() {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glDrawElements(GL_TRIANGLES, numFaces * 3 * 3, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
