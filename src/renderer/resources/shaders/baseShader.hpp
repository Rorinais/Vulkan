#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "../../core/context/logicalDevice.hpp"
#include "../../../core/utils/shaderUtils.hpp"

class BaseShader {
public:
    enum class ShaderType {
        Vertex,
        Fragment,
        Geometry,
        Compute,
        Default
    };

    BaseShader(ShaderType type= BaseShader::ShaderType::Default, const std::string& version = "#version 450");
    ~BaseShader() {
        clearShaderCache();
    }

    void addInput(const std::string& type, const std::string& name, int location = -1);

    void addOutput(const std::string& type, const std::string& name, int location = -1);

    void addUniformBuffer(const std::string& name, int binding, const std::vector<std::string>& members);

    void addSampler(const std::string& type, const std::string& name, int binding);

    void addUniformBuffer(
        const std::string& name,
        int set,
        int binding,
        const std::vector<std::string>& members,
        const std::string& layoutQualifiers = ""
    );

    void addSampler(
        const std::string& type,
        const std::string& name,
        int set,
        int binding,
        const std::string& layoutQualifiers = ""
    );

    void setMainBody(const std::string& body);

    void addCustomCode(const std::string& code);

    void addStruct(const std::string& name, const std::vector<std::string>& members);

    void addFunction(const std::string& signature, const std::string& body);

    std::string getSource() const;

    ShaderType getType() const { return m_type; }

    ShaderProgram::Ptr buildDefaultShaderStages(const LogicalDevice::Ptr& logicalDevice) {

        if (mShaderCache.find("DefaultShader") != mShaderCache.end()) {
            return mShaderCache["DefaultShader"];
        }

        ShaderProgram::Ptr shaderStages = ShaderProgram::create(logicalDevice);

        BaseShader vertBuilder(BaseShader::ShaderType::Vertex,
            "#version 450\n#extension GL_KHR_vulkan_glsl : enable");

        //vertBuilder.addInput("vec3", "inPosition", 0);
        //vertBuilder.addInput("vec3", "inColor", 1);
        //vertBuilder.addOutput("vec3", "fragColor", 0);
        //vertBuilder.addUniformBuffer("UniformBufferObject", 0, 0, {
        //    "mat4 model",
        //    "mat4 view",
        //    "mat4 proj"
        //    });

        //vertBuilder.addUniformBuffer("UniformBufferObject", 0, 0, {
        //    "mat4 mvp",
        //    });

        //vertBuilder.setMainBody(R"(
        //    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
        //    //gl_Position = ubo.mvp*vec4(inPosition, 1.0);
        //    fragColor = inColor;
        //)");

        vertBuilder.addInput("vec3", "inPosition", 0);
        vertBuilder.addInput("vec3", "inColor", 1);
        vertBuilder.addOutput("vec3", "fragColor", 0);
        vertBuilder.addOutput("float", "fragDistance", 1);
        vertBuilder.addUniformBuffer("UniformBufferObject", 0, 0, {
            "mat4 model",
            "mat4 view",
            "mat4 proj"
            });

        vertBuilder.setMainBody(R"(
            vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
            vec4 viewPos = ubo.view * worldPos;
            gl_Position = ubo.proj * viewPos;
    
            fragColor = inColor;
            fragDistance = length(viewPos.xyz);
        )");

        BaseShader fragBuilder(BaseShader::ShaderType::Fragment,
            "#version 450\n#extension GL_KHR_vulkan_glsl : enable");

        //fragBuilder.addInput("vec3", "fragColor", 0);
        //fragBuilder.addOutput("vec4", "outColor", 0);
        //fragBuilder.addSampler("sampler2D", "texSampler", 1, 0);
        //fragBuilder.setMainBody(R"(
        //    //outColor = texture(texSampler, fragTexCoord);
        //    outColor = vec4(fragColor,1.0);
        //)");

        fragBuilder.addInput("vec3", "fragColor", 0);
        fragBuilder.addInput("float", "fragDistance", 1);
        fragBuilder.addOutput("vec4", "outColor", 0);
        fragBuilder.setMainBody(R"(
            float fade = 1.0 - smoothstep(0.0, 50.0, fragDistance);

            float axisFactor = 1.0;
            if (fragColor.r > 0.9 || fragColor.b > 0.9) {
                axisFactor = 2.0;
            }

            vec3 finalColor = fragColor * fade * axisFactor;

            float blueTint = smoothstep(0.0, 100.0, fragDistance) * 0.2;
            finalColor.b += blueTint;

            outColor = vec4(finalColor, 1.0);
        )");

        std::string vertexShader = vertBuilder.getSource();
        std::string fragmentShader = fragBuilder.getSource();

        std::cout << "=== Vertex Shader ===\n" << vertexShader << "\n\n";
        std::cout << "=== Fragment Shader ===\n" << fragmentShader << "\n";

        shaderStages->addGLSLStringStage(vertexShader, VK_SHADER_STAGE_VERTEX_BIT, "main", {}, "VertexShader");
        shaderStages->addGLSLStringStage(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT, "main", {}, "FragmentShader");

        mShaderCache["DefaultShader"] = shaderStages;
        return shaderStages;
    }

    void resetShaderCache(std::string shaderName) {
        mShaderCache.erase(shaderName);
    }

    void clearShaderCache() {
        for (auto& pair : mShaderCache) {
            if (pair.second) {
                pair.second.reset();
            }
        }
        mShaderCache.clear();
    }
private:
    ShaderType m_type;
    std::string m_version;
    std::ostringstream m_inputs;
    std::ostringstream m_outputs;
    std::ostringstream m_uniforms;
    std::ostringstream m_structs;
    std::ostringstream m_functions;
    std::ostringstream m_extensions;
    std::ostringstream m_customCode;
    std::string m_mainBody;

    std::map<std::string, ShaderProgram::Ptr> mShaderCache;
};