#include "structs.h"
#include "colors.h"
#include "Image.h"
#include "OpenFile.h"

#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string.h>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>

#define ToolboxWidthRatio 0.25f

// GLFW callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* InitLibraries();
APP_STATE* InitApp();

// Menu drawing functions
void DrawToolbox(APP_STATE* state);
void DrawViewport(APP_STATE* state);
void DrawMenu(APP_STATE* state);

// Draw pencil checks if it should use DrawLine function or DrawPoint
void DrawPencil(APP_STATE* state, unsigned int x, unsigned int y, Color c, int BrushSize);
void DrawLine(APP_STATE* state, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, Color c, int BrushSize);
void DrawPoint(APP_STATE* state, unsigned int x, unsigned int y, Color c);

int main(int argc, char** argv)
{
	APP_STATE* state = InitApp();


	// Load fonts
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&atlas);
	// For loading fonts Nuklear/demo/glfw_opengl4/main.c 127
	nk_glfw3_font_stash_end();


	Image* testImg = CreateImagePath("paint.bmp");

	state->CurrentImage = testImg;
	struct nk_context* ctx = state->ctx;



	while(!glfwWindowShouldClose(state->window))
	{
		glfwPollEvents();

		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


		nk_glfw3_new_frame();

		if (nk_begin(ctx, "SimplePaint", nk_rect(0, 0, 1600, 900), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALABLE))
		{
			DrawMenu(state);

			nk_layout_row_begin(ctx, NK_STATIC, nk_window_get_height(ctx) - 20, 2);

			nk_layout_row_push(ctx, 150);

			DrawToolbox(state);

			nk_layout_row_push(ctx, nk_window_get_width(ctx) - 150 - 10);
			DrawViewport(state);
			nk_end(ctx);
		}

		nk_glfw3_render(NK_ANTI_ALIASING_ON);

		glfwSwapBuffers(state->window);
	}


	glfwTerminate();
	return 0;
}

