#include "MeshContainer.h"

MeshContainer::MeshContainer() {
    // don't do anything
}

bool MeshContainer::has(std::string id) {
    return meshes.find(id) != meshes.end();
}

Mesh& MeshContainer::get(std::string id) {
    return *meshes.at(id);
}

StaticMesh& MeshContainer::getStatic(std::string id) {
    return staticMeshes.at(id);
}

BonedMesh& MeshContainer::getBoned(std::string id) {
    return bonedMeshes.at(id);
}

BonedMesh& MeshContainer::put(std::string id, BonedMesh mesh) {
    bonedMeshes.emplace(id, mesh);
    meshes.emplace(id, &bonedMeshes.at(id));
    return bonedMeshes.at(id);
}

StaticMesh& MeshContainer::put(std::string id, StaticMesh mesh) {
    staticMeshes.emplace(id, mesh);
    meshes.emplace(id, &staticMeshes.at(id));
    return staticMeshes.at(id);
}
