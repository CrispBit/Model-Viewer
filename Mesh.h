//
// Created by cilan on 7/1/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#ifndef OGL_PRACTICE_MESH_H
#define OGL_PRACTICE_MESH_H

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include <memory>
#include "Texture.h"

#define INVALID_OGL_VALUE 0xFFFFFFFF

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
    void draw();
private:
    #define INVALID_MATERIAL 0xFFFFFFFF

    bool initMaterials(const aiScene* pScene);

    struct MeshEntry{
        MeshEntry();
        ~MeshEntry();

        bool Init(const std::vector<Vertex>& Vertices,
                  const std::vector<GLuint>& Indices);

        GLuint VB;
        GLuint IB;

        GLuint numIndices;
        unsigned int materialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<std::unique_ptr<Texture>> m_Textures;
};

#endif
