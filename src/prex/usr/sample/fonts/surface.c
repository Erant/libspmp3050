#include "stddef.h"
#include "surface.h"

void spmp_surface_drawString(spmp_surface * surface, unsigned char *str, int x, int y, spmp_bitmapFont * font)
{

	int pos = 0;
	unsigned char c;
	spmp_bitmapFontCharacter * character;

	while (*str!=0)
	{
		character = spmp_bitmapFont_findCharacter(font, *str);
		if (character!=NULL)
		{
			if (pos>0) pos += character->horizontal_shift;
			spmp_surface_drawChar(
					surface,
					character,
					x + pos,
					y - character->height + character->vertical_shift
					);

			pos += character->width;
			pos += 1;
		} else
			/* hack */
			pos+=font->font_size/4;

		str++;
	}

}
