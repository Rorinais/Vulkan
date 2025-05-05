#pragma once
#include"../base.hpp"
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <sstream>

class ShaderManager {
public:
    explicit ShaderManager(VkDevice logicalDevice);

    // 从GLSL源码编译（内存中）
    VkShaderModule loadFromGLSL(
        const std::string& filename,
        VkShaderStageFlagBits stage,
        const std::string& debugName = "");

    // 从预编译SPIR-V加载
    VkShaderModule loadFromSPV(
        const std::string& filename,
        const std::string& debugName = "");

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

private:
    std::string readTextFile(const std::string& filename);
    std::vector<uint32_t> readBinaryFile(const std::string& filename);
    std::vector<uint32_t> compileGLSL(const std::string& source,
        shaderc_shader_kind kind,
        const std::string& debugName);
    VkShaderModule createShaderModule(const std::vector<uint32_t>& code,
        const std::string& debugName);

    static void validateSPIRV(const std::vector<uint32_t>& code);

private:
    VkDevice m_logicalDevice;
    shaderc::Compiler m_compiler;
    shaderc::CompileOptions m_compileOptions;
};