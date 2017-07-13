#include "MeshContainer.h"

MeshContainer::MeshContainer() {
    // don't do anything
}

bool MeshContainer::has(std::string id) {
    return meshes.find(id) != meshes.end();
}

Mesh* MeshContainer::get(std::string id) {
    return meshes.at(id);
}

StaticMesh& MeshContainer::getStatic(std::string id) {
    return staticMeshes.at(id);
}

BonedMesh& MeshContainer::getBoned(std::string id) {
    return bonedMeshes.at(id);
}

BonedMesh& MeshContainer::put(std::string id, BonedMesh mesh) {
    bonedMeshes[id] = mesh;
    meshes[id] = &bonedMeshes[id];
    return bonedMeshes.at(id);
}

StaticMesh& MeshContainer::put(std::string id, StaticMesh mesh) {
    staticMeshes[id] = mesh;
    meshes[id] = &staticMeshes[id];
    return staticMeshes.at(id);
}
