add_subdirectory(config)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(VulkanDevice STATIC ${SRC})

target_link_libraries(VulkanDevice PRIVATE
	BaseInterface
	window
	config
	core
)