GLFWwindow* InitLibraries()
{
	if (glfwInit() == GLFW_FALSE)
	{
		printf("Failed to initialize GLFW!\n");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(1600, 900, "SimplePaint", NULL, NULL);

	if (window == NULL)
	{
		printf("Failed to initialize window!\n");
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD!\n");
	}

	glViewport(0, 0, 1600, 900);

	return window;
}

APP_STATE* InitApp()
{
	GLFWwindow* window = InitLibraries();
	// Init nuklear
	struct nk_context* ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	// Init App state
	APP_STATE* state = (APP_STATE*)malloc(sizeof(APP_STATE));
	state->ctx = ctx;
	state->SelectedTool = Pencil;
	state->Palette.foreground = (Color){ 0,0,0 };
	state->Palette.background = (Color){ 255,255,255 };
	state->window = window;
	state->CurrentPath = "";
	state->LastMouseX = -1;
	state->LastMouseY = -1;
	state->BrushSize = 1;

	return state;
}

void DrawToolbox(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;



	if (nk_group_begin(ctx,"Toolbox",NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		// TODO : FIGURE OUT PADDINGS
		// Tools

		nk_layout_row_dynamic(ctx, 200, 1);
		if (nk_group_begin(ctx, "Tools",0))
		{

			nk_layout_row_static(ctx, 50, 50, 2);

			if (nk_button_label(ctx, "Pencil"))
			{
				state->SelectedTool = Pencil;
			}

			if (nk_button_label(ctx, "Brush"))
			{
				state->SelectedTool = Brush;
			}

			if (nk_button_label(ctx, "Eraser"))
			{
				state->SelectedTool = Eraser;
			}

			if (nk_button_label(ctx, "Color picker"))
			{
				state->SelectedTool = ColorPicker;
			}
			
			nk_layout_row_dynamic(ctx, 8, 1);

			nk_layout_row_dynamic(ctx, 50, 1);
			nk_property_int(ctx, "Brush size", 1, &state->BrushSize, 20, 1, 0.5f);
			nk_group_end(ctx);
		}

		// Color
		nk_layout_row_dynamic(ctx, 500, 1);
		if (nk_group_begin(ctx, "Colors", NK_WINDOW_NO_SCROLLBAR))
		{
			struct nk_rect r = nk_window_get_content_region(ctx);

			float picker_size = r.w < r.h ? r.w : r.h;
			if (picker_size > 120)
				picker_size = 120;

			nk_layout_row_static(ctx, picker_size, picker_size, 1);

			Color fg = state->Palette.foreground;
			struct nk_colorf color = nk_color_picker(ctx, ColorToNKf(fg), NK_RGB);
			
			state->Palette.foreground = NKftoColor(color);
			
			// TODO : Make color palette, that can save up to 16 colors : LBM - uses color RBM - loads color MMB - resets to white
			nk_group_end(ctx);
		}


		nk_group_end(ctx);
	}

}

void DrawViewport(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;
	Image* image = state->CurrentImage;

	if (nk_group_begin(ctx, "Viewport", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		struct nk_rect content = nk_window_get_content_region(ctx);

		float pad_x = (content.w - image->Width) * 0.5f;
		float pad_y = (content.h - image->Height) * 0.5f;

		if (pad_x < 0) pad_x = 0;
		if (pad_y < 0) pad_y = 0;

		nk_layout_row_dynamic(ctx,0.5 * pad_y, 1);
		nk_spacing(ctx, 1);

		
		nk_layout_row_begin(ctx, NK_STATIC, image->Height, 3);

		nk_layout_row_push(ctx, pad_x);
		nk_spacing(ctx, 1);
		nk_layout_row_push(ctx, image->Width);
		struct nk_rect bound = nk_widget_bounds(ctx);
		nk_image(ctx, image->handle);
		nk_layout_row_push(ctx, pad_x);

		nk_layout_row_end(ctx);

		if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT))
		{
			
			struct nk_vec2 mouse = ctx->input.mouse.pos;

			if (mouse.x >= bound.x && mouse.x < bound.x + bound.w && mouse.y >= bound.y && mouse.y < bound.y + bound.h)
			{
				int localX = (int)mouse.x - bound.x;
				int localY = (int)mouse.y - bound.y;

				if (localX >= 0 && localY >= 0 && localX < image->Width && localY < image->Height)
				{
					int index = (localY * image->Width + localX) * 3;
					int R = image->Data[index];
					int G = image->Data[index + 1];
					int B = image->Data[index + 2];

					switch (state->SelectedTool)
					{
					case ColorPicker:
						state->Palette.foreground = (Color){ R,G,B };
						break;
					case Pencil:
						DrawPencil(state, localX, localY,state->Palette.foreground,1);
						break;
					case Eraser:
						DrawPencil(state, localX, localY, state->Palette.background,state->BrushSize);
						break;
					case Brush:
						DrawPencil(state, localX, localY, state->Palette.foreground,state->BrushSize);
					}
				}
				state->LastMouseX = localX;
				state->LastMouseY = localY;
			}
		}
		else
		{
			state->LastMouseX = -1;
			state->LastMouseY = -1;
		}
		nk_group_end(ctx);
	}

}

void DrawMenu(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;
	
	static bool NewImageFlag = false;

	nk_menubar_begin(ctx);

	nk_layout_row_static(ctx, 10, 50, 2);

	if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(150, 200)))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_menu_item_label(ctx, "New", NK_TEXT_LEFT))
			NewImageFlag = true;

		if (nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT))
		{
			const char* path = openFile(state,"Bitmap (*.bmp)\0*.bmp\0");
			if (path != NULL)
			{
				FreeImage(state->CurrentImage);
				state->CurrentImage = CreateImagePath(path);
				state->CurrentPath = path;
				state->ShouldCreateFile = false;
			}
		}
		if(nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT))
		{
			if(!state->ShouldCreateFile)
				SaveImage(state->CurrentImage, state->CurrentPath);
			else
			{
				const char* path = saveFile(state, "Bitmap (*.bmp)\0*.bmp\0");
				if (path != NULL)
				{
					state->CurrentPath = path;
					state->ShouldCreateFile = false;
					SaveImage(state->CurrentImage, state->CurrentPath);
				}
			}
		}
		nk_menu_end(ctx);
	}

	if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(150, 200)))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_menu_item_label(ctx, "About", NK_TEXT_LEFT);
		nk_menu_end(ctx);
	}

	nk_menubar_end(ctx);

	if(NewImageFlag)
	{
		if (nk_popup_begin(ctx, NK_POPUP_STATIC, "New image", NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE, nk_rect(100, 100, 220, 150)))
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Create new image", NK_TEXT_CENTERED);

			static int width = 200, height = 200;
			nk_property_int(ctx, "Width", 1, &width, 4096, 1, 0.5f);
			nk_property_int(ctx, "Height", 1, &height, 4096, 1, 0.5f);


			if (nk_button_label(ctx, "Create"))
			{
				FreeImage(state->CurrentImage);
				unsigned char* data = (unsigned char*)calloc((size_t)width * height * 3, sizeof(unsigned char));

				memset(data, 255, (size_t)width * height * 3);
				
				state->CurrentImage = CreateImage(width, height, data);
				state->CurrentPath = "";
				state->ShouldCreateFile = true;
				nk_popup_close(ctx);
				NewImageFlag = false;
			}
			if (nk_button_label(ctx, "Close"))
			{
				nk_popup_close(ctx);
				NewImageFlag = false;
			}
			nk_popup_end(ctx);
		}

	}

}

