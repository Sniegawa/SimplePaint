#pragma once

#include "structs.h"

#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>


struct nk_color ColorToNK(Color c)
{
	struct nk_color nk_c;
	nk_c.r = c.R;
	nk_c.g = c.G;
	nk_c.b = c.B;
	nk_c.a = 255;
	return nk_c;
}

struct nk_colorf ColorToNKf(Color c)
{
	struct nk_colorf nk_cf;
	nk_cf.r = (float)c.R / 255;
	nk_cf.g = (float)c.G / 255;
	nk_cf.b = (float)c.B / 255;
	nk_cf.a = 1.0f;
	return nk_cf;
}

Color NKftoColor(struct nk_colorf nk_cf)
{
	Color c;
	c.R = nk_cf.r * 255;
	c.G = nk_cf.g * 255;
	c.B = nk_cf.b * 255;
	return c;
}