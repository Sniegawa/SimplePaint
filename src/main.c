#include "structs.h"
#include "colors.h"
#include "image.h"
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

#define PALETTE_SIZE 16

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
	// Initialize app state
	APP_STATE* state = InitApp();

	// Creating alias for ctx
	struct nk_context* ctx = state->ctx;

	// I use those to dynamically resize the apps window based on glfw window
	int WindowWidth, WindowHeight;

	// Main loop
	while(!glfwWindowShouldClose(state->window))
	{

		glfwPollEvents();

		glfwGetFramebufferSize(state->window, &WindowWidth, &WindowHeight);
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


		nk_glfw3_new_frame();

		if (nk_begin(ctx, "SimplePaint", nk_rect(0, 0, WindowWidth, WindowHeight), NK_WINDOW_NO_SCROLLBAR))
		{
			DrawMenu(state);

			float ViewToolSize = nk_window_get_height(ctx) - 20; // sizeof window - 20px for menubar
			nk_layout_row_begin(ctx, NK_STATIC, ViewToolSize, 2); // Layout for Tools and Viewport

			float ToolboxWidth = 150.0f;  // Toolbox has to have exactly 150 px of width
			nk_layout_row_push(ctx, ToolboxWidth);
			DrawToolbox(state);

			float ViewportWidth = nk_window_get_width(ctx) - ToolboxWidth - 10.0f;
			nk_layout_row_push(ctx, ViewportWidth); 
			DrawViewport(state);

			nk_end(ctx);
		}

		nk_glfw3_render(NK_ANTI_ALIASING_ON);

		glfwSwapBuffers(state->window);
	}
	nk_free(ctx);
	free(state->Palette.colorsArray);
	glfwDestroyWindow(state->window);
	free(state);
	glfwTerminate();
	return 0;
}

GLFWwindow* InitLibraries()
{
	if (glfwInit() == GLFW_FALSE)
	{
		printf("Failed to initialize GLFW!\n");
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const unsigned int InitialWidth = 1600;
	const unsigned int InitialHeight = 900;
	GLFWwindow* window = glfwCreateWindow(InitialWidth, InitialHeight, "SimplePaint", NULL, NULL);

	if (window == NULL)
	{
		printf("Failed to initialize window!\n");
		glfwTerminate();
		return NULL;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD!\n");
	}

	glViewport(0, 0, InitialWidth, InitialHeight);

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
	state->LastMouseY = -1;
	state->LastMouseX = -1;
	state->BrushSize = 1;
  state->ShouldCreateFile = true;
	// Init color palette
	state->Palette.colorsArray = (Color*)malloc(sizeof(Color) * PALETTE_SIZE);
	for (int i = 0; i < PALETTE_SIZE; ++i)
		state->Palette.colorsArray[i] = (Color){ 255,255,255 };

	// Load fonts
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&atlas);
	// For loading fonts Nuklear/demo/glfw_opengl4/main.c 127
	nk_glfw3_font_stash_end();

	// Create the initial background image
	Image* testImg = CreateBlankImage(800, 800);
	state->CurrentImage = testImg;

	return state;
}

