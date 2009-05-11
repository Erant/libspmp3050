#include <driver.h>

#include <sys/ioctl.h>
#include <sys/tty.h>
#include <prex/ioctl.h>

#include "../../../sys/arch/arm/spmp/platform.h"

/* Forward functions */
static int lcd_init(void);
static int lcd_open(device_t dev, int mode);
static int lcd_close(device_t dev);
static int lcd_read(device_t, char *, size_t *, int);
static int lcd_write(device_t, char *, size_t *, int);
static int lcd_ioctl(device_t, u_long, void *);

/*
 * Driver structure
 */
struct driver lcd_drv = {
	/* name */	"LCD controller",
	/* order */	4,
	/* init */	lcd_init,
};

/*
 * Device I/O table
 */
static struct devio lcd_io = {
	/* open */	lcd_open,
	/* close */	lcd_close,
	/* read */	lcd_read,
	/* write */	lcd_write,
	/* ioctl */	lcd_ioctl,
	/* event */	NULL,
};

static device_t lcd_dev;	/* device object */

int lcd_mode = 0;

void delay_ms( int ms )
{
  static int bananana;
  while(--ms)
    {
      int a;
      for (a=0;a<1000;a++)
        bananana = bananana* 11 ^ bananana + 2;
    }

  /*  timer_delay( ms ); */
}

void delay_us( int us )
{
  delay_ms( 1 + us / 1000 ); /* oh no you didn't (fixme) */
}

void LCD_SetBacklight(int val){
	DEV_ENABLE |= 0x8;
	if(val)
		DEV_ENABLE_OUT |= 0x8;
	else
		DEV_ENABLE_OUT &= ~0x8;
}

void LCD_AddrWrite(uint16_t val){
	LCD_DATA = val;
	
	LCD_DATA_DIR |= LCD_OUT;
	
	LCD_CTRL = LCD_CS;
	LCD_CTRL = LCD_CS | LCD_WR;
	LCD_CTRL = LCD_CS;
	LCD_CTRL = LCD_CS | LCD_nRS;
	
	LCD_DATA_DIR &= ~LCD_OUT;
}

void LCD_CmdWrite(uint16_t val){
	LCD_DATA = val;
	
	LCD_DATA_DIR |= LCD_OUT;
	
	LCD_CTRL = LCD_CS | LCD_nRS;
	LCD_CTRL = LCD_CS | LCD_nRS | LCD_WR;
	LCD_CTRL = LCD_CS | LCD_nRS;
	
	LCD_DATA_DIR &= ~LCD_OUT;
}

void LCD_CtrlWrite(int reg, int val){
	LCD_DATA_DIR |= LCD_OUT;
	LCD_AddrWrite(reg);
	LCD_CmdWrite(val);
}

void LCD_Draw(){
	GFX_BLIT = 1;
}

