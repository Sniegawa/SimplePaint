#include "app.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


int main(int argc, char** argv)
{
	// Initialize app state
	APP_STATE* state = InitApp();

	// Run the app
	RunApp(state);

	// Clear resources afterwards
	CleanupApp(state);

	return 0;
}

