#include "../structs.h"

#include "../OpenFile.h"

#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 512

const char* RunDialogueCommand(const char* command)
{
    char buffer[BUFFER_SIZE];
    size_t result_size = 0;
    char* result = NULL;

    FILE* pipe = popen(command, "r");
    if (!pipe)
        return NULL;

    while (fgets(buffer, sizeof(buffer), pipe))
    {
        size_t len = strlen(buffer);
        char* new_result = realloc(result, result_size + len + 1);
        if (!new_result)
        {
            free(result);
            pclose(pipe);
            return NULL;
        }

        result = new_result;
        memcpy(result + result_size, buffer, len);
        result_size += len;
        result[result_size] = '\0';

        glfwPollEvents(); // keep window responsive
    }

    pclose(pipe);

    // Remove trailing newline
    if (result_size > 0 && result[result_size - 1] == '\n')
        result[result_size - 1] = '\0';

    return result;
}

const char* openFile(APP_STATE* state, const char* filter)
{
    (void)filter; // unused for now

    const char* cmd = NULL;

    if (system("which zenity > /dev/null 2>&1") == 0)
        cmd = "zenity --file-selection 2>/dev/null";
    else if (system("which kdialog > /dev/null 2>&1") == 0)
        cmd = "kdialog --getopenfilename 2>/dev/null";
    else
    {
        printf("No tool found to display file dialog\n");
        return NULL;
    }

    return RunDialogueCommand(cmd);
}

const char* saveFile(APP_STATE* state, const char* filter)
{
    (void)filter; // unused for now

    const char* cmd = NULL;

    if (system("which zenity > /dev/null 2>&1") == 0)
        cmd = "zenity --file-selection --save --confirm-overwrite 2>/dev/null";
    else if (system("which kdialog > /dev/null 2>&1") == 0)
        cmd = "kdialog --getopenfilename 2>/dev/null";
    else
    {
        printf("No tool found to display file dialog\n");
        return NULL;
    }

    return RunDialogueCommand(cmd);
}