void LCD_Init_0(){
  /* todo: generate this from the ida code */
  /* btw, this does magic:
      cat ida.disassembly.txt | awk '($2=="MOV" || $2=="MOVNE") {gsub("#","",$4); reg[$3]=$4} ($3=="sub_240D6260") { print "LCD_CtrlWrite( "reg["R0,"],",",reg["R1,"]" );" } ($3=="sub_240C8404") {print "delay_ms( ",reg["R0,"],");" }' > lcd_init_code_lcd0.generated
  */

  printf("Init code for LCD type 0 is highly experimental.\n");

  LCD_CtrlWrite( 0 , 1 );
  LCD_CtrlWrite( 1 , 0x100 );
  LCD_CtrlWrite( 2 , 0x400 );
  LCD_CtrlWrite( 3 , 0x1030 );
  LCD_CtrlWrite( 4 , 0 );
  LCD_CtrlWrite( 8 , 0x202 );
  LCD_CtrlWrite( 9 , 0 );
  LCD_CtrlWrite( 0xC , 1 );
  LCD_CtrlWrite( 0xD , 0 );
  LCD_CtrlWrite( 0xF , 0 );
  LCD_CtrlWrite( 0x10 , 0 );
  LCD_CtrlWrite( 0x11 , 7 );
  LCD_CtrlWrite( 0x12 , 0 );
  LCD_CtrlWrite( 0x13 , 0 );
  delay_ms(  0x32 );
  LCD_CtrlWrite( 0x10 , 0x17B0 );
  LCD_CtrlWrite( 0x11 , 1 );
  delay_ms(  0x32 );
  LCD_CtrlWrite( 0x12 , 0x13C );
  delay_ms(  0x32 );
  LCD_CtrlWrite( 0x13 , 0x1300 );
  LCD_CtrlWrite( 0x29 , 4 );
  delay_ms(  0x32 );
  LCD_CtrlWrite( 0x20 , 0 );
  LCD_CtrlWrite( 0x21 , 0 );
  LCD_CtrlWrite( 0x2B , 0x20 );
  LCD_CtrlWrite( 0x30 , 0 );
  LCD_CtrlWrite( 0x31 , 0x306 );
  LCD_CtrlWrite( 0x32 , 0x200 );
  LCD_CtrlWrite( 0x35 , 0x107 );
  LCD_CtrlWrite( 0x36 , 0x404 );
  LCD_CtrlWrite( 0x37 , 0x606 );
  LCD_CtrlWrite( 0x38 , 0x505 );
  LCD_CtrlWrite( 0x39 , 0x707 );
  LCD_CtrlWrite( 0x3C , 0x606 );
  LCD_CtrlWrite( 0x3D , 0x807 );
  LCD_CtrlWrite( 0x50 , 0 );
  LCD_CtrlWrite( 0x51 , 0xEF );
  LCD_CtrlWrite( 0x52 , 0 );
  LCD_CtrlWrite( 0x53 , 0x13F );
  LCD_CtrlWrite( 0x60 , 0x2700 );
  LCD_CtrlWrite( 0x61 , 1 );
  LCD_CtrlWrite( 0x6A , 0 );
  LCD_CtrlWrite( 0x80 , 0 );
  LCD_CtrlWrite( 0x81 , 0 );
  LCD_CtrlWrite( 0x82 , 0 );
  LCD_CtrlWrite( 0x83 , 0 );
  LCD_CtrlWrite( 0x84 , 0 );
  LCD_CtrlWrite( 0x85 , 0 );
  LCD_CtrlWrite( 0x90 , 0x13 );
  LCD_CtrlWrite( 0x92 , 0 );
  LCD_CtrlWrite( 0x93 , 3 );
  LCD_CtrlWrite( 0x95 , 0x110 );
  LCD_CtrlWrite( 0x97 , 0 );
  LCD_CtrlWrite( 0x98 , 0 );
  LCD_CtrlWrite( 7 , 1 );
  delay_ms(  0x32 );
  LCD_CtrlWrite( 7 , 0x21 );
  LCD_CtrlWrite( 7 , 0x23 );
  delay_ms(  0x32 );
  LCD_CtrlWrite( 7 , 0x170 );
}

