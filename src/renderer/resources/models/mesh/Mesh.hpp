#pragma once 
#include "../../../base.hpp"
#include "../../buffers/IndexBuffer.hpp"
#include "../../buffers/VertexArrayBuffer.hpp"
#include "../boundingBox/BoundingBox.hpp"
#include "../geometry/Geometry.hpp"
#include "../materials/Material.hpp"

class Mesh {
public:
    Mesh(Geometry geometry, Material::Ptr material)
        : geometry(std::move(geometry)), material(material) {
    }

    // 变换操作
    void setTransform(const glm::mat4& transform);
    void setMaterial(const std::string& matID);

    // 渲染准备
    void uploadToGPU();
    void bind() const;

    // 数据访问
    const Geometry& getGeometry() const { return geometry; }
    Material::Ptr getMaterial() const { return material; }
    const BoundingBox& getBoundingBox() const { return boundingBox; }

private:
    std::string name = "DefaultMesh";
    std::string materialID = "0";
    VertexArrayBuffer::Ptr vertexBuffer;
    IndexBuffer::Ptr indexBuffer;

    Geometry geometry;
    std::unique_ptr<BoundingBox> boundingBox; 
    std::unique_ptr<BoundingBox> worldBoundingBox;  



    void calculateBoundingBox() {
        boundingBox->reset();
        for (const auto& vertex : geometry.vertices) {
            boundingBox->expand(vertex.position);
        }
        updateWorldBoundingBox();
    }

    void updateWorldBoundingBox() {
        if (!worldBoundingBox) {
            worldBoundingBox.reset(boundingBox->clone());
        }
        *worldBoundingBox = *boundingBox;  
        worldBoundingBox->transform(transform);  
    }
};