void DrawPencil(APP_STATE* state, unsigned int x, unsigned int y, Color c, int BrushSize)
{
	Image* image = state->CurrentImage;

	if (state->LastMouseX > -1 && state->LastMouseY > -1)
	{
		DrawLine(state, x, y, state->LastMouseX, state->LastMouseY,c,BrushSize);
	}
	else
	{
		if (BrushSize == 1)
		{
			DrawPoint(state, x, y, c);
		}
		else
		{
			int cx = x;
			int cy = y;
			int radius = (BrushSize+1) * 0.5;
			
			for (int i = cx - radius; i < cx + radius; ++i)
			{
				for (int j = cy - radius; j < cy + radius; ++j)
				{
					int dx = i - cx;
					int dy = j - cy;
					float dist2 = dx * dx + dy * dy; // I will compare on squared values to omit square roots
					if(dist2 <= radius*radius)
					{
						if (i < image->Width && j < image->Height && i >= 0 && j >= 0)
						{
							DrawPoint(state, i, j, c);
						}
					}
				}
			}


		}
	}
	UpdateImage(image);
}


void DrawLine(APP_STATE* state, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, Color c, int BrushSize)
{
	Image* image = state->CurrentImage;

	int radius = (BrushSize+1) * 0.5;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;


	int err = dx - dy;

	while (x0 != x1 || y0 != y1)
	{
		int cx = x0;
		int cy = y0;


		for (int i = cx - radius; i < cx + radius; ++i)
		{
			for (int j = cy - radius; j < cy + radius; ++j)
			{
				int dx = i - cx;
				int dy = j - cy;
				float dist2 = dx * dx + dy * dy; // I will compare on squared values to omit square roots
				if (dist2 <= radius * radius)
				{
					if (i < image->Width && j < image->Height && i >= 0 && j >= 0)
					{
						DrawPoint(state, i, j, c);
					}
				}
			}
		}


		int e2 = 2 * err;
		if (e2 > -dy)
		{
			x0 += sx;
			err -= dy;
		}
		if(e2 < dx)
		{
			y0 += sy;
			err += dx;
		}
		
	}
	UpdateImage(image);
}

void DrawPoint(APP_STATE* state, unsigned int x, unsigned int y, Color c)
{
	Image* image = state->CurrentImage;
	int index = (y * image->Width + x) * 3;
	image->Data[index] = c.R;
	image->Data[index + 1] = c.G;
	image->Data[index + 2] = c.B;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}