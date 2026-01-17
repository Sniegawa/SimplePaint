cmake_minimum_required(VERSION 3.16)

project(SimplePaint)

if(WIN32)
	set(PLATFORM_SOURCES
		"Platform/OpenFileWindows.c"
	)
elseif(UNIX)
	set(PLATFORM_SOURCES
		"Platform/OpenFileLinux.c"
	)
endif()

add_executable(SimplePaint
	"main.c"
	"structs.h"
	"colors.h"
	"image.c" 
	"image.h" 
	"OpenFile.h" 
	${PLATFORM_SOURCES}
 "interface.h" "interface.c" "app.h" "app.c" "draw.h" "draw.c")

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