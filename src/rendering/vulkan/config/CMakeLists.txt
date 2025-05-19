file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(config STATIC ${SRC})

target_link_libraries(config PRIVATE
	BaseInterface
)

