#pragma once
#include "../../base.hpp"
#include "../../resources/textures/Texture.hpp"
#include "../shaders/ShaderProgram.hpp"

class Material {
public:
    using Ptr = std::shared_ptr<Material>;

    void setShaderProgram(ShaderProgram::Ptr program) { shaderProgram = program; }
    void addTexture(Texture::Ptr texture) { textures.push_back(texture); }

    void setPipelineState(const PipelineState& state) { pipelineState = state; }

    void setBaseColor(const glm::vec4& color) { baseColor = color; }
    void setMetallic(float value) { metallic = value; }
    void setRoughness(float value) { roughness = value; }

private:
    std::string name = "DefaultMaterial";

    std::vector<Texture::Ptr> textures;
    ShaderProgram::Ptr shaderProgram;

    PipelineState pipelineState;



    // PBR 材质参数
    glm::vec4 baseColor = glm::vec4(1.0f);
    float metallic = 0.0f;
    float roughness = 1.0f;
    // 可扩展：emissive, ao 等
};