#include "MeshContainer.h"
#include <memory>

MeshContainer::MeshContainer(boost::filesystem::path assetsDir) : assetsDir(assetsDir) {
    // don't do anything
}

bool MeshContainer::has(std::string id) {
    return meshes.find(id) != meshes.end();
}

Mesh* MeshContainer::get(std::string id) {
    return meshes.at(id);
}

StaticMesh* MeshContainer::getStatic(std::string id) {
    return staticMeshes.at(id).get();
}

BonedMesh* MeshContainer::getBoned(std::string id) {
    return bonedMeshes.at(id).get();
}

BonedMesh* MeshContainer::put(std::string id, std::unique_ptr<BonedMesh> mesh) {
    bonedMeshes.emplace(id, std::move(mesh));
    meshes.emplace(id, bonedMeshes.at(id).get());
    return bonedMeshes.at(id).get();
}

BonedMesh* MeshContainer::create(std::string id, boost::filesystem::path relativePath) {
    std::unique_ptr<BonedMesh> tempObject = std::make_unique<BonedMesh>();
    tempObject->loadMesh(relativePath, assetsDir, textures);
    return put(id, std::move(tempObject));
}

StaticMesh* MeshContainer::put(std::string id, std::unique_ptr<StaticMesh> mesh) {
    staticMeshes.emplace(id, std::move(mesh));
    meshes.emplace(id, staticMeshes.at(id).get());
    return staticMeshes.at(id).get();
}
