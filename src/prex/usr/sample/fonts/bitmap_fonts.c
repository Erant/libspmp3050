#include "stdlib.h"
#include "stdio.h"

#include "bitmap_fonts.h"

#define freadU8(file) (fgetc(file)&0xff)
#define freadU16(file) ( (freadU8(file)<<8) | freadU8(file) )
#define freadU32(file) ( (freadU8(file)<<24) | (freadU8(file)<<16) | (freadU8(file)<<8) | freadU8(file) )

spmp_bitmapFont * spmp_bitmapFont_create(void)
{
	spmp_bitmapFont * ret = (spmp_bitmapFont *)malloc(sizeof(spmp_bitmapFont));

	/* set pointers to null, to make sure that a call to spmp_bitmapFont_destroy() doesn't segfault */
	ret->data = NULL;
	ret->characters = NULL;

	return ret;
}

void spmp_bitmapFont_destroy(spmp_bitmapFont * font)
{
	free(font->data);
	free(font->characters);
	free(font);
}

int spmp_bitmapFont_load(spmp_bitmapFont * font, const char *filename)
{

	unsigned int data_size;
	int i;

	/* open file */
	FILE * file = fopen(filename, "rb");
	if (!file)
		return(FONT_LOAD_FOPEN_FAIL);

	/* check magic */
	if (fgetc(file)!='B') return FONT_LOAD_HEADER_FAIL;
	if (fgetc(file)!='F') return FONT_LOAD_HEADER_FAIL;
	if (fgetc(file)!='N') return FONT_LOAD_HEADER_FAIL;
	if (fgetc(file)!='T') return FONT_LOAD_HEADER_FAIL;

	/* get number of glyphs */
	font->bitmap_type = freadU8(file);
	font->font_size = freadU8(file);
	font->num_characters = freadU16(file);

	font->space_advance = freadU16(file);
	font->leading = freadU16(file);

	data_size = freadU32(file);

	/* allocate the glyph entry table */
	font->characters = (spmp_bitmapFontCharacter*)malloc(sizeof(spmp_bitmapFontCharacter) * font->num_characters);

	/* allocate the data block */
	font->data = (unsigned char*)malloc(data_size);

	/* load the glyph table from file and set all the data pointers */
	for (i=0; i<font->num_characters; i++)
	{
		font->characters[i].character = fgetc(file);
		font->characters[i].width = fgetc(file);
		font->characters[i].height = fgetc(file);
		font->characters[i].advance = fgetc(file);
		font->characters[i].ascent = fgetc(file);
		font->characters[i].data = (unsigned char *)((int)(font->data) + freadU32(file));
	}

	/* load bitmap data */
	fread(font->data, 1, data_size, file);

	fclose(file);

	return FONT_LOAD_SUCCESS;

}

spmp_bitmapFontCharacter * spmp_bitmapFont_findCharacter(spmp_bitmapFont * font, char c)
{
	int i;

	for (i=0; i<font->num_characters; i++)
		if (font->characters[i].character == c)
			return &(font->characters[i]);

	/* not found, return null */
	return NULL;
}
