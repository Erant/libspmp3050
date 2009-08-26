#include <prex/prex.h>
#include <prex/ioctl.h>
#include <sys/ioctl.h>

#include "bitmap_fonts.h"
#include "surface.h"
#include "surface16bpp.h"

static device_t lcddev;

int spmp_surface16bpp_init(void)
{
	if (device_open("lcd", 0, &lcddev))
		return SPMP_SURFACE16BPP_INIT_FAIL;

	/* turn on backlight */
	device_ioctl(lcddev, LCDIOC_SET_BACKLIGHT, 1);



	return SPMP_SURFACE16BPP_INIT_SUCCESS;

}

int spmp_surface16bpp_close(void)
{
	return device_close(lcddev);
}


static void spmp_surface16bpp_drawChar(spmp_surface * surface, spmp_bitmapFontCharacter * character, int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char c, dr, dg, db;
	unsigned short src_pixel;
	int i, j;
	int character_width = character->width;
	int character_height = character->height;
	unsigned char * char_data = character->data;

	int screen_width = surface->width;
	int screen_height = surface->height;
	unsigned short * framebuffer = surface->framebuffer.u16 + surface->stride * y + x;

	unsigned int pos, skip = surface->stride - character_width;

	r >>= 3;
	g >>= 2;
	b >>= 3;

	for (i=0; i<character_height; i++)
	{
		for (j=0; j<character_width; j++)
		{
			c = *(char_data++);
			src_pixel = *(framebuffer);

			dr = ((src_pixel & 31)*(256-c) + r*c) >> 8;
			dg = (((src_pixel>>5) & 63)*(256-c) + g*c) >> 8;
			db = (((src_pixel>>11) & 31)*(256-c) + b*c) >> 8;

			*(framebuffer) = dr | (dg<<5) | (db<<11);
			framebuffer++;
		}
		framebuffer += skip;
	}
}

static void spmp_surface16bpp_blit(spmp_surface * surface)
{
	device_ioctl(lcddev, LCDIOC_SET_FB, surface->framebuffer.u16);
	device_ioctl(lcddev, LCDIOC_DRAW, NULL);
}


spmp_surface * spmp_surface16bpp_create(void)
{

	/* allocate new surface object */
	spmp_surface * ret = (spmp_surface *)malloc(sizeof(spmp_surface));
	if (ret==NULL) return NULL;

	/* allocate framebuffer. If null, return null rather than an invalid surface */
	ret->framebuffer.u8 = (unsigned char*)malloc(320 * 240 * 2);
	if (ret->framebuffer.u8==NULL) return NULL;

	ret->height = 240;
	ret->width = 320;
	ret->stride = 320;

	/* hook implementation */
	ret->drawChar = spmp_surface16bpp_drawChar;
	ret->blit = spmp_surface16bpp_blit;

	return ret;
}

void spmp_surface16bpp_destroy(spmp_surface * surface)
{
	free(surface->framebuffer.u16);
	free(surface);
}
