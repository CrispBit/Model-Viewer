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
    glm::vec3 m_normal;

    Vertex() {}

    Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal) : m_pos(pos), m_tex(tex), m_normal(normal)
    {
        // do nothing
    }
};

class Mesh
{
public:
    Mesh();
    ~Mesh() {};

    glm::mat4 boneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms);
    bool loadMesh(const std::string& path);
    bool initFromScene(const aiScene* pScene);
    void draw();
private:
    #define INVALID_MATERIAL 0xFFFFFFFF
    #define NUM_BONES_PER_VEREX 4

    unsigned int findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    unsigned int findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
    unsigned int findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

    bool initMaterials(const aiScene* pScene);
    const aiScene* m_pScene;
    Assimp::Importer m_importer;
    aiMatrix4x4 m_GlobalInverseTransform;
    std::map<std::string, unsigned int> m_boneMapping;

    unsigned int m_numBones = 0;
    struct VertexBoneData
    {
        unsigned int ids[NUM_BONES_PER_VEREX];
        float weights[NUM_BONES_PER_VEREX];

        void addBoneData(unsigned int boneID, float weight);

        VertexBoneData() {
            memset(&ids, 0, sizeof(ids));
            memset(&weights, 0, sizeof(weights));
        }
    };

    enum VB_TYPES {
        INDEX_BUFFER,
        POS_VB,
        NORMAL_VB,
        TEXCOORD_VB,
        BONE_VB,
        NUM_VBs
    };

    GLuint m_VAO;
    GLuint m_buffers[NUM_VBs];

    struct MeshEntry{
        MeshEntry();

        unsigned int numIndices;
        unsigned int baseVertex;
        unsigned int baseIndex;
        unsigned int materialIndex;
    };

    struct BoneInfo
    {
        glm::mat4 boneOffset;
        glm::mat4 finalTransformation;
    };

    std::vector<BoneInfo> m_boneInfo;
    glm::mat4 m_globalInverseTransform;

    std::vector<std::unique_ptr<MeshEntry>> m_Entries;
    std::vector<std::unique_ptr<Texture>> m_Textures;
};

#endif
