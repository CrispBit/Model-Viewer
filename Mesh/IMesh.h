//
// Created by cilan on 7/12/2017
//

#ifndef IMESH_H
#define IMESH_H

#include <assimp/scene.h>
#include <boost/filesystem.hpp>
#include <string>

class Mesh
{
public:
    virtual bool loadMesh(const boost::filesystem::path& path) = 0;
    virtual bool initFromScene(const aiScene* pScene) = 0;
    virtual void draw() = 0;
};

#endif