void LCD_Init_3(){
	/* //--- Init sequence ---// */
	LCD_CtrlWrite(0xE5, 0x8000);
	
	LCD_CtrlWrite(0x00, 0x0001);
	LCD_CtrlWrite(0x01, 0x0100);
	LCD_CtrlWrite(0x02, 0x0700);
	LCD_CtrlWrite(0x03, 0x1030);
	LCD_CtrlWrite(0x04, 0x0000);
	LCD_CtrlWrite(0x08, 0x0202);
	LCD_CtrlWrite(0x09, 0x0000);
	LCD_CtrlWrite(0x0A, 0x0000);
	LCD_CtrlWrite(0x0C, 0x0000);
	LCD_CtrlWrite(0x0D, 0x0000);
	LCD_CtrlWrite(0x0F, 0x0000);
	
	/* //--- Power on sequence ---// */
	LCD_CtrlWrite(0x10, 0x0000);
	LCD_CtrlWrite(0x11, 0x0007);
	LCD_CtrlWrite(0x12, 0x0000);
	LCD_CtrlWrite(0x13, 0x0000);
	delay_ms(200);
	LCD_CtrlWrite(0x10, 0x17B0);
	LCD_CtrlWrite(0x11, 0x0137);
	delay_ms(50);
	LCD_CtrlWrite(0x12, 0x013C);
	delay_ms(50);
	LCD_CtrlWrite(0x13, 0x1800);
	LCD_CtrlWrite(0x29, 0x0016);
	delay_ms(50);
	LCD_CtrlWrite(0x20, 0x0000);
	LCD_CtrlWrite(0x21, 0x0000);
	
	/* //--- Adjust the Gamma Curve ---// */
	LCD_CtrlWrite(0x30, 0x0006);
	LCD_CtrlWrite(0x31, 0x0407);
	LCD_CtrlWrite(0x32, 0x0200);
	LCD_CtrlWrite(0x35, 0x0007);
	LCD_CtrlWrite(0x36, 0x0F07);
	LCD_CtrlWrite(0x37, 0x0506);
	LCD_CtrlWrite(0x38, 0x0203);
	LCD_CtrlWrite(0x39, 0x0607);
	LCD_CtrlWrite(0x3C, 0x0601);
	LCD_CtrlWrite(0x3D, 0x1F00);
	
	/* //--- Set GRAM Area ---// */
	LCD_CtrlWrite(0x50, 0x0000);
	LCD_CtrlWrite(0x51, 0x00EF);
	LCD_CtrlWrite(0x52, 0x0000);
	LCD_CtrlWrite(0x53, 0x013F);
	LCD_CtrlWrite(0x60, 0x2700);
	LCD_CtrlWrite(0x61, 0x0001);
	LCD_CtrlWrite(0x6A, 0x0000);
	
	/* //--- Partial display control ---// */
	LCD_CtrlWrite(0x80, 0x0000);
	LCD_CtrlWrite(0x81, 0x0000);
	LCD_CtrlWrite(0x82, 0x0000);
	LCD_CtrlWrite(0x83, 0x0000);
	LCD_CtrlWrite(0x84, 0x0000);
	LCD_CtrlWrite(0x85, 0x0000);
	
	/*  //--- Panel Control ---// */
	LCD_CtrlWrite(0x90, 0x0010);
	LCD_CtrlWrite(0x92, 0x0000);
	LCD_CtrlWrite(0x93, 0x0003);
	LCD_CtrlWrite(0x95, 0x0110);
	LCD_CtrlWrite(0x97, 0x0000);
	LCD_CtrlWrite(0x98, 0x0000);
	LCD_CtrlWrite(0x07, 0x0173);
	
	LCD_AddrWrite(0x22);
}

void LCD_Reset(){
	LCD_RESET_REG &= ~LCD_RESET;
	delay_us(10);
	LCD_RESET_REG |= LCD_RESET;
	delay_us(50);
}

/*
 *	Inits the LCD based on the lcd_type. This function is based on a LOT of magic register pokes.
 *	Anything that uses the lcd_base pointer is a magic poke, and we don't know what it does.
 */

void LCD_DoMagic();

void LCD_Init(int lcd_type){
	volatile uint8_t* lcd_base = (volatile uint8_t*)LCD_BASE;
	LCD_DATA_EXT = 0;
	
	/* Magic register pokes. Peripheral turn-on? */
	*((volatile uint32_t*)0x10000008) = 0xFFFFFFFF;
	*((volatile uint32_t*)0x10000110) = 0xFFFFFFFF;

	lcd_base[0] |= 0x1;
	LCD_GFX_ENABLE = 1;
	lcd_base[0x1B9] |= 0x80;
	
	LCD_Reset();
	
	switch(lcd_type){
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			LCD_Init_3();
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		default:
			return;
	}
	LCD_AddrWrite(0x22);
	
	lcd_base[0x1B2] &= ~0x1;
	lcd_base[0x1BA] |= 0x1;
	lcd_base[0x1B2] |= 0x1;
	
	/* Do the voodoo that makes hardware acceleration work. */
	LCD_DoMagic();
}

void LCD_SetFramebuffer(void* fb){
	GFX_FB_START = ((uint32_t)fb) >> 1;
	GFX_FB_END = (((uint32_t)fb) + LCD_WIDTH * LCD_HEIGHT * (LCD_BPP / 8)) >> 1;
	GFX_FB_HORIZ = LCD_WIDTH;
	GFX_FB_VERT = LCD_HEIGHT;
	LCD_GFX_ENABLE |= 2;	/* Possibly 'reinitialize framebuffer' */
}

