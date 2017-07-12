//
// Created by cilan on 7/12/2017
//

#ifndef IMESH_H
#define IMESH_H

#include <string>
#include <assimp/scene.h>

class Mesh
{
public:
    virtual bool loadMesh(const std::string& path) = 0;
    virtual bool initFromScene(const aiScene* pScene) = 0;
    virtual void draw() = 0;
};

#endif
