cmake_minimum_required(VERSION 3.15)
project(Editor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 统一可执行文件和资源输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) 

# 资源路径配置
set(SHADERS_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
set(FONTS_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts)
set(MODELS_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/assets/models)
set(TEXTURES_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/assets/textures)

set(MAIN_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/launch/vkTest1.cpp)

# 添加子目录
add_subdirectory(external/vulkan)
add_subdirectory(external/glfw)
add_subdirectory(external/assimp)
add_subdirectory(external/glm)
add_subdirectory(external/imgui)
add_subdirectory(external/spdlog)
 
add_subdirectory(src) 

#DEBUG 
#cmake  -DCMAKE_BUILD_TYPE=Debug ..

#Relese
#cmake  -DCMAKE_BUILD_TYPE=Release ..



