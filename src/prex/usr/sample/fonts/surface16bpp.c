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


static void spmp_surface16bpp_drawChar(spmp_surface * surface, spmp_bitmapFontCharacter * character, int x, int y)
{

	int i, j;
	int character_width = character->width;
	int character_height = character->height;
	unsigned char * character_data = character->data;

	int screen_width = surface->width;
	int screen_height = surface->height;
	int screen_stride = surface->stride;
	unsigned short * framebuffer = surface->framebuffer.u16;

	for (i=0; i<character_height; i++)
	{
		for (j=0; j<character_width; j++)
		{
			char c = (character_data[i*character_width+j]) >> 3;
			framebuffer[(screen_width-1-x-j)*screen_stride + (screen_height-1-y-i)] = c | (c<<6) | (c<<11);
		}
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
	ret->stride = 240;

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
