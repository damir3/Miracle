#ifndef _READPCX_H
#define _READPCX_H

#include	"struct.h"
#include	"unpak.h"

typedef struct
{
	char	manufacturer;
	char	version;
	char	encoding;
	char	bits_per_pixel;
	short	xmin;
	short	ymin;
	short	xmax;
	short	ymax;
	short	hres;
	short	vres;
	char	palette[48];
	char	reserved;
	char	color_planes;
	short	bytes_per_line;
	short	palette_type;
	char	filler[58];
	char	data;
} pcx_header;

int		LoadPCX8(bitmap8 *bm, char *name, CPakFile *pf);
int		LoadPCX16(bitmap *bm, char *name, CPakFile *pf);
int		LoadPCX24(bitmap *bm, char *name, CPakFile *pf);
void	ResizeToBitmap16(bitmap *bm_dest, bitmap8 *bm_src, int new_width, int new_height);
void	ResizeToBitmap24(bitmap *bm_dest, bitmap8 *bm_src, int new_width, int new_height);
void	ConvertToGrayscale(bitmap *bm_dest, bitmap8 *bm_src);

#endif