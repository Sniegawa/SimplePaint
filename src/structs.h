#pragma once

#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>


typedef enum Tool
{
	None = 0,
	Pencil,
	Brush,
	Eraser,
	ColorPicker
}Tool;

typedef struct Image
{
	unsigned int GlTextureID;
	struct nk_image handle;
	unsigned int Width;
	unsigned int Height;
	unsigned char* Data;
}Image;

typedef struct Color
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
}Color;

typedef struct ColorPalette
{
	Color foreground;
	Color background;
	Color* colorsArray;
}ColorPalette;

typedef struct APP_STATE 
{
	struct nk_context* ctx;
	GLFWwindow* window;

	Image* CurrentImage;
	bool ShouldCreateFile;
	const char* CurrentPath;
  bool NewFileFlag;

	Tool SelectedTool;
	int BrushSize;
	int LastMouseX, LastMouseY;

	ColorPalette Palette;
}APP_STATE;
