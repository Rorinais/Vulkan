# 场景模块
file(GLOB SCENE_SOURCES CONFIGURE_DEPENDS
    "*.cpp"
)

add_library(RenderingScene STATIC ${SCENE_SOURCES})

target_include_directories(RenderingScene PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLM_INCLUDE_DIR}
)

target_link_libraries(RenderingScene PUBLIC
    RenderingCore
    RenderingVulkan
)