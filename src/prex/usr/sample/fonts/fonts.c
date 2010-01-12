#include <stdlib.h>
#include <stdio.h>

#include "bitmap_fonts.h"
#include "surface.h"
#include "surface16bpp.h"
#include "imath.h"

int main(int argc, char *argv[])
{
	int x, y, i;
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

	for (y=0; y<240; y++)
		for (x=0; x<320; x++)
			surface->framebuffer.u16[y*320+x] = quicksin(x, 15) + 15;

	/* create a new font */
	/*
	int baseline = 40;

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "goudy32.bfnt");
	spmp_surface_drawString(surface, "Goudy 32pt.", 12, baseline, font, 0, 0, 0);
	spmp_surface_drawString(surface, "The quick brown fox", 12, 168, font, 189, 21, 80);
	spmp_surface_drawString(surface, "jumps over the lazy dog.", 12, 168 + font->leading, font, 138, 155, 15);
	baseline += font->leading;
	spmp_bitmapFont_destroy(font);
	spmp_surface_blit(surface);

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "italic24.bfnt");
	spmp_surface_drawString(surface, "Italic nice 24pt.", 12, baseline, font, 0, 0, 0);
	baseline += font->leading;
	spmp_bitmapFont_destroy(font);
	spmp_surface_blit(surface);

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "mono24.bfnt");
	spmp_surface_drawString(surface, "Monospace 24pt.", 12, baseline, font, 0, 0, 0);
	baseline += font->leading;
	spmp_bitmapFont_destroy(font);
	spmp_surface_blit(surface);

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "mono16.bfnt");
	spmp_surface_drawString(surface, "Monospace 16pt.", 12, baseline, font, 0, 0, 0);
	baseline += font->leading;
	spmp_bitmapFont_destroy(font);
	spmp_surface_blit(surface);

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "mono12.bfnt");
	spmp_surface_drawString(surface, "Monospace 12pt.", 12, baseline, font, 0, 0, 0);
	font->tracking = -1;

	x = 12;
	for (i=0; i<6; i++)
	{
		font->tracking = -3 + i;
		char * str = "tracking";
		spmp_surface_drawString(
				surface, str,
				x,
				220,
				font,
				0, 0, 0
				);
		x += spmp_bitmapFont_stringWidth(font, str) + 8;
	}
	baseline += font->leading;
	spmp_bitmapFont_destroy(font);
	spmp_surface_blit(surface);

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "mono8.bfnt");
	spmp_surface_drawString(surface, "Monospace 8pt.", 12, baseline, font, 0, 0, 0);
	baseline += font->leading;
	spmp_bitmapFont_destroy(font);
	spmp_surface_blit(surface);
	*/

	font = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font, "goudy32.bfnt");

	spmp_bitmapFont * font2 = spmp_bitmapFont_create();
	spmp_bitmapFont_load(font2, "mono24.bfnt");

	int l = 0;

	int * tab1, *tab2, *tab3;
	tab1 = (int*)malloc(320*4);
	tab2 = (int*)malloc(320*4);
	tab3 = (int*)malloc(320*4);

	while (1)
	{
		unsigned short * fb = surface->framebuffer.u16;

		for (i=0; i<320; i++) tab1[i] = _sinetable[((_sinetable[(i+l*7)&4095] + _sinetable[(i*5+-l*7+512)&4095]) >> 4) & 4095];
		for (i=0; i<320; i++) tab2[i] = _sinetable[((_sinetable[(i*3-l*5+512)&4095] + _sinetable[(-i*2+l*3+1024)&4095]) >> 4) & 4095];
		for (i=0; i<320; i++) tab3[i] = _sinetable[((_sinetable[(-i*3-l*6+3000)&4095] + _sinetable[(i*4+l*7+2560)&4095]) >> 4) & 4095];

		for (y=0; y<240; y++)
			for (x=0; x<320; x++)
				*(fb++) =
					(((tab1[x] + tab2[y]) >> 13) + 16) |
					((((tab2[x] + tab3[y]) >> 12) + 32) << 5) |
					((((tab3[x] + tab1[y]) >> 13) + 16) << 11);

		l+=3;

		/*
		char * str = "SPMP Prex FTW";
		int w = spmp_bitmapFont_stringWidth(font, str);
		spmp_surface_drawString(surface, str, (320-w)/2, 140, font, 255, 255, 255);

		str = "leipe shit, ouwe!";
		w = spmp_bitmapFont_stringWidth(font2, str);
		spmp_surface_drawString(surface, str, (320-w)/2, 160, font2, 255, 255, 255);
		*/

		spmp_surface_blit(surface);
	}

	spmp_bitmapFont_destroy(font);

	/* close LCD */
	spmp_surface16bpp_close();

	return 0;
}
