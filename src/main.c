#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string.h>


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>

#define BMP_IMPLEMENTATION
#include <SimpleBMP.h>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

typedef struct Image
{
	GLuint GlTextureID;
	struct nk_image nk_imageHandle;
	unsigned int Width;		//I think w,h is also stored inside struct nk_image
	unsigned int Height;
	unsigned char* Data;
}Image;

//GLFW callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* InitLibraries();

Image* CreateImage(const char* path);
void DrawImage(struct nk_context* ctx, const Image* image);

void DrawToolbox(struct nk_Context* ctx);
void DrawViewport(struct nk_context* ctx, const Image* image);
void DrawMenu(struct nk_context* ctx);

int main(int argc, char** argv)
{
	GLFWwindow* window = InitLibraries();

	//init nuklear
	struct nk_context* ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

	//load fonts
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&atlas);
	// For loading fonts Nuklear/demo/glfw_opengl4/main.c 127
	nk_glfw3_font_stash_end();

	Image* testImg = CreateImage("test.bmp");
	Image* testImg2 = CreateImage("paint.bmp");



	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


		nk_glfw3_new_frame();

		if (nk_begin(ctx, "SimplePaint", nk_rect(0, 0, 800, 600), NK_WINDOW_NO_SCROLLBAR))
		{
			DrawMenu(ctx);

			nk_layout_row_begin(ctx, NK_STATIC, nk_window_get_height(ctx) - 20, 2);

			nk_layout_row_push(ctx, 150);

			DrawToolbox(ctx);

			nk_layout_row_push(ctx, nk_window_get_width(ctx) - 150 - 10);
			DrawViewport(ctx, testImg);
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

Image* CreateImage(const char* path)
{

	BMP_IMAGE* imageData = BMP_LOAD(path);

	Image* img = (Image*)malloc(sizeof(Image));
	int width =  imageData->width;
	int height = imageData->height;
	img->Width = width;
	img->Height = height;
	img->Data = (unsigned char*)malloc(width * height * 3);

	memcpy(img->Data, imageData->pixels, width * height * 3);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width , height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->pixels);

	img->nk_imageHandle = nk_image_id(tex);

	BMP_FREE(imageData);

	return img;
}

void DrawImage(struct nk_context* ctx, const Image* image)
{
	nk_layout_row_static(ctx, image->Height, image->Width, 1);

	nk_image(ctx, image->nk_imageHandle);

}

void DrawToolbox(struct nk_Context* ctx)
{
	if (nk_group_begin(ctx,"Toolbox",NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_static(ctx, 0, 50, 2);
		
		if (nk_button_label(ctx, "Pencil"))
		{
			printf("Selected Pencil\n");
		}

		if (nk_button_label(ctx, "Brush"))
		{
			printf("Selected Brush\n");
		}

		if(nk_button_label(ctx, "Eraser"))
		{
			printf("Selected Eraser\n");
		}

		nk_group_end(ctx);
	}

}

void DrawViewport(struct nk_context* ctx, const Image* image)
{
	if(nk_group_begin(ctx,"Viewport",NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_static(ctx, image->Height, image->Width, 1);

		struct nk_rect bound = nk_widget_bounds(ctx);
		
		nk_image(ctx, image->nk_imageHandle);
		
		if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_LEFT, bound, nk_true))
		{
			struct nk_vec2 mouse = ctx->input.mouse.pos;

			int localX = (int)mouse.x - bound.x;
			int localY = (int)mouse.y - bound.y;


			//This part doesn't work
			int R = image->Data[localY * image->Width + localX];
			int G = image->Data[localY * image->Width + localX + 1];
			int B = image->Data[localY * image->Width + localX + 2];

			printf("Mouse pressed at %d , %d\n color is %d, %d, %d\n", localX, localY, R, G, B);

		}


		nk_group_end(ctx);
	}

}

void DrawMenu(struct nk_context* ctx)
{

	nk_menubar_begin(ctx);


	nk_layout_row_static(ctx, 10, 50, 2);

	if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(150, 200)))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_menu_item_label(ctx, "New", NK_TEXT_LEFT)) printf("a\n");
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

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
