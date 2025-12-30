#define BMP_IMPLEMENTATION
#include <SimpleBMP.h>

#include "Image.h"


#include <stdlib.h>

#include <glad/glad.h>


Image* CreateImagePath(const char* path)
{

	BMP_IMAGE* imageData = BMP_LOAD(path);

	Image* img = (Image*)malloc(sizeof(Image));
	int width = imageData->width;
	int height = imageData->height;
	img->Width = width;
	img->Height = height;
	size_t dataSize = (size_t)width * height * 3;
	img->Data = (unsigned char*)malloc(dataSize);

	if (!img->Data)
	{
		free(img);
		return NULL;
	}

	memcpy(img->Data, imageData->pixels, dataSize);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->pixels);

	img->handle = nk_image_id(tex);
	img->GlTextureID = tex;

	BMP_FREE(imageData);

	return img;
}

Image* CreateImage(unsigned int width, unsigned int height, const unsigned char* data)
{
	Image* img = (Image*)malloc(sizeof(Image));
	img->Width = width;
	img->Height = height;
	size_t dataSize = (size_t)width * height * 3;
	img->Data = (unsigned char*)malloc(dataSize);

	if (!img->Data)
	{
		free(img);
		return NULL;
	}

	memcpy(img->Data, data, dataSize);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	img->handle = nk_image_id(tex);
	img->GlTextureID = tex;
	free(data);
	return img;

}

Image* CreateBlankImage(unsigned int width, unsigned int height)
{
	Image* img = (Image*)malloc(sizeof(Image));
	img->Width = width;
	img->Height = height;
	size_t dataSize = (size_t)width * height * 3;
	img->Data = (unsigned char*)malloc(dataSize);

	if (!img->Data)
	{
		free(img);
		return NULL;
	}

	unsigned char* data = (unsigned char*)calloc((size_t)width * height * 3, sizeof(unsigned char));
	memset(data, 255, (size_t)width * height * 3);

	memcpy(img->Data, data, dataSize);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	img->handle = nk_image_id(tex);
	img->GlTextureID = tex;
	free(data);
	return img;
}

void FreeImage(Image* img)
{
	// Free the opengl texture memory
	// Note : glDeleteTextures DOES NOT free the memory from the driver, it allows gpu driver to reuse the texture handle, and the memory is still held for reuse
	glDeleteTextures(1, &img->GlTextureID);
	free(img->Data); // Free loaded pixel data
	free(img); // Free the struct
}

void DrawImage(struct nk_context* ctx, const Image* image)
{
	nk_layout_row_static(ctx, image->Height, image->Width, 1);

	nk_image(ctx, image->handle);

}

void SaveImage(Image* img, const char* path)
{
	BMP_IMAGE bmp;
	bmp.width = img->Width;
	bmp.height = img->Height;
	bmp.pixels = img->Data;
	BMP_WRITE(&bmp, path);
}

void UpdateImage(Image* img)
{
	glBindTexture(GL_TEXTURE_2D, img->GlTextureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->Width, img->Height, GL_RGB, GL_UNSIGNED_BYTE, img->Data);
	glBindTexture(GL_TEXTURE_2D, 0);
}