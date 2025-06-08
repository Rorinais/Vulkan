#pragma once
#include <unordered_map>
#include "../../base.hpp"
#include "Material.hpp"

class MaterialWarehouse {
public:
    void addMaterial(const std::string& id, Material::Ptr mat) {
        materials[id] = std::move(mat);
    }

    Material::Ptr getMaterial(const std::string& id) {
        auto it = materials.find(id);
        return it != materials.end() ? it->second : nullptr;
    }

private:
    std::unordered_map<std::string, Material::Ptr> materials;
};