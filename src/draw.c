#include "draw.h"
#include "image.h"

void DrawPencil(APP_STATE* state, unsigned int x, unsigned int y, Color c, int BrushSize)
{
	Image* image = state->CurrentImage;

	if (state->LastMouseX > -1 && state->LastMouseY > -1) // Check if user was holding LMB
	{
		DrawLine(state, x, y, state->LastMouseX, state->LastMouseY, c, BrushSize); // If user was holding LMB draw line to avoid leaving gaps
	}
	else // If it's user first click in canvas
	{
		if (BrushSize == 1) // If brush size is just 1 draw simple point
		{
			DrawPoint(state, x, y, c);
		}
		else
		{
			// Simple algorithm to draw pixels in a radius around a pixel
			int cx = x;
			int cy = y;
			int radius = (BrushSize + 1) * 0.5;

			for (int i = cx - radius; i < cx + radius; ++i)
			{
				for (int j = cy - radius; j < cy + radius; ++j)
				{
					int dx = i - cx;
					int dy = j - cy;
					float dist2 = dx * dx + dy * dy; // I will compare on squared values to omit square roots
					if (dist2 <= radius * radius && i < image->Width && j < image->Height && i >= 0 && j >= 0)
						DrawPoint(state, i, j, c);
				}
			}
		}
	}
	UpdateImage(image); // To avoid updating the gpu texture too much i update it once after all changes were made
}

void DrawLine(APP_STATE* state, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, Color c, int BrushSize)
{
	Image* image = state->CurrentImage;

	int radius = (BrushSize + 1) * 0.5;

	// Bresenham’s Line algorithm

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;


	int err = dx - dy;

	while (x0 != x1 || y0 != y1)
	{
		int cx = x0;
		int cy = y0;


		for (int i = cx - radius; i < cx + radius; ++i)
		{
			for (int j = cy - radius; j < cy + radius; ++j)
			{
				int dx = i - cx;
				int dy = j - cy;
				float dist2 = dx * dx + dy * dy; // I will compare squared values to omit square roots
				if (dist2 <= radius * radius)
				{
					if (i < image->Width && j < image->Height && i >= 0 && j >= 0)
					{
						DrawPoint(state, i, j, c);
					}
				}
			}
		}


		int e2 = 2 * err;
		if (e2 > -dy)
		{
			x0 += sx;
			err -= dy;
		}
		if (e2 < dx)
		{
			y0 += sy;
			err += dx;
		}

	}
	UpdateImage(image);
}

void DrawPoint(APP_STATE* state, unsigned int x, unsigned int y, Color c)
{
	Image* image = state->CurrentImage;
	int index = (y * image->Width + x) * 3;
	image->Data[index] = c.R;
	image->Data[index + 1] = c.G;
	image->Data[index + 2] = c.B;
}