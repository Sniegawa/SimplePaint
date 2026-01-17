#include "structs.h"

// Draw pencil checks if it should use DrawLine function or DrawPoint
void DrawPencil(APP_STATE* state, unsigned int x, unsigned int y, Color c, int BrushSize);
void DrawLine(APP_STATE* state, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, Color c, int BrushSize);
void DrawPoint(APP_STATE* state, unsigned int x, unsigned int y, Color c);
