cmake_minimum_required(VERSION 3.10)
project(VulkanProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty)
set(THIRD_PARTY_INCLUDE_DIR ${THIRD_PARTY_DIR}/include)
set(THIRD_PARTY_LIB_DIR ${THIRD_PARTY_DIR}/lib) 

# 添加头文件搜索路径
include_directories(
    ${THIRD_PARTY_INCLUDE_DIR}
    ${THIRD_PARTY_INCLUDE_DIR}/vulkan
    ${THIRD_PARTY_INCLUDE_DIR}/GLFW
    ${THIRD_PARTY_INCLUDE_DIR}/glm
    ${THIRD_PARTY_INCLUDE_DIR}/assimp
)

# 添加库文件搜索路径（关键！）
link_directories(
    ${THIRD_PARTY_LIB_DIR}/vulkan
    ${THIRD_PARTY_LIB_DIR}/glfw
    ${THIRD_PARTY_LIB_DIR}/assimp
)

add_executable(${PROJECT_NAME} main.cpp)


if(WIN32)
    if (MSVC)
        set(GLFW_LIBRARY ${THIRD_PARTY_LIB_DIR}/glfw/glfw3.lib)
        set(VULKAN_LIBRARY ${THIRD_PARTY_LIB_DIR}/vulkan/vulkan-1.lib)
        set(ASSIMP_LIBRARY ${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.lib)
    elseif(MINGW)
        set(GLFW_LIBRARY ${THIRD_PARTY_LIB_DIR}/glfw/libglfw3.a)
        set(VULKAN_LIBRARY ${THIRD_PARTY_LIB_DIR}/vulkan/vulkan-1.lib)
        set(ASSIMP_LIBRARY ${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.lib)
    endif()

    if(NOT EXISTS "${THIRD_PARTY_LIB_DIR}/vulkan/VkLayer_khronos_validation.dll" OR 
       NOT EXISTS "${THIRD_PARTY_LIB_DIR}/vulkan/VkLayer_khronos_validation.json")
        message(FATAL_ERROR "Vulkan validation layer files missing in thirdParty/lib/vulkan!")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${THIRD_PARTY_LIB_DIR}/vulkan/vulkan-1.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy
            "${THIRD_PARTY_LIB_DIR}/assimp/assimp-vc143-mt.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy
            "${THIRD_PARTY_LIB_DIR}/vulkan/VkLayer_khronos_validation.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy
            "${THIRD_PARTY_LIB_DIR}/vulkan/VkLayer_khronos_validation.json"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

elseif(UNIX AND NOT APPLE)
    # Linux配置：使用绝对路径指定库文件
    set(GLFW_LIBRARY ${THIRD_PARTY_LIB_DIR}/glfw/libglfw.so)
    set(VULKAN_LIBRARY ${THIRD_PARTY_LIB_DIR}/vulkan/libvulkan.so)

    #set(GLFW_LIBRARY glfw)
    #set(VULKAN_LIBRARY vulkan)
    #set(ASSIMP_LIBRARY assimp)
    # 设置验证层路径环境变量
    #set(VK_LAYER_PATH "${THIRD_PARTY_LIB_DIR}/vulkan:$ENV{VK_LAYER_PATH}")
    #set_target_properties(${PROJECT_NAME} PROPERTIES
        #ENVIRONMENT "VK_LAYER_PATH=${VK_LAYER_PATH}"
    #)
    #复制验证层文件
    #add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        #COMMAND ${CMAKE_COMMAND} -E copy
            #"${THIRD_PARTY_LIB_DIR}/vulkan/libVkLayer_khronos_validation.so"
            #$<TARGET_FILE_DIR:${PROJECT_NAME}>
        #COMMAND ${CMAKE_COMMAND} -E copy
            #"${THIRD_PARTY_LIB_DIR}/vulkan/VkLayer_khronos_validation.json"
            #$<TARGET_FILE_DIR:${PROJECT_NAME}>
    #)

endif()

    target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${GLFW_LIBRARY}
    ${ASSIMP_LIBRARY}
    ${VULKAN_LIBRARY}
    )

# 特殊平台处理（macOS部分保持不变）
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${THIRD_PARTY_LIB_DIR}/vulkan/libMoltenVK.dylib
    )
endif()

foreach(header_file
    stb_image.h
    vulkan/vulkan.h
    GLFW/glfw3.h
    assimp/config.h
)
    if(NOT EXISTS ${THIRD_PARTY_INCLUDE_DIR}/${header_file})
        message(FATAL_ERROR "Required header ${header_file} not found in thirdParty/include!")
    endif()
endforeach()