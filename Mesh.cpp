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
                           const std::vector<GLuint>& Indices,
                           const std::vector<VertexBoneData>& bones) {
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

    m_pScene = m_importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (m_pScene) {
        m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform.Inverse();
        ret = initFromScene(m_pScene);
    } else {
        std::cout << m_importer.GetErrorString() << std::endl;
    }

    return ret;
}

bool Mesh::initMaterials(const aiScene* pScene) {
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
        aiString texturePath;
        pScene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL);
        if (std::string(texturePath.C_Str()).empty()) {
            texturePath = aiString("white.png");
        }

        std::unique_ptr<Texture> texture = std::make_unique<Texture>(GL_TEXTURE_2D, std::string("assets/") + texturePath.C_Str());
        texture->load();

        m_Textures.push_back(std::move(texture));
    }
    return false;
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

unsigned int Mesh::findRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

void Mesh::calcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

unsigned int Mesh::findScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

void Mesh::calcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

unsigned int Mesh::findPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
    for (unsigned int i = 0 ; i < pNodeAnim->mNumPositionKeys - 1 ; i++) {
        if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
            return i;
        }
    }

    assert(0);
    return 0;
}

void Mesh::calcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

void Mesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
    std::string NodeName(pNode->mName.data);

    const aiAnimation* pAnimation = m_pScene->mAnimations[0]; // TODO

    glm::mat4 nodeTransformation(aiMatrix4x4ToGlm(&pNode->mTransformation));

    const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, NodeName);

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
        m_boneInfo[boneIndex].finalTransformation = m_globalInverseTransform * GlobalTransformation *
                                                    m_boneInfo[boneIndex].boneOffset;
    }

    for (unsigned int i = 0 ; i < pNode->mNumChildren ; i++) {
        ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}

glm::mat4 Mesh::boneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms)
{
    glm::mat4 Identity = glm::mat4(1.0); // 1.0 is redundant but was added for understanding

    float TicksPerSecond = m_pScene->mAnimations[0]->mTicksPerSecond != 0 ?
                           m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f;
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, m_pScene->mAnimations[0]->mDuration);

    ReadNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

    Transforms.resize(m_numBones);

    for (unsigned int i = 0 ; i < m_numBones ; i++) {
        Transforms[i] = m_boneInfo[i].finalTransformation;
    }
}

void Mesh::VertexBoneData::addBoneData(unsigned int boneID, float weight)
{
    for (unsigned int i = 0 ; i < sizeof(ids) / sizeof(*ids); ++i) {
        if (weights[i] == 0.0) {
            ids[i]     = boneID;
            weights[i] = weight;
            std::cout << i << std::endl;
            return;
        }
    }

    // ignore rest of bones
}

bool Mesh::initFromScene(const aiScene* pScene) {
    m_Entries.resize(pScene->mNumMeshes);

    unsigned int numVertices = 0;

    for (unsigned int i = 0; i < m_Entries.size(); i++) {
        std::unique_ptr<MeshEntry> entry = std::make_unique<MeshEntry>();
        numVertices += pScene->mMeshes[i]->mNumVertices;
        entry->materialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Entries[i] = std::move(entry);
    }

    for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
        aiMesh* meshy = pScene->mMeshes[i];
        std::vector<Vertex> vaortishes;
        std::vector<GLuint> indexs;
        std::vector<VertexBoneData> bones;
        bones.resize(meshy->mNumVertices);
        unsigned int vertices = meshy->mNumVertices;
        for (unsigned int j = 0; j < vertices; ++j) {
            const aiVector3D &potato = meshy->mVertices[j];
            const aiVector3D &n = meshy->mNormals[j];
            const aiVector3D &tc = meshy->HasTextureCoords(0) ? meshy->mTextureCoords[0][j] : aiVector3D(0,0,0);
            vaortishes.push_back(Vertex(glm::vec3(potato.x, potato.y, potato.z),
                                        glm::vec2(tc.x, tc.y),
                                        glm::vec3(n.x, n.y, n.z)
            ));
        }

        for (unsigned int j = 0; j < meshy->mNumFaces; ++j) {
            const aiFace& fase = meshy->mFaces[j];
            assert(fase.mNumIndices == 3);
            indexs.push_back(fase.mIndices[0]);
            indexs.push_back(fase.mIndices[1]);
            indexs.push_back(fase.mIndices[2]);
        }

        for (unsigned int j = 0; j < meshy->mNumBones; j++) {
            const aiBone* bone = meshy->mBones[j];
            unsigned int boneIndex = 0;
            std::string boneName(bone->mName.data);

            if (m_boneMapping.find(boneName) == m_boneMapping.end()) {
                boneIndex = m_numBones;
                m_numBones++;
                m_boneInfo.push_back(BoneInfo());
            } else {
                boneIndex = m_boneMapping[boneName];
            }

            m_boneMapping[boneName] = boneIndex;
            m_boneInfo[boneIndex].boneOffset = aiMatrix4x4ToGlm(&bone->mOffsetMatrix);

            for (unsigned int k = 0; k < bone->mNumWeights; k++) {
                unsigned int vId = bone->mWeights[k].mVertexId;
                float weight = bone->mWeights[k].mWeight;
                bones[vId].addBoneData(boneIndex, weight);
            }
        }

        m_Entries[i]->Init(vaortishes, indexs, bones);
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
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    for (const auto &mesh : m_Entries) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VB);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);                 // position
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); // texture coordinate
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20); // vector normals
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0); // bones
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16); // bone weights

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IB);

        m_Textures[mesh->materialIndex]->bind(GL_TEXTURE0);
        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);
    }

    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    // check OpenGL error
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}

const aiNodeAnim* Mesh::findNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
    for (unsigned int i = 0 ; i < pAnimation->mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}
