#include "ShaderManager.hpp"

ShaderManager::ShaderManager(VkDevice logicalDevice)
    : m_logicalDevice(logicalDevice)
{
    m_compileOptions.SetTargetEnvironment(
        shaderc_target_env_vulkan,
        shaderc_env_version_vulkan_1_2);
    m_compileOptions.SetOptimizationLevel(
        shaderc_optimization_level_performance);
}

VkShaderModule ShaderManager::loadFromGLSL(
    const std::string& filename,
    VkShaderStageFlagBits stage,
    const std::string& debugName)
{
    const std::string source = readTextFile(filename);

    shaderc_shader_kind kind;
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:   kind = shaderc_vertex_shader; break;
    case VK_SHADER_STAGE_FRAGMENT_BIT: kind = shaderc_fragment_shader; break;
    case VK_SHADER_STAGE_COMPUTE_BIT:  kind = shaderc_compute_shader; break;
    default:
        throw std::runtime_error("Unsupported shader stage");
    }

    auto spirv = compileGLSL(source, kind, debugName);

    return createShaderModule(spirv, debugName);
}

VkShaderModule ShaderManager::loadFromSPV(
    const std::string& filename,
    const std::string& debugName)
{
    auto spirv = readBinaryFile(filename);

    validateSPIRV(spirv);

    return createShaderModule(spirv, debugName);
}

//--- 私有方法实现 ---//
std::string ShaderManager::readTextFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open GLSL file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<uint32_t> ShaderManager::readBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open SPIR-V file: " + filename);
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize % sizeof(uint32_t) != 0) {
        throw std::runtime_error("SPIR-V file size is not 4-byte aligned: " + filename);
    }

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    if (!file) {
        throw std::runtime_error("Failed to read entire SPIR-V file: " + filename);
    }

    return buffer;
}

std::vector<uint32_t> ShaderManager::compileGLSL(
    const std::string& source,
    shaderc_shader_kind kind,
    const std::string& debugName)
{
    shaderc::SpvCompilationResult result = m_compiler.CompileGlslToSpv(
        source, kind, debugName.c_str(), m_compileOptions);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::string errorMsg = "Shader compilation failed (" + debugName + "):\n";
        errorMsg += result.GetErrorMessage();
        throw std::runtime_error(errorMsg);
    }

    return { result.cbegin(), result.cend() };
}

VkShaderModule ShaderManager::createShaderModule(
    const std::vector<uint32_t>& code,
    const std::string& debugName)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module: " + debugName);
    }

    return shaderModule;
}

void ShaderManager::validateSPIRV(const std::vector<uint32_t>& code) {
    if (code.empty()) {
        throw std::runtime_error("Empty SPIR-V code");
    }

    constexpr uint32_t SPIRV_MAGIC = 0x07230203;
    if (code[0] != SPIRV_MAGIC) {
        throw std::runtime_error("Invalid SPIR-V magic number");
    }
}