//
// Created by cilan on 7/1/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#ifndef OGL_BONEDMESH_H
#define OGL_BONEDMESH_H

#include "IMesh.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <vector>
#include "Texture.h"

#define INVALID_OGL_VALUE 0xFFFFFFFF

class BonedMesh : public Mesh
{
public:
    BonedMesh();

    void boneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms);
    bool loadMesh(const std::string& path) override;
    bool initFromScene(const aiScene* pScene) override;
    void draw() override;
private:
#define INVALID_MATERIAL 0xFFFFFFFF

enum VB_TYPES {
    POS_VB,
    TEXCOORD_VB,
    NORMAL_VB,
    BONE_VB,
    INDEX_BUFFER,
    NUM_VBs
};

    GLuint m_VAO;
    GLuint m_Buffers[NUM_VBs];

    unsigned int findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    unsigned int findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
    unsigned int findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

    bool initMaterials(const aiScene* pScene);
    const aiScene* m_pScene;
    Assimp::Importer m_importer;
    aiMatrix4x4 m_GlobalInverseTransform;

    struct VertexBoneData {
        GLuint ids[8] = {0};
        GLfloat weights[8] = {0}; // same length as ids

        void addBoneData(GLuint boneID, GLfloat weight);
    };

    struct MeshEntry {
        MeshEntry();

        GLuint numIndices,
               materialIndex,
               baseVertex,
               baseIndex;
    };

    struct BoneInfo {
        glm::mat4 boneOffset;
        glm::mat4 finalTransformation;
    };

    GLuint numBones = 0;
    std::map<std::string, GLuint> m_boneMapping;
    std::vector<BoneInfo> m_boneInfo;
    glm::mat4 m_globalInverseTransform;

    std::vector<MeshEntry> m_Entries;
    std::vector<Texture> m_Textures;
};

#endif