void LCD_DoMagic(){
	uint16_t temp;
	volatile uint8_t* lcd_base = (volatile uint8_t*)LCD_BASE;
	/* LCD_init */
	lcd_base[0x242] = 0x5;
	lcd_base[0x203] &= ~0x1;
	lcd_base[0x204] = 0xD;
	lcd_base[0x205] = 1;
	lcd_base[0x194] |= 0x4;
	
	lcd_base[0x203] |= 0x1;
	
	LCD_SCREEN_HEIGHT = LCD_HEIGHT;
	LCD_SCREEN_WIDTH = LCD_WIDTH;
	LCD_SCREEN_UNK = 0x0505;	

	LCD_GFX_ENABLE = 1;
	
	/* init_more_gfx */
	
	lcd_base[0x100] = 0x4;
	
	lcd_base[0x1D1] = 0xA;
	lcd_base[0x226] &= ~0x1;
	
	lcd_base[0x1DB] = 0x00;
	lcd_base[0x1DC] = 0xFC;
	lcd_base[0x1DD] = 0x00;
	lcd_base[0x1DE] = 0xFF;
	lcd_base[0x1DF] = 0xFF;
	lcd_base[0x1E0] = 0xFF;
	
	/* Unaligned access for resize_screen */
	temp = LCD_WIDTH - 1;
	lcd_base[0x145] = temp & 0xFF;
	lcd_base[0x146] = (temp >> 8) & 0xFF;
	
	lcd_base[0x14D] = temp & 0xFF;
	lcd_base[0x14E] = (temp >> 8) & 0xFF;
	
	temp = LCD_HEIGHT - 1;
	lcd_base[0x147] = temp & 0xFF;
	lcd_base[0x148] = (temp >> 8) & 0xFF;

	lcd_base[0x14F] = temp & 0xFF;
	lcd_base[0x150] = (temp >> 8) & 0xFF;
}

void LCD_GenTestImage()
{
  int i;
  LCD_DATA_DIR |= LCD_OUT;
  
  LCD_AddrWrite(0x22);
  
  LCD_CTRL = LCD_CS | LCD_nRS;
  for(i = 0; i < 320 * 240; i++){
    LCD_DATA = i*i;
    LCD_CTRL = LCD_WR | LCD_CS | LCD_nRS;
    LCD_CTRL = LCD_CS | LCD_nRS;
  }
}

/* wrappers for the above */
static int lcd_init(void)
{
  int deviceType = 3;

  /* Create LCD device as an alias of the registered device. */
  lcd_dev = device_create(&lcd_io, "lcd", DF_CHR);
  if (lcd_dev == DEVICE_NULL)
    return -1;

  LCD_Init( deviceType );
  /* LCD_SetBacklight( 1 ); */

  /* LCD_GenTestImage(); Won't work with the new LCD code. */

  return 0;
}

static int lcd_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
  printf( "lcd_read called with size %d\n", *nbyte  );
  return *nbyte;
}

static int lcd_write(device_t dev, char *buf, size_t *nbyte, int blkno)
{
  printf( "lcd_write called with value %d\n", buf[0] );
  LCD_SetBacklight(  buf[0] );
  return *nbyte;
}

static int lcd_ioctl(device_t dev, u_long cmd, void *arg)
{
	switch(cmd){
		case LCDIOC_SET_FB:
			/* printf("Setting framebuffer to %08X.\n", *((void**)arg)); */
			LCD_SetFramebuffer(arg);
			return 0;
		case LCDIOC_SET_BACKLIGHT:
			/* printf("Setting backlight.\n"); */
			LCD_SetBacklight(((int)arg));
			return 0;
		case LCDIOC_DRAW:
			/* printf("Drawing framebuffer.\n"); */
			LCD_Draw();
			return 0;
	}
	printf("Whoops, wrong ioctl received!\n");
	return -1;
}

static int lcd_open(device_t dev, int mode)
{
  printf("lcd_open called\n");
  return 0;
}

static int lcd_close(device_t dev)
{
  printf("lcd_close called\n");
  return 0;
}

