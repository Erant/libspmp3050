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
			spmp_surface_drawChar(
					surface,
					character,
					x + pos,
					y - character->ascent
					);

			pos += character->advance;
		} else
			/* hack */
			pos+=font->space_advance;

		str++;
	}

}
