#ifndef __surface__
#define __surface__

#include "bitmap_fonts.h"

typedef struct _spmp_surface spmp_surface;

/* platform specific paint code */
typedef void (*spmp_surface_blit_FP)(spmp_surface * surface);
typedef void (*spmp_surface_drawChar_FP)(spmp_surface * surface, spmp_bitmapFontCharacter * character, int x, int y, unsigned char r, unsigned char g, unsigned char b);

struct _spmp_surface
{
		int bpp;
		int width, height, stride;

		union
		{
			unsigned char * u8;
			unsigned short * u16;
			unsigned int * u32;
		} framebuffer;


		spmp_surface_blit_FP blit;
		spmp_surface_drawChar_FP drawChar;
};

void spmp_surface_drawString(spmp_surface * surface, unsigned char *str, int x, int y, spmp_bitmapFont * font, unsigned char r, unsigned char g, unsigned char b);

/* convenience methods */

static void spmp_surface_blit(spmp_surface * surface)
{
	surface->blit(surface);
}

static void spmp_surface_drawChar(spmp_surface * surface, spmp_bitmapFontCharacter * character, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	surface->drawChar(surface, character, x, y, r, g, b);
}

#endif
