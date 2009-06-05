#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <prex/prex.h>
#include <sys/ioctl.h>

#include "gp32.h"

#include "gp_font.c"
#include "title.c"

#include "coin.c"
#include "copter.c"
#include "music.c"

void drawcave(int x, int y, int size, u16 *framebuf);
void box(int x, int y, int xx, int yy, u16 color, u16 *framebuf);
void game(int gm);
int title(void);
void redbar(int y, u16 *framebuffer);

u16 px(int x, int y, u16 *framebuffer);

u16 *fb[2];
int a1=0,a2=0;
int score=0;

device_t lcddev;

void gp_clearFramebuffer16(void* buf, u16 val){
	memset(buf, val, 320 * 240 * 2);
}

void gp_setFramebuffer(void* buf, int unk){
	device_ioctl(lcddev, LCDIOC_SET_FB, buf);
	/* Assuming don used doublebuffering, this will work */
	device_ioctl(lcddev, LCDIOC_DRAW, NULL);
}

int gp_getButton(){
	return 0;
}

void gp_Reset(){
	exit(0);
}

void gp_drawLine16(int x0, int y0, int x1, int y1, u16 color, void* fb){
	
}

void gp_drawSpriteT(void* buf, int x, int y, void* fb, u16 transparent, int x_size, int y_size){

}

int main(void){
	int x;
	device_open("lcd", 0, &lcddev);
	device_ioctl(lcddev, LCDIOC_SET_BACKLIGHT, 1);
	fb[0] = (u16*)malloc(320 * 240 * 2);
	fb[1] = (u16*)malloc(320 * 240 * 2);
	gp_clearFramebuffer16(fb[0],0x00);
	gp_clearFramebuffer16(fb[1],0x00);
	gp_setFramebuffer(fb[0],1);
	while(1)
	{
		x=title();
		game(x);
	}
}

int title(void)
{
	int sbuf=0;
	int i;
	int cvy[34];
	int cvs[34];
	int scr=0;
	int chscr=0;
	int scrpos=0;
	int o;
	int menu=2;
	u16 gelb=RGB(31,31,0);
	u16 norm=RGB(16,16,16);
	u16 colz;
	int gmmode=0;
	int gmspd=0;
	int penalty=10;
	int nofire=0;
	unsigned char scroller[]="                                        Cave Copter! Written by donskeeto, ported to the awesome SPMP platform by Erant.        ";

	gmmode=a1;
	gmspd=a2;
	unsigned char xscr[40];
	for(i=0;i<=33;i++)
	{
		cvy[i]=30;
		cvs[i]=180;
	}
	while(nofire==0)
	{
		gp_clearFramebuffer16(fb[sbuf],0x00);
		for(i=0;i<=33;i++)
		{
			drawcave(i*10-scr,cvy[i],cvs[i],fb[sbuf]);
		}
		scr+=2;
		
		if (scr==10)
		{
			scr=0;
			for (i=1;i<=33;i++)
			{
				cvy[i-1]=cvy[i];
				cvs[i-1]=cvs[i];
			}
			if (rand()%2==0) cvy[33]-=4; else cvy[33]+=4;
			
		}
		gp_drawSpriteT((short *)titlegfx,10,10,fb[sbuf],0xFFFE,300,78);
		o=0;
		for(i=scrpos;i<=scrpos+40;i++)
		{
			drawchar((u16 *)font, o*8-chscr-1, 220, 0x00,scroller[i],fb[sbuf]);
			drawchar((u16 *)font, o*8-chscr+1, 220, 0x00,scroller[i],fb[sbuf]);
			drawchar((u16 *)font, o*8-chscr, 219, 0x00,scroller[i],fb[sbuf]);
			drawchar((u16 *)font, o*8-chscr-1, 221, 0x00,scroller[i],fb[sbuf]);
			drawchar((u16 *)font, o*8-chscr, 220,0xFFFF,scroller[i],fb[sbuf]);
			o++;
		}

		sprintf(xscr,"LAST SCORE: %i",score);
		text(100,180,gelb,xscr,fb[sbuf]);
		if (menu==0) redbar(100,fb[sbuf]);
		if (menu==1) redbar(130,fb[sbuf]);
		if (menu==2) redbar(160,fb[sbuf]);
		text(35,100,0xFFFF,"GAME MODE -",fb[sbuf]);
		if(gmmode==0) colz=gelb; else colz=norm;
		text(139,100,colz, "NORMAL",fb[sbuf]);
		if(gmmode==1) colz=gelb; else colz=norm;
		text(195,100,colz, "COIN",fb[sbuf]);
		if(gmmode==2) colz=gelb; else colz=norm;
		text(235,100,colz, "MOVING",fb[sbuf]);
		text(27,130,0xFFFF,"SPEED -",fb[sbuf]);
		if(gmspd==0) colz=gelb; else colz=norm;
		text(91,130,colz,"SLOW",fb[sbuf]);
		if(gmspd==1) colz=gelb; else colz=norm;
		text(131,130,colz,"MEDIUM",fb[sbuf]);
		if(gmspd==2) colz=gelb; else colz=norm;
		text(187,130,colz,"FAST",fb[sbuf]);
		if(gmspd==3) colz=gelb; else colz=norm;
		text(227,130,colz,"MADNESS!",fb[sbuf]);
	
		text(120, 160, 0xFFFF, "LET'S GO!",fb[sbuf]);
		if(penalty==0)
		{
			if(gp_getButton()&BUTTON_DOWN && menu<2)
			{
				menu++;
				penalty=5;
			}
			if(gp_getButton()&BUTTON_UP && menu>0)
			{
				menu--;
				penalty=5;
			}
			if(menu==0)
			{
				if(gp_getButton()&BUTTON_RIGHT && gmmode<2)
				{
					gmmode++;
					penalty=5;
				}
				if(gp_getButton()&BUTTON_LEFT && gmmode>0)
				{
					gmmode--;
					penalty=5;
				}
			}
			if(menu==1)
			{
				if(gp_getButton()&BUTTON_RIGHT && gmspd<3)
				{
					gmspd++;
					penalty=5;
				}
				if(gp_getButton()&BUTTON_LEFT && gmspd>0)
				{
					gmspd--;
					penalty=5;
				}
			}
			if(menu==2)
			{
				if(gp_getButton()&BUTTON_A)
				{
					nofire=1;
				}
			}

		}
		
                gp_setFramebuffer(fb[sbuf],1);
                sbuf=!sbuf;
		chscr++;
		if (chscr==8)
		{
			chscr=0;
			scrpos++;
			if (scrpos==700) scrpos=0;
		}
		if (gp_getButton()&BUTTON_L && gp_getButton()&BUTTON_R && gp_getButton()&BUTTON_START && gp_getButton()&BUTTON_SELECT) gp_Reset();
		if (penalty>0) penalty--;
	}
	return(gmmode*10+gmspd);
}				
		
