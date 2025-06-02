add_library(BaseInterface INTERFACE)
target_include_directories(BaseInterface INTERFACE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(BaseInterface INTERFACE
    assimp
    glfw
    glm
    imgui
    spdlog
    vulkan 
    shaderc
)

add_subdirectory(core/application)
add_subdirectory(core/platform)
add_subdirectory(core/utils)
add_subdirectory(renderer/core)
add_subdirectory(renderer/passes)
add_subdirectory(renderer/pipeline)
add_subdirectory(renderer/resources)

add_executable(${PROJECT_NAME} ${MAIN_SOURCE})

target_link_libraries(${PROJECT_NAME} 
PRIVATE 
    BaseInterface
    window
    application
    resources
)

function(copy_target_resources TARGET_NAME)
    cmake_parse_arguments(PARSE_ARGV 1 "ARG" "" "SHADERS;FONTS;MODELS;TEXTURES" "")
    
    get_target_property(TARGET_OUTPUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    set(ASSETS_DEST "${TARGET_OUTPUT_DIR}/assets")

    if(ARG_SHADERS)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
            file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS 
                "${ARG_SHADERS}/*.vert"
                "${ARG_SHADERS}/*.frag"
                "${ARG_SHADERS}/*.comp"
                "${ARG_SHADERS}/*.glsl"
            )
        else()
            file(GLOB_RECURSE SHADER_FILES
                "${ARG_SHADERS}/*.vert"
                "${ARG_SHADERS}/*.frag"
                "${ARG_SHADERS}/*.comp"
                "${ARG_SHADERS}/*.glsl"
            )
        endif()

        set(SHADER_TARGETS)

        foreach(shader_file IN LISTS SHADER_FILES)
            file(RELATIVE_PATH relative_path "${ARG_SHADERS}" "${shader_file}")
            set(final_dest "${ASSETS_DEST}/shaders/${relative_path}")
            get_filename_component(final_dir "${final_dest}" DIRECTORY)
            
            set(flag_file "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_shaders/${relative_path}.flag")

            add_custom_command(
                OUTPUT ${flag_file}
                COMMAND ${CMAKE_COMMAND} -E make_directory "${final_dir}"
                COMMAND ${CMAKE_COMMAND} -E copy "${shader_file}" "${final_dest}"
                COMMAND ${CMAKE_COMMAND} -E touch ${flag_file}
                DEPENDS "${shader_file}"
                COMMENT "强制更新 Shader: ${relative_path}"
            )
            list(APPEND SHADER_TARGETS ${flag_file})
        endforeach()

        add_custom_target(${TARGET_NAME}_CopyShaders ALL
            DEPENDS ${SHADER_TARGETS}
            COMMENT "确保${TARGET_NAME}的Shader文件最新"
        )

        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_CopyShaders)
    endif()

    foreach(res_type FONTS MODELS TEXTURES)
        string(TOLOWER "${res_type}" res_type_lower)
        if(ARG_${res_type})
            file(GLOB_RECURSE RES_FILES CONFIGURE_DEPENDS "${ARG_${res_type}}/*")
            foreach(res_file IN LISTS RES_FILES)
                file(RELATIVE_PATH relative_path "${ARG_${res_type}}" "${res_file}")
                set(final_dest "${ASSETS_DEST}/${res_type_lower}/${relative_path}")
                get_filename_component(final_dir "${final_dest}" DIRECTORY)
                add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${final_dir}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${res_file}"
                        "${final_dest}"
                    COMMENT "差异更新 ${res_type}: ${relative_path}"
                )
            endforeach()
        endif()
    endforeach()
endfunction()

copy_target_resources(${PROJECT_NAME}
    SHADERS    ${SHADERS_DIR}    
    FONTS      ${FONTS_DIR}      
    MODELS     ${MODELS_DIR}     
    TEXTURES   ${TEXTURES_DIR}   
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/external/glfw/glfw3.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying vulkan-1.dll to output directory"
        VERBATIM
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/external/vulkan/lib/vulkan-1.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying vulkan-1.dll to output directory"
        VERBATIM
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/external/vulkan/lib/shaderc/shaderc_sharedd.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying vulkan-1.dll to output directory"
        VERBATIM
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/external/assimp/assimp/lib/assimp-vc143-mt.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying vulkan-1.dll to output directory"
        VERBATIM
    )
endif()

# Vulkan验证层配置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        ENABLE_VALIDATION_LAYERS
    )

    # 验证层文件处理
    set(VK_LAYER_SRC "${CMAKE_SOURCE_DIR}/external/vulkan/lib/layers")
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

