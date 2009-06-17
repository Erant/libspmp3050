// #include <lzo/lzo1x.h>


#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#define HOSTHACK
#include "../boot/common/minilzo.h"
#include "../boot/common/minilzo.c"


int main( int argc, char * argv[])
{
  lzo_init();
  if (argc != 3)
    {
      printf("usage: %s <infile> <outfile>\n", argv[0] );
      return 1;
    }
  FILE * ifp = fopen( argv[1] , "rb" );
  fseek( ifp, 0, SEEK_END );
  int ifpsize = ftell( ifp );
  fseek(ifp,0,SEEK_SET);

  unsigned char * src;
  unsigned char * dst;
  unsigned char wrkmem[LZO1X_MEM_COMPRESS];

  lzo_uint src_len = ifpsize;
  lzo_uint dst_len = src_len;

  src = malloc( src_len );
  dst = malloc( dst_len );
  int br = fread( src, 1,src_len, ifp );
  printf("br=%d\n", br );
  lzo1x_1_compress( src, src_len,
                      dst,  &dst_len,
                      (void*) wrkmem );
  printf("dl=%d\n", (int)dst_len );
  printf("compressed to %2.2f%%\n", 100.0*dst_len/src_len );
  FILE * ofp = fopen( argv[2], "wb" );
  fputc( (dst_len>>0) & 0xff, ofp );
  fputc( (dst_len>>8) & 0xff, ofp );
  fputc( (dst_len>>16) & 0xff, ofp );
  fputc( (dst_len>>24) & 0xff, ofp );
  fwrite( dst, 1, dst_len, ofp );
  printf("done\n");
  return 0;

}
