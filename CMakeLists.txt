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

file(GLOB SHADER_FILES
    "${CMAKE_SOURCE_DIR}/renderer/resources/shaders/*"
)

add_executable(${PROJECT_NAME} ${SOURCES})

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

    # 设置环境变量（Windows）
    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            VS_DEBUGGER_ENVIRONMENT 
            "VK_LAYER_PATH=${VK_LAYER_DEST};%VK_LAYER_PATH%"
        )
    endif()
endif()