//
// Created by cilan on 7/12/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#ifndef OGL_STATICMESH_H
#define OGL_STATICMESH_H

#include "IMesh.h"
#include "Texture.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>

#define INVALID_OGL_VALUE 0xFFFFFFFF

class StaticMesh : public Mesh
{
public:
    StaticMesh();

    bool loadMesh(const std::string& path) override;
    bool initFromScene(const aiScene* pScene) override;
    void draw() override;
private:
#define INVALID_MATERIAL 0xFFFFFFFF

enum VB_TYPES {
    POS_VB,
    TEXCOORD_VB,
    NORMAL_VB,
    NUM_VBs
};

    GLuint m_VAO;
    GLuint m_Buffers[NUM_VBs];

    bool initMaterials(const aiScene* pScene);
    const aiScene* m_pScene;
    Assimp::Importer m_importer;
    aiMatrix4x4 m_GlobalInverseTransform;

    struct MeshEntry {
        MeshEntry();

        GLuint numIndices,
               materialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<Texture> m_Textures;
};

#endif