void DrawToolbox(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;

	if (nk_group_begin(ctx,"Toolbox",NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		// Tools

		nk_layout_row_dynamic(ctx, 200, 1);
		if (nk_group_begin(ctx, "Tools",0))
		{
			// grid of 50x50 boxes 2 cols wide
			nk_layout_row_static(ctx, 50, 50, 2);

			if (nk_button_label(ctx, "Pencil"))
				state->SelectedTool = Pencil;

			if (nk_button_label(ctx, "Brush"))
				state->SelectedTool = Brush;

			if (nk_button_label(ctx, "Eraser"))
				state->SelectedTool = Eraser;

			if (nk_button_label(ctx, "Color picker"))
				state->SelectedTool = ColorPicker;

			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_int(ctx, "Size:", 1, &state->BrushSize, 20, 1, 0.5f);
			nk_group_end(ctx);
		}

		// Color
		nk_layout_row_dynamic(ctx, 300, 1);
		if (nk_group_begin(ctx, "Colors", NK_WINDOW_NO_SCROLLBAR))
		{
			struct nk_rect r = nk_window_get_content_region(ctx);

			// Make color picker square
			float picker_size = r.w < r.h ? r.w : r.h;
			if (picker_size > 120)
				picker_size = 120;

			nk_layout_row_static(ctx, picker_size, picker_size, 1);

			Color fg = state->Palette.foreground;
			struct nk_colorf color = nk_color_picker(ctx, ColorToNKf(fg), NK_RGB);
			
			state->Palette.foreground = NKftoColor(color);

			//Grid for palette colors
			nk_layout_row_dynamic(ctx, 32, 4);

			Color* Palette = state->Palette.colorsArray;

			for (int i = 0; i < PALETTE_SIZE; i++)
			{
				struct nk_color nkc = ColorToNK(Palette[i]);

				nk_style_push_style_item(ctx, &ctx->style.button.normal, nk_style_item_color(nkc));
				nk_style_push_style_item(ctx, &ctx->style.button.hover, nk_style_item_color(nkc));
				nk_style_push_style_item(ctx, &ctx->style.button.active, nk_style_item_color(nkc));

				struct nk_rect bounds;

				bounds = nk_widget_bounds(ctx);
				nk_button_label(ctx, "");

				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_LEFT, bounds))
				{
					state->Palette.foreground = Palette[i];
				}

				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_RIGHT, bounds))
				{
					Palette[i] = state->Palette.foreground;
				}

				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_MIDDLE, bounds))
				{
					Palette[i] = (Color){ 255,255,255 };
				}

				nk_style_pop_style_item(ctx);
				nk_style_pop_style_item(ctx);
				nk_style_pop_style_item(ctx);

			}

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
  static bool AboutFlag = false;
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
    if(nk_menu_item_label(ctx,"Save as",NK_TEXT_LEFT))
    {
      const char* path = saveFile(state, "Bitmap (*.bmp)\0*.bmp\0");
			if (path != NULL)
			{
				state->CurrentPath = path;
				state->ShouldCreateFile = false;
				SaveImage(state->CurrentImage, state->CurrentPath);
			} 
    }
		nk_menu_end(ctx);
	}

	if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(150, 200)))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		if(nk_menu_item_label(ctx, "About", NK_TEXT_LEFT))
      AboutFlag = true;
		nk_menu_end(ctx);
	}

	nk_menubar_end(ctx);

	if(NewImageFlag)
	{
		if (nk_popup_begin(ctx, NK_POPUP_STATIC, "New image", NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR, nk_rect(100, 100, 220, 180)))
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Create new image", NK_TEXT_CENTERED);

			static int width = 200, height = 200;
			nk_property_int(ctx, "Width", 1, &width, 4096, 1, 0.5f);
			nk_property_int(ctx, "Height", 1, &height, 4096, 1, 0.5f);


			if (nk_button_label(ctx, "Create"))
			{
				FreeImage(state->CurrentImage);
				
				state->CurrentImage = CreateBlankImage(width, height);
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

  if(AboutFlag)
  {
    if (nk_popup_begin(ctx,NK_POPUP_STATIC,"About",NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR, nk_rect(0,0,200,120)))
    {
      nk_layout_row_dynamic(ctx, 30, 1);
      nk_label(ctx, "About", NK_TEXT_CENTERED);

      nk_label(ctx, "Author : Mikolaj Lisowski", NK_TEXT_CENTERED);
      

      if (nk_button_label(ctx, "Close"))
      {
        nk_popup_close(ctx);
        AboutFlag = false;
      }
      nk_popup_end(ctx);
    }
  }

}

void DrawPencil(APP_STATE* state, unsigned int x, unsigned int y, Color c, int BrushSize)
{
	Image* image = state->CurrentImage;

	if (state->LastMouseX > -1 && state->LastMouseY > -1) // Check if user was holding LMB
	{
		DrawLine(state, x, y, state->LastMouseX, state->LastMouseY, c, BrushSize); // If user was holding LMB draw line to avoid leaving gaps
	}
	else // If it's user first click in canvas
	{
		if (BrushSize == 1) // If brush size is just 1 draw simple point
		{
			DrawPoint(state, x, y, c);
		}
		else
		{
			// Simple algorithm to draw pixels in a radius around a pixel
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
					if(dist2 <= radius*radius && i < image->Width && j < image->Height && i >= 0 && j >= 0)
							DrawPoint(state, i, j, c);
				}
			}
		}
	}
	UpdateImage(image); // To avoid updating the gpu texture too much i update it once after all changes were made
}

void DrawLine(APP_STATE* state, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, Color c, int BrushSize)
{
	Image* image = state->CurrentImage;

	int radius = (BrushSize+1) * 0.5;

	// Bresenham’s Line algorithm

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
				float dist2 = dx * dx + dy * dy; // I will compare squared values to omit square roots
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
