#pragma once 
#include "../../../base.hpp"
#include "mesh/Mesh.hpp"
#include "boundingBox/BoundingBox.hpp"

class Model {
public:
    // 模型操作
    void addMesh(Mesh mesh);
    void setGlobalTransform(const glm::mat4& transform);

    // 模型处理
    void updateBoundingBox();
    void uploadAllToGPU();

    // 渲染
    void draw() const;

    // 数据访问
    const std::vector<Mesh>& getMeshes() const { return meshes; }
    const BoundingBox& getBoundingBox() const { return boundingBox; }

private:
    std::vector<Mesh> meshes;

    std::string name = "DefaultModel";
    BoundingBox boundingBox;
    glm::mat4 globalTransform = glm::mat4(1.0f);
};