#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


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

Image* CreateImage(unsigned char* data, int width, int height);

void DrawImage(struct nk_context* ctx, const Image* image);

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




	unsigned char textureData[4 * 4 * 4] = {
		// Row 0 (top row)
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
				255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
		255,   0,   0, 255,   // Red
		0,   0,   0, 255,   // Black
	};

	Image* testImg = CreateImage(textureData, 4, 4);
	

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		nk_glfw3_new_frame();

		/*nk_begin(ctx, "Demo Window", nk_rect(0, 0, 230, 150),
			NK_WINDOW_TITLE);
		

			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Hello from Nuklear!", NK_TEXT_LEFT);

			if (nk_button_label(ctx, "Press me!"))
				printf("Button pressed!\n");
		
		nk_end(ctx);
		*/

		nk_begin(ctx, "Viewport", nk_rect(230, 0, 500, 500), NK_WINDOW_MOVABLE | NK_WINDOW_BORDER | NK_WINDOW_TITLE);
			//nk_layout_row_static(ctx,8,8,1);
		DrawImage(ctx, testImg);
			//nk_image(ctx, checkerImg);
		nk_end(ctx);



		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


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
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(800, 600, "SimplePaint", NULL, NULL);

	if (window == NULL)
	{
		printf("Failed to initialize window!\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD!\n");
		return -1;
	}

	glViewport(0, 0, 800, 600);

	return window;
}

Image* CreateImage(unsigned char* data, int width, int height)
{
	Image* img = (Image*)malloc(sizeof(Image));
	img->Width = 4;
	img->Height = 4;
	img->Data = (unsigned char*)malloc(sizeof(unsigned char) * 64);
	memcpy_s(img->Data, sizeof(unsigned char) * 64, data, sizeof(unsigned char) * 64);

	GLuint tex = nk_glfw3_create_texture(img->Data, img->Width, img->Height);

	img->nk_imageHandle = nk_image_id(tex);

	return img;
}

void DrawImage(struct nk_context* ctx, const Image* image)
{
	nk_layout_row_static(ctx, image->Width, image->Height, 1);

	nk_image(ctx, image->nk_imageHandle);

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}