void game(int gm)
{
	int sbuf=0;
	int i;
	int cvy[34];
	int scr=0;
	int cvs[34];
	int blockx=-30, blocky;
	int speed=5;
	int helix=50, heliy=109;
	int drop=0;
	int thrust=0;
	int counters=0;
	int counterup=0;
	int lift=0;
	int divider=80;
	int mode=1;
	int move=1;
	int fcount=0;
	/* int a1,a2; */
	int coinx=-40, coiny;
	int hit=0;
	unsigned char xscr[20];
	/* clear cave */
	score=0;	
	a1=gm/10;
	a2=gm%10;
	mode=a1;
	if(a2==0) speed=1;
	if(a2==1) speed=2;
	if(a2==2) speed=5;
	if(a2==3) speed=10;
	for(i=0;i<=33;i++)
	{
		cvy[i]=30;
		cvs[i]=180;
	}

	
	while(hit!=1)
	{
		fcount++;
		counters++;
		gp_clearFramebuffer16(fb[sbuf],0x00);
		for(i=0;i<=33;i++)
		{
			drawcave(i*10-scr,cvy[i],cvs[i],fb[sbuf]);
		}
		scr+=speed;
		if (scr==10) 
		{
			scr=0;
			for (i=1;i<=33;i++)
			{
				cvy[i-1]=cvy[i];
				cvs[i-1]=cvs[i];
			}
			if (rand()%2==0)
			{
				if (cvy[33]>5) 
				{
					cvy[33]-=4;
					cvs[33]=180-fcount/100;
				}
			} else
			{
				if (cvy[33]<55) 
				{
					cvy[33]+=4;
					cvs[33]=180-fcount/100;
				}
				
			}
		}
		if (blockx<-20) 	/* spawn block? */
		{
			blockx=320; 	/* block spawned */
			blocky=rand()%180;
			move=rand()%2;
		}
		if (blockx>-30)		/* draw block */
		{
			box(blockx,blocky,blockx+19,blocky+59,RGB(0,28,0),fb[sbuf]);
			blockx-=speed;
			if(mode==2)
			{
				if(move==1)
				{
					if (blocky<180) blocky++;
					if (blocky>=180) move=0;
				}
				if(move==0)
				{
					if (blocky>0) blocky--;
					if (blocky<=0) move=1;
				}
			}
		}
		if (mode==1)
		{
			/* coin mode */

			if (coinx==-40)
			{
				if (rand()%80==0 && blockx<300)
				{
					coinx=320;
					coiny=rand()%121+60;
				}
			}
			
			if (coinx>-40)
			{
				gp_drawSpriteT((short *)coin,coinx,coiny,fb[sbuf],0xFFFE,32,32);
				coinx-=speed;
			}
			if (px(helix-1,heliy-1,fb[sbuf])==0xF740 ||
              		    px(helix+10,heliy+1,fb[sbuf])==0xF740 ||
			    px(helix+31,heliy-1,fb[sbuf])==0xF740 ||
			    px(helix+50,heliy+2,fb[sbuf])==0xF740 ||
			    px(helix+42,heliy+13,fb[sbuf])==0xF740 ||
			    px(helix+42,heliy+22,fb[sbuf])==0xF740 ||
			    px(helix+32,heliy+23,fb[sbuf])==0xF740 ||
			    px(helix+22,heliy+22,fb[sbuf])==0xF740 ||
			    px(helix+15,heliy+13,fb[sbuf])==0xF740 ||
			    px(helix+3,heliy+17,fb[sbuf])==0xF740)
			{ 
				score+=40*speed;
				coinx=-40;
			}
		}
				
				
		/* draw helicopter */
		if (counters<=3) gp_drawSpriteT((short *)copter0,helix,heliy,fb[sbuf],0xFFFE,50,23);
		if (counters>3) gp_drawSpriteT((short *)copter1,helix,heliy,fb[sbuf],0xFFFE,50,23);
		if (counters==6) counters=0;

		/* collision? */

		if (px(helix-1,heliy-1,fb[sbuf])==RGB(0,28,0) || 
		    px(helix+10,heliy+1,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+31,heliy-1,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+50,heliy+2,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+42,heliy+13,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+42,heliy+22,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+32,heliy+23,fb[sbuf])==RGB(0,28,0) ||
	            px(helix+22,heliy+22,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+15,heliy+13,fb[sbuf])==RGB(0,28,0) ||
		    px(helix+3,heliy+17,fb[sbuf])==RGB(0,28,0))
		{
			hit=1;
		}
		/* drop? */
	
		if (gp_getButton()&BUTTON_A) drop--; else drop++;
		if (drop<0) lift=-((drop-9)*(drop-9)/divider);
		if (drop>=0) lift=(drop+9)*(drop+9)/divider;
			
		
		sprintf(xscr,"SCORE: %i",score);
		text(10,220,RGB(31,31,0),xscr,fb[sbuf]);
	
		/* reset? */

		if (gp_getButton()&BUTTON_L && gp_getButton()&BUTTON_R && gp_getButton()&BUTTON_START && gp_getButton()&BUTTON_SELECT) gp_Reset();
		
		heliy+=lift;
		score+=speed;
		
		gp_setFramebuffer(fb[sbuf],1);
		sbuf=!sbuf;
	}
}

