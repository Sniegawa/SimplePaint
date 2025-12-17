#include "structs.h"

#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string.h>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>

#define BMP_IMPLEMENTATION
#include <SimpleBMP.h>




// GLFW callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* InitLibraries();

Image* CreateImagePath(const char* path);
Image* CreateImage(unsigned int width, unsigned int height, const unsigned char* data);
void DrawImage(struct nk_context* ctx, const Image* image);
void FreeImage(Image* img);

// Helper color functions
struct nk_color ColorToNK(Color c);
struct nk_colorf ColorToNKf(Color c);
Color NKftoColor(struct nk_colorf nk_cf);


// Menu drawing functions
void DrawToolbox(APP_STATE* state);
void DrawViewport(APP_STATE* state);
void DrawMenu(APP_STATE* state);
void DrawColorMenu(APP_STATE* state);

int main(int argc, char** argv)
{
	GLFWwindow* window = InitLibraries();

	// Init nuklear
	struct nk_context* ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	// Init App state
	APP_STATE* state = (APP_STATE*)malloc(sizeof(APP_STATE));
	state->ctx = ctx;
	state->SelectedTool = Pencil;
	state->Palette.foreground = (Color){ 255,255,255 };

	// Load fonts
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&atlas);
	// For loading fonts Nuklear/demo/glfw_opengl4/main.c 127
	nk_glfw3_font_stash_end();

	Image* testImg = CreateImagePath("paint.bmp");

	state->CurrentImage = testImg;


	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


		nk_glfw3_new_frame();

		if (nk_begin(ctx, "SimplePaint", nk_rect(0, 0, 800, 600), NK_WINDOW_NO_SCROLLBAR))
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

		glfwSwapBuffers(window);
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


	GLFWwindow* window = glfwCreateWindow(800, 600, "SimplePaint", NULL, NULL);

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

	glViewport(0, 0, 800, 600);

	return window;
}

Image* CreateImagePath(const char* path)
{

	BMP_IMAGE* imageData = BMP_LOAD(path);

	Image* img = (Image*)malloc(sizeof(Image));
	int width =  imageData->width;
	int height = imageData->height;
	img->Width = width;
	img->Height = height;
	size_t dataSize = (size_t)width * height * 3;
	img->Data = (unsigned char*)malloc(dataSize);

	if(!img->Data)
	{
		free(img);
		return NULL;
	}

	memcpy(img->Data, imageData->pixels, dataSize);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width , height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->pixels);

	img->handle = nk_image_id(tex);
	img->GlTextureID = tex;

	BMP_FREE(imageData);

	return img;
}

Image* CreateImage(unsigned int width, unsigned int height, const unsigned char* data)
{
	Image* img = (Image*)malloc(sizeof(Image));
	img->Width = width;
	img->Height = height;
	size_t dataSize = (size_t)width * height * 3;
	img->Data = (unsigned char*)malloc(dataSize);

	if (!img->Data)
	{
		free(img);
		return NULL;
	}

	memcpy(img->Data, data, dataSize);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	img->handle = nk_image_id(tex);
	img->GlTextureID = tex;
	free(data);
	return img;

}

void FreeImage(Image* img)
{
	// Free the opengl texture memory
	// Note : glDeleteTextures DOES NOT free the memory from the driver, it allows gpu driver to reuse the texture handle, and the memory is still held for reuse
	glDeleteTextures(1, &img->GlTextureID);
	free(img->Data); // Free loaded pixel data
	free(img); // Free the struct
}

void DrawImage(struct nk_context* ctx, const Image* image)
{
	nk_layout_row_static(ctx, image->Height, image->Width, 1);

	nk_image(ctx, image->handle);

}

void DrawToolbox(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;

	float height = nk_window_get_height(ctx) - 40;

	if (nk_group_begin(ctx,"Toolbox",NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		// TODO : FIGURE OUT PADDINGS
		// Tools
		nk_layout_row_dynamic(ctx, height * 0.5f - 40, 1);
		if (nk_group_begin(ctx, "Tools",0))
		{

			nk_layout_row_static(ctx, 0, 50, 2);

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
			nk_group_end(ctx);
		}

		// Color
		nk_layout_row_dynamic(ctx, height * 0.5f, 1);
		if(nk_group_begin(ctx,"Colors",0))
		{
			struct nk_rect content = nk_window_get_content_region(ctx);

			nk_layout_row_static(ctx, content.w, content.w, 1);
			Color fg = state->Palette.foreground;
			struct nk_colorf color = nk_color_picker(ctx, ColorToNKf(fg), NK_RGB);
			
			state->Palette.foreground = NKftoColor(color);

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

		if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_LEFT, bound, nk_true))
		{
			struct nk_vec2 mouse = ctx->input.mouse.pos;

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
					state->Palette.foreground = (Color){R,G,B};
					break;
				}
			}
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

		nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT);
		nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
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

void DrawColorMenu(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

struct nk_color ColorToNK(Color c)
{
	struct nk_color nk_c;
	nk_c.r = c.R;
	nk_c.g = c.G;
	nk_c.b = c.B;
	nk_c.a = 255;
	return nk_c;
}

struct nk_colorf ColorToNKf(Color c)
{
	struct nk_colorf nk_cf;
	nk_cf.r = (float)c.R / 255;
	nk_cf.g = (float)c.G / 255;
	nk_cf.b = (float)c.B / 255;
	nk_cf.a = 1.0f;
	return nk_cf;
}

Color NKftoColor(struct nk_colorf nk_cf)
{
	Color c;
	c.R = nk_cf.r * 255;
	c.G = nk_cf.g * 255;
	c.B = nk_cf.b * 255;
	return c;
}