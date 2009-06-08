/*
 * Copyright (c) 2005-2006, Kohsuke Ohtani
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <prex/prex.h>
#include <prex/ioctl.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>

#define FONT_LOAD_OK 0
#define FONT_LOAD_FAIL -1
#define FONT_LOAD_FOPEN_FAIL -2
#define FONT_LOAD_HEADER_FAIL -3

#define freadU8(file) (fgetc(file)&0xff)
#define freadU16(file) ( (freadU8(file)<<8) | freadU8(file) )
#define freadU32(file) ( (freadU8(file)<<24) | (freadU8(file)<<16) | (freadU8(file)<<8) | freadU8(file) )

typedef struct _font_file font_file;
typedef struct _font_character_entry font_character_entry;

struct _font_file
{
	int num_characters;
	int font_size;
	font_character_entry * entries;
	unsigned char * data;
};

struct _font_character_entry
{
	unsigned char character;
	unsigned char width, height;
	signed char horizontal_shift;
	signed char vertical_shift;
	unsigned char * data;
};

int load_font(const char *filename, font_file * font)
{
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
	font->num_characters = freadU16(file);
	font->font_size = freadU16(file);
	unsigned int data_size = freadU32(file);

	printf("%d %d\n", font->num_characters, data_size);
	
	/* allocate the glyph entry table */
	font->entries = (font_character_entry*)malloc(sizeof(font_character_entry) * font->num_characters);
	
	/* allocate the data block */
	font->data = (unsigned char*)malloc(data_size);
	
	/* load the glyph table from file and set all the data pointers */
	int i;
	for (i=0; i<font->num_characters; i++)
	{
		font->entries[i].character = fgetc(file);
		font->entries[i].width = fgetc(file);
		font->entries[i].height = fgetc(file);
		font->entries[i].horizontal_shift = fgetc(file);
		font->entries[i].vertical_shift = fgetc(file);
		font->entries[i].data = (unsigned char *)((int)(font->data) + freadU32(file));
	}
	
	/* load bitmap data */
	fread(font->data, 1, data_size, file);
		
	fclose(file);

}

font_character_entry * font_find_character(font_file * font, char c)
{
	int i;
	
	for (i=0; i<font->num_characters; i++)
		if (font->entries[i].character == c)
			return &(font->entries[i]);
			
	/* not found, return null */
	return NULL;
}

void font_draw_16(font_character_entry * character, int x, int y, unsigned short * screen, int screen_width, int screen_height)
{
	
	int i, j;
	
	for (i=0; i<character->height; i++)
	{
		for (j=0; j<character->width; j++)
		{
			char c = (character->data[i*character->width+j]) >> 3;
			screen[(screen_width-1-x-j)*screen_height + (screen_height-1-y-i)] = c | (c<<6) | (c<<11);
		}	
	}	
}

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

int main(int argc, char *argv[])
{
	device_t lcddev;
	uint16_t* fb;
	uint8_t pixel[3];
	/* uint8_t* image = header_data; */
	int i, j, k;
	if(device_open("lcd", 0, &lcddev)){
		printf("Couldn't open LCD device, exiting...\n");
		return -1;
	}

	/* Can result in unaligned framebuffer! */
	fb = malloc(320 * 240 * 2);
	/* fb = memalign(0x400, 320 * 240 * 2); seems to be broken in Prex... */
	if(!fb){
		printf("Couldn't allocate framebuffer, exiting...\n");
		return -1;
	}
	
	device_ioctl(lcddev, LCDIOC_SET_FB, fb);
	device_ioctl(lcddev, LCDIOC_SET_BACKLIGHT, 1);
	printf("Now drawing alternating patterns to the framebuffer for 60 seconds.\n");
	
	for(i = 0; i < 320; i++)
	{
		for(j = 0; j < 240; j++)
		{
			fb[(i * 240) + j] = 0;
		}
	}
	device_ioctl(lcddev, LCDIOC_DRAW, NULL);

	for (i=0; i<320; i++) fb[239-30+i*240] = 31<<11;
	for (i=0; i<320; i++) fb[239-50+i*240] = 31<<11;
	for (i=0; i<320; i++) fb[239-90+i*240] = 31<<11;
	for (i=0; i<320; i++) fb[239-180+i*240] = 31<<11;
	
	int err;
	font_file font;
	
	if (err=load_font("hvdp20.bfnt", &font))
	{
		printf("Error loading font: %d\n", err);
		return -1;
	}
	
	font_draw_string_16("The quick brown fox", 8, 30, &font, (unsigned short*)fb, 320, 240);
	font_draw_string_16("jumps over the lazy dog", 8, 50, &font, (unsigned short*)fb, 320, 240);
	
	if (err=load_font("age11_24.bfnt", &font))
	{
		printf("Error loading font: %d\n", err);
		return -1;
	}
	
	font_draw_string_16("Age 11 handwriting", 8, 90, &font, (unsigned short*)fb, 320, 240);
	
	if (err=load_font("fab32.bfnt", &font))
	{
		printf("Error loading font: %d\n", err);
		return -1;
	}
	
	font_draw_string_16("Fabulous 50's", 8, 180, &font, (unsigned short*)fb, 320, 240);

	if (err=load_font("mono8.bfnt", &font))
	{
		printf("Error loading font: %d\n", err);
		return -1;
	}
	
	font_draw_string_16("This is monospaced 8pt", 8, 200, &font, (unsigned short*)fb, 320, 240);
	font_draw_string_16("A tiny font for the console maybe?", 8, 208, &font, (unsigned short*)fb, 320, 240);
	
	device_ioctl(lcddev, LCDIOC_DRAW, NULL);
	device_ioctl(lcddev, LCDIOC_SET_BACKLIGHT, 1);
	device_close(lcddev);
}
