#pragma once

#include "structs.h"

#include <nuklear.h>

Image* CreateImagePath(const char* path);
Image* CreateImage(unsigned int width, unsigned int height, const unsigned char* data);
Image* CreateBlankImage(unsigned int width, unsigned int height);
void DrawImage(struct nk_context* ctx, const Image* image);
void FreeImage(Image* img);
void SaveImage(Image* img, const char* path);
void UpdateImage(Image* img);
