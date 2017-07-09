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
    GLuint mID;

    Vertex() {}

    Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal, const GLuint& mID) : m_pos(pos), m_tex(tex), m_normal(normal), mID(mID)
    {
        // do nothing
    }
};

class Mesh
{
public:
    Mesh();
    ~Mesh() {};

    void boneTransform(float TimeInSeconds, std::vector<std::vector<glm::mat4>>& Transforms);
    bool loadMesh(const std::string& path);
    bool initFromScene(const aiScene* pScene);
    void draw();
private:
    #define INVALID_MATERIAL 0xFFFFFFFF

    unsigned int findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
    unsigned int findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform, unsigned int mID);
    unsigned int findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
    void calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
    const aiNodeAnim* findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);

    bool initMaterials(const aiScene* pScene);
    const aiScene* m_pScene;
    Assimp::Importer m_importer;
    aiMatrix4x4 m_GlobalInverseTransform;
    std::vector<std::map<std::string, unsigned int>> m_boneMapping;

    struct VertexBoneData
    {
        unsigned int ids[4];
        float weights[4]; // same length as ids

        void addBoneData(unsigned int boneID, float weight);

        VertexBoneData() {
            memset(&ids, 0, sizeof(ids));
            memset(&weights, 0, sizeof(weights));
        }
    };

    struct MeshEntry{
        MeshEntry();
        ~MeshEntry();

        bool Init(const std::vector<Vertex>& Vertices,
                  const std::vector<GLuint>& Indices,
                  const std::vector<VertexBoneData>& bone);

        GLuint VB;
        GLuint IB;

        GLuint bVB;

        GLuint numIndices;
        unsigned int materialIndex;
    };

    struct BoneInfo
    {
        glm::mat4 boneOffset;
        glm::mat4 finalTransformation;
    };

    std::vector<std::vector<BoneInfo>> m_boneInfo;
    glm::mat4 m_globalInverseTransform;

    std::vector<std::unique_ptr<MeshEntry>> m_Entries;
    std::vector<std::unique_ptr<Texture>> m_Textures;
};

#endif
