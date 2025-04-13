cmake_minimum_required(VERSION 3.20)
project(VulkanProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 第三方库配置 --------------------------------------------------
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdParty)
set(THIRD_PARTY_INCLUDE_DIR ${THIRD_PARTY_DIR}/include)
set(THIRD_PARTY_LIB_DIR ${THIRD_PARTY_DIR}/lib)

list(APPEND THIRD_PARTY_INCLUDES
    ${THIRD_PARTY_INCLUDE_DIR}
    ${THIRD_PARTY_INCLUDE_DIR}/assimp
    ${THIRD_PARTY_INCLUDE_DIR}/GLFW
    ${THIRD_PARTY_INCLUDE_DIR}/glm
    ${THIRD_PARTY_INCLUDE_DIR}/shaderc
    ${THIRD_PARTY_INCLUDE_DIR}/vulkan
)

# 头文件包含
include_directories(${THIRD_PARTY_INCLUDES})

# 源文件
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/renderer/*.cpp"
    "${CMAKE_SOURCE_DIR}/renderer/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

#=============================shader==================================
# 使用CONFIGURE_DEPENDS自动跟踪新增/删除的Shader文件（需要CMake 3.12+）
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS 
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.frag"
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.comp"
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.glsl"
    )
else()
    file(GLOB_RECURSE SHADER_FILES
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.frag"
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.comp"
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*.glsl"
    )
endif()

set(SHADER_TARGETS)  # 存储所有生成的Shader标记文件

foreach(shader_file IN LISTS SHADER_FILES)
    # 计算相对路径（相对于renderer/resources/shaders）
    file(RELATIVE_PATH relative_path 
        "${CMAKE_SOURCE_DIR}/renderer/resources/shaders" 
        ${shader_file}
    )
    
    # 中间标记文件路径（构建目录中）
    get_filename_component(relative_dir "${relative_path}" DIRECTORY)
    set(flag_file "${CMAKE_CURRENT_BINARY_DIR}/shaders_copied/${relative_path}.flag")
    
    # 创建目标目录并复制文件，最后生成标记文件
    add_custom_command(
        OUTPUT ${flag_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders/${relative_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy ${shader_file} "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders/${relative_path}"
        COMMAND ${CMAKE_COMMAND} -E touch ${flag_file}  # 创建标记文件以指示完成
        DEPENDS ${shader_file}
        COMMENT "Updating shader: ${relative_path}"
    )
    
    list(APPEND SHADER_TARGETS ${flag_file})
endforeach()

# 创建全局Shader复制目标
add_custom_target(CopyShaders ALL
    DEPENDS ${SHADER_TARGETS}
    COMMENT "Ensuring latest shader files"
)

# 将Shader复制目标绑定到主程序
add_dependencies(${PROJECT_NAME} CopyShaders)
#=============================shader==================================

# 平台判断逻辑
if(MSVC)
    set(GLFW_LIB "${THIRD_PARTY_LIB_DIR}/glfw/glfw3_mt.lib")
    set(VULKAN_LIB "${THIRD_PARTY_LIB_DIR}/vulkan/loader/vulkan-1.lib")
    set(ASSIMP_LIB "${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.lib")
    set(SHADERC_LIB "${THIRD_PARTY_LIB_DIR}/vulkan/shaderc/shaderc_shared.lib")
    
    set(RUNTIME_DLLS
        "${THIRD_PARTY_LIB_DIR}/glfw/glfw3.dll"
        "${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.dll"
        "${THIRD_PARTY_LIB_DIR}/vulkan/shaderc/shaderc_shared.dll"
        "${THIRD_PARTY_LIB_DIR}/vulkan/loader/vulkan-1.dll"
    )

elseif(MINGW)
    set(GLFW_LIB "${THIRD_PARTY_LIB_DIR}/glfw/libglfw3.a")
    set(VULKAN_LIB "${THIRD_PARTY_LIB_DIR}/vulkan/loader/vulkan-1.lib")
    set(ASSIMP_LIB "${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.lib")
    set(SHADERC_LIB "${THIRD_PARTY_LIB_DIR}/vulkan/shaderc/shaderc_shared.lib")
    
    set(RUNTIME_DLLS
        "${THIRD_PARTY_LIB_DIR}/glfw/libglfw3.dll"
        "${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.dll"
        "${THIRD_PARTY_LIB_DIR}/vulkan/shaderc/shaderc_shared.dll"
        "${THIRD_PARTY_LIB_DIR}/vulkan/loader/vulkan-1.dll"
    )

elseif(UNIX AND NOT APPLE)
    set(GLFW_LIB "${THIRD_PARTY_LIB_DIR}/glfw/libglfw.so")
    set(VULKAN_LIB "${THIRD_PARTY_LIB_DIR}/vulkan/loader/libvulkan.so")
    set(ASSIMP_LIB "${THIRD_PARTY_LIB_DIR}/assimp/libassimp.so")
    set(SHADERC_LIB "${THIRD_PARTY_LIB_DIR}/vulkan/shaderc/libshaderc_shared.so")
    
    set(RUNTIME_LIBS
        ${GLFW_LIB}
        "${THIRD_PARTY_LIB_DIR}/glfw/libglfw.so.3"
        ${VULKAN_LIB}
        ${ASSIMP_LIB}
        ${SHADERC_LIB}
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW_LIB}
    ${VULKAN_LIB}
    ${ASSIMP_LIB}
    ${SHADERC_LIB}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy
        ${SHADER_FILES}
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources/shaders"
    COMMENT "Copying shader files"
)

foreach(DLL_FILE IN LISTS RUNTIME_DLLS)
    if(EXISTS "${DLL_FILE}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "${DLL_FILE}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ${DLL_FILE}"
        )
    endif()
endforeach()

foreach(SO_FILE IN LISTS RUNTIME_LIBS)
    if(EXISTS "${SO_FILE}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "${SO_FILE}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ${SO_FILE}"
        )
    endif()
endforeach()

# Vulkan验证层配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        ENABLE_VALIDATION_LAYERS
    )

    # 验证层文件处理
    set(VK_LAYER_SRC "${THIRD_PARTY_LIB_DIR}/vulkan/layers")
    set(VK_LAYER_DEST "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${VK_LAYER_DEST}/layers"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${VK_LAYER_SRC}/VkLayer_khronos_validation.dll"
            "${VK_LAYER_DEST}/"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${VK_LAYER_SRC}/VkLayer_khronos_validation.json"
            "${VK_LAYER_DEST}/layers"
        COMMENT "Copying Vulkan validation layers"
    )

    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_ENVIRONMENT 
            "VK_LAYER_PATH=${VK_LAYER_DEST};%VK_LAYER_PATH%"
        )
    endif()
endif()