void drawcave(int x, int y, int size, u16 *framebuf)
{
	int xx=x+9;
	if (xx>319) xx=320;
	if (x<0) x=0;
	box(x,0,xx,y-1, RGB(0,28,0),framebuf);
/*	box(x,y,xx,y+size, 0x00, framebuf); */
	box(x,y+size+1,xx,319, RGB(0,28,0),framebuf);

}

void box(int x, int y, int xx, int yy, u16 color, u16 *framebuf)
{
	int i;
	for(i=y;i<=yy;i++)
	{
		gp_drawLine16(x,i,xx,i,color,framebuf);
	}
}

u16 px(int x, int y, u16 *framebuffer)
{
	u16 col;
	col=framebuffer[(239-y)+(240*x)];
	return(col);
}

void redbar(int y, u16 *framebuffer)
{
	u16 col;
	int i;
	int cnt=0;
	
	for(i=0;i<=7;i++)
	{
		col=RGB(i*4,0,0);
		gp_drawLine16(0,y+cnt-1,320,y+cnt,col,framebuffer);
		cnt++;
	}
	for(i=7;i>=0;i--)
	{
		col=RGB(i*4,0,0);
		gp_drawLine16(0,y+cnt-1,320,y+cnt,col,framebuffer);
		cnt++;
	}
}
