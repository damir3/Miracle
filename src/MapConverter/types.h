#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#define	TRUE	1
#define	FALSE	0

typedef unsigned char	byte;
typedef unsigned short	ushort;
typedef unsigned int	uint;

typedef float	vector[3];

typedef struct
{
	byte	r, g, b, bpp;
    int     texturenum;
	int		width, height;
	char	*bm;
} bitmap;

typedef struct
{
	short	left;
	short	top;
	short	right;
	short	bottom;
} rect_t;

typedef struct
{
	float	fraction;
	vector	start;
	vector	end;
	int		contents;
	vector	normal;
	float	dist;
} trace_t;

float	StrToFloat (char *str);
int		StrToInt (char *str);
int		StrGetArgs (char *str, char	*args[32]);

#ifdef __cplusplus
}
#endif

#endif