#include <stdlib.h>
#include <stdio.h>

#include "bitmap_fonts.h"
#include "surface.h"
#include "surface16bpp.h"

/*
void font_draw_string_16(unsigned char * str, int x, int y, font_file * font, unsigned short * screen, int screen_width, int screen_height)
{

	char first = 1;
	unsigned char c;
	font_character_entry * character_entry;

	while (*str!=0)
	{
		character_entry = font_find_character(font, *str);
		if (character_entry!=NULL)
		{
			if (!first) x += character_entry->horizontal_shift;
			first = 0;
			font_draw_16(
				character_entry,
				x,
				y - character_entry->height + character_entry->vertical_shift,
				screen,
				screen_width,
				screen_height
				);
			x += character_entry->width;
			x += 1;
		} else
			x+=font->font_size/4;

		str++;
	}

}
*/

int main(int argc, char *argv[])
{
	spmp_bitmapFont * font;
	spmp_surface * surface;

	/* init LCD */
	if (spmp_surface16bpp_init()==SPMP_SURFACE16BPP_INIT_FAIL)
	{
		printf("Couldn't open LCD device, exiting...\n");
		return -1;
	}

	/* create surface */
	surface = spmp_surface16bpp_create();
	if (surface==NULL)
	{
		printf("Couldn't create surface, exiting...\n");
		return -1;
	}

	/* create a new font */
	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "mono12.bfnt");

	/* draw test character */
	spmp_surface_drawString(surface, "Hello world!", 16, 32, font);

	/* blit */
	spmp_surface_blit(surface);

	/* close LCD */
	spmp_surface16bpp_close();

	return 0;
}
