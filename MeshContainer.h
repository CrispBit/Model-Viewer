//
// Created by cilan on 7/12/2017
//

#ifndef MESHCONTAINER_H
#define MESHCONTAINER_H

#include "StaticMesh.h"
#include "BonedMesh.h"
#include <memory>

class MeshContainer
{
public:
    MeshContainer();
    std::unique_ptr<Mesh>& put(std::string id, BonedMesh mesh);
    std::unique_ptr<Mesh>& put(std::string id, StaticMesh mesh);
    std::unique_ptr<Mesh>& get(std::string id);
    std::unique_ptr<StaticMesh>& getStatic(std::string id);
    std::unique_ptr<BonedMesh>& getBoned(std::string id);
    bool has(std::string id);
private:
    std::map<std::string, std::unique_ptr<StaticMesh>> staticMeshes;
    std::map<std::string, std::unique_ptr<Mesh>&> meshes;
};

#endif
