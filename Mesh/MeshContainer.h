//
// Created by cilan on 7/12/2017
//

#ifndef MESHCONTAINER_H
#define MESHCONTAINER_H

#include "StaticMesh.h"
#include "BonedMesh.h"

class MeshContainer
{
public:
    MeshContainer(boost::filesystem::path assetsDir);
    BonedMesh* put(std::string id, std::unique_ptr<BonedMesh> mesh);
    BonedMesh* create(std::string id, boost::filesystem::path relativePath);
    StaticMesh* put(std::string id, std::unique_ptr<StaticMesh> mesh);
    StaticMesh* getStatic(std::string id);
    BonedMesh* getBoned(std::string id);
    Mesh* get(std::string id);
    bool has(std::string id);
private:
    std::map<std::string, std::unique_ptr<StaticMesh>> staticMeshes;
    std::map<std::string, std::unique_ptr<BonedMesh>> bonedMeshes;
    std::map<std::string, Mesh*> meshes;

    std::map<std::string, Texture> textures;

    boost::filesystem::path assetsDir;
};

#endif
