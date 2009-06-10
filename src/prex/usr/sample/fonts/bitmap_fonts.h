#ifndef __bitmap_fonts__
#define __bitmap_fonts__

typedef struct _spmp_bitmapFont spmp_bitmapFont;
typedef struct _spmp_bitmapFontCharacter spmp_bitmapFontCharacter;

enum
{
	FONT_LOAD_SUCCESS=0,
	FONT_LOAD_FOPEN_FAIL=-1,
	FONT_LOAD_HEADER_FAIL=-2
};

struct _spmp_bitmapFont
{
	int num_characters;
	int bitmap_type;
	int font_size;
	int space_advance;
	int leading;
	spmp_bitmapFontCharacter * characters;
	unsigned char * data;
};

struct _spmp_bitmapFontCharacter
{
	unsigned char character;
	unsigned char width, height;
	signed char advance;
	signed char ascent;
	unsigned char * data;
};

spmp_bitmapFont * spmp_bitmapFont_create(void);
void spmp_bitmapFont_destroy(spmp_bitmapFont * font);
int spmp_bitmapFont_load(spmp_bitmapFont * font, const char *filename);
spmp_bitmapFontCharacter * spmp_bitmapFont_findCharacter(spmp_bitmapFont * font, char c);

#endif
