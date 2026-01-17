#include "app.h"

#include "image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_GLFW_GL4_IMPLEMENTATION
#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
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
	state->NewFileFlag = false;

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

void RunApp(APP_STATE* state)
{
	// Creating alias for ctx
	struct nk_context* ctx = state->ctx;

	// I use those to dynamically resize the apps window based on glfw window
	int WindowWidth, WindowHeight;

	// Main loop
	while (!glfwWindowShouldClose(state->window))
	{

		glfwPollEvents();

		glfwGetFramebufferSize(state->window, &WindowWidth, &WindowHeight);
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


		CheckShortcuts(state);

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
	glfwTerminate();
}

void CleanupApp(APP_STATE* state)
{
	nk_free(state->ctx);
	free(state->Palette.colorsArray);
	glfwDestroyWindow(state->window);
	free(state);
}



void APP_OPEN(APP_STATE* state)
{
	const char* path = openFile(state, "Bitmap (*.bmp)\0*.bmp\0");
	if (path != NULL)
	{
		FreeImage(state->CurrentImage);
		state->CurrentImage = CreateImagePath(path);
		state->CurrentPath = path;
		state->ShouldCreateFile = false;
	}
}

void APP_NEW(APP_STATE* state)
{
	state->NewFileFlag = true;
}

void APP_SAVE(APP_STATE* state)
{
	SaveImage(state->CurrentImage, state->CurrentPath);
}

void APP_SAVEAS(APP_STATE* state)
{
	const char* path = saveFile(state, "Bitmap (*.bmp)\0*.bmp\0");
	if (path != NULL)
	{
		state->CurrentPath = path;
		state->ShouldCreateFile = false;
		SaveImage(state->CurrentImage, path);
	}
}

void CheckShortcuts(APP_STATE* state)
{
	GLFWwindow* window = state->window;

	if (glfwGetKey(window, GLFW_KEY_B))
	{
		state->SelectedTool = Brush;
	}
	if (glfwGetKey(window, GLFW_KEY_E))
	{
		state->SelectedTool = Eraser;
	}
	if (glfwGetKey(window, GLFW_KEY_P))
	{
		state->SelectedTool = Pencil;
	}
	if (glfwGetKey(window, GLFW_KEY_I))
	{
		state->SelectedTool = ColorPicker;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
	{
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || state->ShouldCreateFile)
				APP_SAVEAS(state);
			else
				APP_SAVE(state);
		}

		if (glfwGetKey(window, GLFW_KEY_N))
			APP_NEW(state);

		if (glfwGetKey(window, GLFW_KEY_O))
			APP_OPEN(state);

	}
}