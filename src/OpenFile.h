#pragma once

#pragma once

typedef struct APP_STATE APP_STATE; // Forward declaration

const char* openFile(APP_STATE* state, const char* filter);
const char* saveFile(APP_STATE* state, const char* filter);