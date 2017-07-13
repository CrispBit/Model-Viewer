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
    MeshContainer();
    BonedMesh& put(std::string id, BonedMesh mesh);
    StaticMesh& put(std::string id, StaticMesh mesh);
    StaticMesh& getStatic(std::string id);
    BonedMesh& getBoned(std::string id);
    Mesh& get(std::string id);
    bool has(std::string id);
private:
    std::map<std::string, StaticMesh> staticMeshes;
    std::map<std::string, BonedMesh> bonedMeshes;
    std::map<std::string, Mesh*> meshes;
};

#endif
