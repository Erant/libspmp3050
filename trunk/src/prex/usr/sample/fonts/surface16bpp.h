#ifndef __surface_16bpp__
#define __surface_16bpp__

enum
{
	SPMP_SURFACE16BPP_INIT_SUCCESS=0,
	SPMP_SURFACE16BPP_INIT_FAIL=-1
};

int spmp_surface16bpp_init(void);
int spmp_surface16bpp_close(void);

spmp_surface * spmp_surface16bpp_create(void);
void spmp_surface16bpp_destroy(spmp_surface * surface);

#endif
