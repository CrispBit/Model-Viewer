//
// Created by cilan on 7/1/2017.
// From tutorial: http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//

#include "BonedMesh.h"

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <glm/gtx/transform.hpp>
#include <assimp/postprocess.h>
#include <chrono>

BonedMesh::MeshEntry::MeshEntry()
{
    numIndices = 0;
    baseVertex = 0;
    baseIndex = 0;
    materialIndex = INVALID_MATERIAL;
};

BonedMesh::BonedMesh() {
    m_VAO = 0;
    memset(&m_Buffers, 0, sizeof(m_Buffers)); // m'buffers *tips hat*
}

bool BonedMesh::loadMesh(const std::string& path) {
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

bool BonedMesh::initMaterials(const aiScene* pScene) {
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

inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;

    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

    return to;
}

unsigned int BonedMesh::findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (unsigned int i = 0 ; i < pNodeAnim->mNumRotationKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);
    return 0;
}

void BonedMesh::calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1) {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    unsigned int RotationIndex = findRotation(AnimationTime, pNodeAnim);
    unsigned int NextRotationIndex = (RotationIndex + 1);
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float deltaTime = pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime;
    float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / deltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = Out.Normalize();
}

unsigned int BonedMesh::findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (unsigned int i = 0 ; i < pNodeAnim->mNumScalingKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);

    return 0;
}

void BonedMesh::calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1) {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    unsigned int ScalingIndex = findScaling(AnimationTime, pNodeAnim);
    unsigned int NextScalingIndex = (ScalingIndex + 1);
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D& End   = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

unsigned int BonedMesh::findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (unsigned int i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);
    return 0;
}

void BonedMesh::calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1) {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    unsigned int PositionIndex = findPosition(AnimationTime, pNodeAnim);
    unsigned int NextPositionIndex = (PositionIndex + 1);
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
    float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

void BonedMesh::ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
    std::string NodeName(pNode->mName.data);

    const aiAnimation *pAnimation = m_pScene->mAnimations[0]; // TODO

    glm::mat4 nodeTransformation(aiMatrix4x4ToGlm(&pNode->mTransformation));

    const aiNodeAnim *pNodeAnim = findNodeAnim(pAnimation, NodeName);

    if (pNodeAnim) {
        // Interpolate scaling and generate scaling transformation matrix
        aiVector3D Scaling;
        calcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
        glm::mat4 ScalingM = glm::scale(glm::vec3(Scaling.x, Scaling.y, Scaling.z));

        // Interpolate rotation and generate rotation transformation matrix
        aiQuaternion RotationQ;
        calcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
        const aiMatrix4x4t<float> tempMat(RotationQ.GetMatrix());
        glm::mat4 RotationM = aiMatrix4x4ToGlm(&tempMat);

        // Interpolate translation and generate translation transformation matrix
        aiVector3D Translation;
        calcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
        glm::mat4 TranslationM = glm::translate(glm::vec3(Translation.x, Translation.y, Translation.z));

        // Combine the above transformations
        nodeTransformation = TranslationM * RotationM * ScalingM;
    }

    glm::mat4 GlobalTransformation = ParentTransform * nodeTransformation;

    if (m_boneMapping.find(NodeName) != m_boneMapping.end()) {
        unsigned int boneIndex = m_boneMapping[NodeName];
        m_boneInfo[boneIndex].finalTransformation = m_globalInverseTransform * GlobalTransformation
                                                                * m_boneInfo[boneIndex].boneOffset;
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}

void BonedMesh::boneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
{
    Transforms.resize(numBones); // 5 is max meshes
    glm::mat4 Identity = glm::mat4(1.0); // 1.0 is redundant but was added for understanding

    float TicksPerSecond = m_pScene->mAnimations[0]->mTicksPerSecond != 0 ?
                               m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, m_pScene->mAnimations[0]->mDuration);

    ReadNodeHierarchy(AnimationTime, m_pScene->mRootNode, Identity);

    for (unsigned int i = 0; i < numBones; ++i) {
        Transforms[i] = m_boneInfo[i].finalTransformation;
    }
}

void BonedMesh::VertexBoneData::addBoneData(GLuint boneID, GLfloat weight)
{
    for (unsigned int i = 0 ; i < sizeof(ids) / sizeof(*ids); ++i) {
        if (weights[i] == 0.0) {
            ids[i]     = boneID;
            weights[i] = weight;
            return;
        }
    }

    std::cout << "warning: more bones than supported" << std::endl;
}

bool BonedMesh::initFromScene(const aiScene* pScene) {
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<VertexBoneData> boneVertices;
    std::vector<GLuint> indices;

    unsigned int numVertices = 0,
                 numIndices = 0;

    for (unsigned int i = 0; i < m_Entries.size(); ++i) {
        MeshEntry entry;
        entry.materialIndex = pScene->mMeshes[i]->mMaterialIndex;
        entry.numIndices = pScene->mMeshes[i]->mNumFaces * 3;
        entry.baseVertex = numVertices;
        entry.baseIndex = numIndices;
        
        numVertices += pScene->mMeshes[i]->mNumVertices;
        numIndices += entry.numIndices;

        m_Entries[i] = std::move(entry);
    }

    positions.reserve(numVertices);
    normals.reserve(numVertices);
    texCoords.reserve(numVertices);
    normals.reserve(numVertices);
    boneVertices.resize(numVertices); // bones vertex space isn't reserved!
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

        for (unsigned int j = 0; j < meshy->mNumBones; ++j) {
            const aiBone* bone = meshy->mBones[j];
            GLuint boneIndex = 0;
            std::string boneName(bone->mName.data);

            if (m_boneMapping.find(boneName) == m_boneMapping.end()) {
                boneIndex = numBones;
                m_boneInfo.push_back(BoneInfo());
                m_boneMapping[boneName] = boneIndex;
                m_boneInfo[boneIndex].boneOffset = aiMatrix4x4ToGlm(&bone->mOffsetMatrix);
                numBones++;
            } else {
                boneIndex = m_boneMapping[boneName];
            }

            for (unsigned int k = 0; k < bone->mNumWeights; ++k) {
                GLuint vId = m_Entries[i].baseVertex + bone->mWeights[k].mVertexId;
                float weight = bone->mWeights[k].mWeight;
                boneVertices[vId].addBoneData(boneIndex, weight);
            }
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

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boneVertices[0]) * boneVertices.size(), &boneVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(VertexBoneData), (const GLvoid*)0);

    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(VertexBoneData), (const GLvoid*)16);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)32);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)48);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW); 

    return glGetError() == GL_NO_ERROR;
}

void BonedMesh::draw() {
    glBindVertexArray(m_VAO);

    for (const auto &mesh : m_Entries) {
        m_Textures[mesh.materialIndex].bind(GL_TEXTURE0);
        glDrawElementsBaseVertex(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * mesh.baseIndex), mesh.baseVertex);
    }

    glBindVertexArray(0);
}

const aiNodeAnim* BonedMesh::findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
    for (unsigned int i = 0 ; i < pAnimation->mNumChannels ; ++i) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}
