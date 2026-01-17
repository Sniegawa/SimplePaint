#include "app.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


int main(int argc, char** argv)
{
	char* openedFile = "";
	if (argc >= 2)
		openedFile = argv[1];

	// Initialize app state
	APP_STATE* state = InitApp(openedFile);

	// Run the app
	RunApp(state);

	// Clear resources afterwards
	CleanupApp(state);

	return 0;
}

