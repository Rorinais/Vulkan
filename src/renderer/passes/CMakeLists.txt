file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(passes STATIC ${SRC})

target_link_libraries(passes PRIVATE
	BaseInterface
	core
	pipeline
)


