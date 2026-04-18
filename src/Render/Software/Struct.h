#ifndef _STRUCT_H
#define _STRUCT_H

typedef unsigned char	byte;
typedef unsigned short	ushort;
typedef unsigned int	uint;

typedef struct
{
	int		sx, sy;
} scrpoint_t;

typedef struct
{
	int		width, height;
	byte	pal[768];
	byte	*bm;
} bitmap8;

typedef struct
{
	byte	r, g, b, bpp;
	int		width, height;
	char	*bm;
} bitmap;

typedef struct
{
	int		width;
	int		height;
	int		bpp;
} vm;

typedef struct
{
	short	left;
	short	top;
	short	right;
	short	bottom;
} rect_t;

typedef struct
{
	byte	a;
	byte	b, g, r;
} color_t;

typedef struct
{
	void	(*CPrintf) (char *format, ...);
	void	(*CPrintfMessage) (char *format, ...);
} GameImport;

extern GameImport	gi;

#endif