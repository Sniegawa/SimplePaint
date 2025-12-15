cmake_minimum_required(VERSION 3.16)

project(SimplePaint)

add_executable(SimplePaint
	"main.c"
 "structs.h")

target_link_libraries(SimplePaint PUBLIC
	GLAD
	glfw
)

target_include_directories(SimplePaint PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}/
	${CMAKE_SOURCE_DIR}/libs/GLAD/include
	${CMAKE_SOURCE_DIR}/libs/glfw/include
	${CMAKE_SOURCE_DIR}/libs/Nuklear/
)