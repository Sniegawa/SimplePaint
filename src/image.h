#pragma once

#include "structs.h"

#include <SimpleBMP.h>
#include <nuklear.h>

Image* CreateImagePath(const char* path);
Image* CreateImage(unsigned int width, unsigned int height, const unsigned char* data);
void DrawImage(struct nk_context* ctx, const Image* image);
void FreeImage(Image* img);
void SaveImage(Image* img, const char* path);
void UpdateImage(Image* img);