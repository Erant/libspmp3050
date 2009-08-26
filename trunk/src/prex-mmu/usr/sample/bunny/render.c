#include "types.h"
#include "render.h"
#include "gfx.h"
/* #include "uart.h" */

#include "bunny.h"

u16 tx[BUNNY_VERTEX_COUNT];
u16 ty[BUNNY_VERTEX_COUNT];

extern u16 gfx_queue[131072];

#define FPM(x,y) ((s32)((((s64)(x)) * ((s64)(y))) >> 16))

static int llen = 1;
static u64 lpat = 1;

static u16 fg = 0xffff, bg = 0x0000;

void set_render_colors(u16 f, u16 b)
{
	fg = f;
	bg = b;
}

void set_render_style(int length, u64 pattern)
{
	llen = length;
	lpat = pattern;
}

#define Dprintf(x)

void render_bunny(s32 m[3][3])
{
	int i;	
	//debug_printf("RENDER BUNNY\n");

    Dprintf("Setting the rop\n");
	gfx_set_rop(ROP_PEN);
	gfx_set_color(bg);
/* 	gfx_simple_fill(0,0,240,320); */
    Dprintf("flushing the queue\n");
	gfx_flush_queue();
    Dprintf("waiting a bit\n");
	gfx_wait();

    Dprintf("setting some colors\n");
	gfx_set_color(fg);
	gfx_set_line_style(llen, lpat);
	
    Dprintf("pushing a bunch of vertices\n");
	for(i=0; i<BUNNY_VERTEX_COUNT; i++) {
		s32 x,y,z,nx,ny,nz;
		x = bunny_v[i][0];
		y = bunny_v[i][1];
		z = bunny_v[i][2];
        /* 		printf("Vertex %d %6x %6x %6x\n", i, x, y, z); */
		nx = FPM(m[0][0],x) + FPM(m[0][1],y) + FPM(m[0][2],z);
		ny = FPM(m[1][0],x) + FPM(m[1][1],y) + FPM(m[1][2],z);
		nz = FPM(m[2][0],x) + FPM(m[2][1],y) + FPM(m[2][2],z);
        /* 		printf("       -> %6x %6x %6x\n", x, y, z); */
		nx >>= 16;
		ny >>= 16;
		nx += 160;
		ny += 120;
		tx[i] = nx;
		ty[i] = ny;
        /* 		printf(" -> screen %d: %d,%d\n", i, nx, ny); */
	}
	u16 flags = 0;
	if(llen != 1 || lpat != 1)
		flags |= LINE_FLAG_STYLED;
	for(i=0; i<BUNNY_LINE_COUNT; i++) {
		u16 start, end;
		start = bunny_l[i][0];
		end = bunny_l[i][1];
		//debug_printf("Line %d: %d->%d %d,%d %d,%d\n", i, start, end, ty[start], tx[start], ty[end], tx[end]);
		gfx_simple_line(flags, ty[start], tx[start], ty[end], tx[end]);
	}
	gfx_flush_queue();
    gfx_wait();
}


int main(int argc, char *argv[])
{
  /*
    ",".join([str(int(65536.0*math.sin(x/256.0*2.0*3.1415))) for x in range(256)])
  */

  static s32 sintab[256] = { 0,1608,3215,4820,6423,8022,9615,11203,12785,14358,15923,17478,19023,20556,22077,23585,25078,26557,28019,29464,30892,32301,33691,35060,36408,37735,39038,40318,41574,42805,44010,45188,46339,47463,48557,49623,50658,51663,52637,53580,54490,55367,56211,57021,57796,58537,59242,59912,60546,61143,61704,62227,62713,63161,63571,63943,64276,64570,64826,65042,65220,65358,65456,65516,65535,65516,65457,65358,65220,65043,64827,64571,64277,63944,63572,63162,62715,62229,61706,61145,60548,59915,59245,58540,57799,57024,56214,55370,54493,53583,52641,51667,50662,49627,48561,47467,46344,45193,44014,42809,41579,40323,39043,37740,36413,35065,33696,32306,30897,29470,28024,26562,25084,23591,22083,20562,19029,17484,15929,14364,12791,11209,9621,8028,6429,4827,3221,1614,6,-1602,-3209,-4814,-6417,-8016,-9609,-11197,-12779,-14352,-15917,-17472,-19017,-20551,-22072,-23579,-25073,-26551,-28013,-29459,-30887,-32296,-33686,-35055,-36403,-37730,-39033,-40314,-41569,-42800,-44005,-45184,-46335,-47458,-48553,-49619,-50655,-51660,-52634,-53576,-54486,-55364,-56207,-57018,-57793,-58534,-59240,-59910,-60544,-61141,-61702,-62225,-62711,-63159,-63569,-63941,-64275,-64569,-64825,-65042,-65219,-65357,-65456,-65516,-65535,-65516,-65457,-65359,-65221,-65044,-64828,-64572,-64278,-63945,-63574,-63164,-62716,-62231,-61708,-61148,-60551,-59917,-59248,-58542,-57802,-57027,-56217,-55373,-54496,-53587,-52645,-51671,-50666,-49631,-48566,-47471,-46348,-45197,-44019,-42814,-41583,-40328,-39048,-37745,-36419,-35070,-33701,-32312,-30903,-29475,-28030,-26568,-25090,-23596,-22089,-20568,-19035,-17490,-15935,-14370,-12796,-11215,-9627,-8034,-6435,-4833,-3227,-1620};

  unsigned int frame;
  int lcd = 6;
  unsigned char angle = 0;
  printf("we init some gfx\n");
  if(argc > 1)
	sscanf(argv[1], "%d", &lcd);

  gfx_init(lcd);

  printf("overclocking...\n");
  *((volatile unsigned char*)0x10000123)  = 3;
  *((volatile unsigned char*)0x10000122)  = 1;
      
  printf("so we're going to render a bunny\n");

  for(frame=0;frame<256;frame++)
    {
      s32 matrix[3][3];
      unsigned char x,y;
      for(x=0;x<3;x++)
        for(y=0;y<3;y++)
          matrix[x][y] = 4* (x==y)?-65536:0;

      matrix[0][0] = sintab[angle];
      matrix[1][0] = sintab[(64+angle)&0xff];
      matrix[0][1] = sintab[(64+angle)&0xff];
      matrix[1][1] = -sintab[angle];
      

      fg = frame | ((255-frame)<<8);
      /*       printf("we set up a matrix\n"); */
      render_bunny( matrix );

      gfx_blit();

      angle++;
    }
  printf("and it worked perfectly fine, lets see if we can get back to prex\n");
  return 0;
}
