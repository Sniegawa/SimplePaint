#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "structs.h"


GLFWwindow* InitLibraries();
APP_STATE* InitApp();

void APP_OPEN(APP_STATE* state);
void APP_NEW(APP_STATE* state);
void APP_SAVE(APP_STATE* state);
void APP_SAVEAS(APP_STATE* state);
void CheckShortcuts(APP_STATE* state);