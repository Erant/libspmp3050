#include "font.c"
#include <string.h>

void drawchar(u16 *sprite, int x, int y, u16 color, int chr, u16 *framebuffer);
void text(int x, int y, u16 color, char *buffer, u16 *framebuffer);

void gp_drawPixel16(int x, int y, u16 color, void* fb){
	u16* framebuffer = (u16*) fb;
	framebuffer[(239-y)+(240*x)] = color;	
	/* framebuffer[240 * x + y] = color; */
}

void drawchar(u16 *sprite, int x, int y, u16 color, int chr, u16 *framebuffer)
{
        int xx, yy, xpos, ypos;
        u16 pixel;

        xpos=(chr % 32)*8;
        ypos=(chr / 32)*16;
        
        for (yy=0;yy<16;yy++)
        {
                for (xx=0;xx<8;xx++)
                {
                        pixel=sprite[xpos+ypos*256];
			if (xx+x<320 && xx+x>0)
                        if(pixel==0) gp_drawPixel16(xx+x,yy+y,color,framebuffer);
                        xpos++;
                }
                xpos=xpos-8;
                ypos++;
        }
}

void text(int x, int y, u16 color, char *buffer, u16 *framebuffer)
{
        int i, len, c;
        len=strlen(buffer);
        for(i=0;i<len;i++)
        {
                c=buffer[i];
                drawchar((u16 *)font,x+i*8,y,color,c, framebuffer);
        }
}

