//
// Created by cilan on 7/1/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#ifndef OGL_PRACTICE_MESH_H
#define OGL_PRACTICE_MESH_H

#include <GL/glew.h>

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "Mesh.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <chrono>

struct Vertex
{
    glm::vec3 m_pos;
    glm::vec2 m_tex;

    Vertex() {}

    Vertex(glm::vec3 pos, glm::vec2 tex)
    {
        m_pos = pos;
        m_tex = tex;
    }
};

class Mesh
{
public:
    Mesh();
    ~Mesh() {};

    bool loadMesh(const std::string& path);
    bool initFromScene(const aiScene* pScene);
    void render();
    void draw();
private:
    GLuint texture_handle;
    Assimp::Importer importer;
    const aiMesh* meshy;
    aiString texturePath;
    int numFaces;
};

#endif
