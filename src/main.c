#include <stdio.h>

#define NK_IMPLEMENTATION
#include <nuklear.h>

#include <glad/glad.h>

//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//GLFW callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* InitLibraries();

int main(int argc, char** argv)
{
	GLFWwindow* window = InitLibraries();


	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();




		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
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
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}