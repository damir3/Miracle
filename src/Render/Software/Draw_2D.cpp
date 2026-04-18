/*
	Draw_2D.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdio.h>
#include	<Memory.h>
#include	<Stdlib.h>
#include	<Stdarg.h>
#include	<Math.h>

#include	"Math_3D.h"
#include	"Video.h"
#include	"Draw_2D.h"
#include	"MapFile.h"

extern ushort	transparency_map[16][0x10000];
extern ushort	r_color_map[64][256];
extern ushort	g_color_map[64][256];
extern ushort	b_color_map[32][256];
extern int		scr_face_edge[MAX_SY_SIZE][2];

bitmap	font, background, bumpmap, phongmap;
static int		sx, sy, width, height, pointer, row_dest, row_out;

ushort	Color(color_t c)
{
	if(bits==15)
		return	((c.r>>3)<<10) + ((c.g>>3)<<5) + (c.b>>3);
	else
		return	((c.r>>3)<<11) + ((c.g>>2)<<5) + (c.b>>3);
}

void	DrawBitmap(int x,int y, bitmap *bm, byte alpha, byte type)
{
	if(bm->bpp!=bits)	return;
	alpha >>= 4;
	if(!alpha)	return;
	sx = x;
	sy = y;
	width = bm->width;
	height = bm->height;

	row_out = width;
	if(sy<clip_rect.top)
	{
		pointer = (clip_rect.top - sy)*width;
		height += (sy - clip_rect.top);
		sy = clip_rect.top;
	} else	pointer = 0;
	if((sy+height-1) > clip_rect.bottom)
		height = clip_rect.bottom - sy + 1;
	if(sx<clip_rect.left)
	{
		pointer += (clip_rect.left - sx);
		width += (sx - clip_rect.left);
		sx = clip_rect.left;
	}
	if((sx+width-1) > clip_rect.right)
		width = clip_rect.right - sx + 1;
	row_out -= width;
	row_dest = sx_size - width;

	switch(type)
	{
	case	0:
		{
			if(bm->bpp!=bits)	return;
			ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy] + sx;
			ushort	*out = (ushort *)bm->bm + pointer;
			if(alpha<15)
			{
				ushort	*map1 = transparency_map[alpha];
				ushort	*map2 = transparency_map[15-alpha];
				while(height--)
				{
					int	i = width;
					while(i--)
						*dest++ = map1[*out++] + map2[*dest];
					out += row_out;
					dest += row_dest;
				}
			} else
			{
				while(height--)
				{
					int	i = width;
					while(i--)
						*dest++ = *out++;
					out += row_out;
					dest += row_dest;
				}
			}
		}
		break;
	case	1:
		{
			if(bm->bpp!=bits)	return;
			ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy] + sx;
			ushort	*out = (ushort *)bm->bm + pointer;
			ushort	c;
			if(alpha<15)
			{
				ushort	*map1 = transparency_map[alpha];
				ushort	*map2 = transparency_map[15-alpha];
				while(height--)
				{
					int	i = width;
					while(i--)
					{
						c = *out++;
						if(c)
							*dest++ = map1[c] + map2[*dest];
						else	dest++;
					}
					out += row_out;
					dest += row_dest;
				}
			} else
			{
				while(height--)
				{
					int	i = width;
					while(i--)
					{
						c = *out++;
						if(c)	*dest++ = c;
						else	dest++;
					}
					out += row_out;
					dest += row_dest;
				}
			}
		}
		break;
	case	2:
		{
			if(bm->bpp!=24)	return;
			ushort	*dest = (ushort *)virtual_screen + screen_row_table[sy] + sx;
			byte	*out = (byte *)bm->bm + pointer*3;
			row_out += row_out<<1;
			while(height--)
			{
				int	i = width;
				while(i--)
				{
					ushort	c = *dest;
					byte	r = c>>10;
					byte	g = (c>>5)&63;
					byte	b = c&31;
					*dest++ = r_color_map[r][out[0]]+
						g_color_map[g][out[1]]+
						b_color_map[b][out[2]];
					out += 3;
				}
				out += row_out;
				dest += row_dest;
			}
		}
		break;
	}
}

void	DrawString(int x, int y, color_t c, char *format, ...)
{
	byte	alpha = c.a>>4;
	if(!alpha)	return;
	char	tmpstr[1024];
	va_list	(args);
	va_start(args, format);
	vsprintf(tmpstr, format, args);
	va_end(args);
	byte	*str = (byte *)tmpstr;
	ushort	color = Color(c);
	if(alpha<15)
	{
		ushort	*map2 = transparency_map[15-alpha];
		ushort	c = transparency_map[alpha][color];
		while(*str)
		{
			if(*str!=' ')
			{
				ushort	*dest = (ushort *)virtual_screen + screen_row_table[y] + x;
				ushort	*out = (ushort *)font.bm + ((*str&15)<<3) + ((*str>>4)<<10);
				int		j=8;
				while(j--)
				{
					int	i=8;
					while(i--)
					{
						if(*out++)
							*dest++ = c + map2[*dest];
						else
							dest++;
					}
					out += 15<<3;
					dest += sx_size-8;
				}
			}
			x += 8;
			str++;
		}
	} else
	{
		while(*str)
		{
			if(*str!=' ')
			{
				ushort	*dest = (ushort *)virtual_screen + screen_row_table[y] + x;
				ushort	*out = (ushort *)font.bm + ((*str&15)<<3) + ((*str>>4)<<10);
				int		j=8;
				while(j--)
				{
					int	i=8;
					while(i--)
					{
						if(*out++)
							*dest++ = color;
						else
							dest++;
					}
					out += 15<<3;
					dest += sx_size-8;
				}
			}
			x += 8;
			str++;
		}
	}
}
void	DrawBackground(int time)
{
	int		x = int(background.width*(0.5+0.3*sin(time*pi/1000)));
	int		y = int(background.height*(0.5+0.3*cos(time*pi/3000)));
	int		i, j, xtemp, ytemp;
	byte	*src = (byte *)bumpmap.bm;
	byte	*dest = (byte *)background.bm;
	byte	*phong = (byte *)phongmap.bm;
	j=background.height;
	ytemp = -y;
	while(j-->0)
	{
		xtemp = -x;
		i = background.width;
		while(i-->0)
		{
			int		nX = *src++ + xtemp;
			int		nY = *src++ + ytemp;
			if (nX<0 || nX>255) nX=0;
			if (nY<0 || nY>255) nY=0;
			*dest++ = phong[nX+(nY<<8)];
			xtemp++;
		}
		ytemp++;
	}
	float	k, dk;
	dk = float(background.height)/sy_size;	k=0;
	for(i=0; i<sy_size; i++, k+=dk)
		scr_face_edge[i][1] = int(k)*background.width;
	dk = float(background.width)/sx_size;	k=0;
	for(i=0; i<sx_size; i++, k+=dk)
		scr_face_edge[i][0] = int(k);
	ushort	pal[256];
	int		r_shl, g_shr;
	if(bits==15)
	{
		r_shl = 10;
		g_shr = 3;
	} else
	{
		r_shl = 11;
		g_shr = 2;
	}
	#define	FACE_RED		160
	#define	FACE_GREEN		255
	#define	FACE_BLUE		0
	for (i=0; i<128; i++)
	{
		float	intensity = float(i)/127.0f;
		int	r = int(/*pow(intensity, 10)*100 + */intensity*FACE_RED);
		int	g = int(/*pow(intensity, 10)*100 + */intensity*FACE_GREEN);
		int	b = int(/*pow(intensity, 10)*100 + */intensity*FACE_BLUE);
		if(r>255)	r=255;
		if(g>255)	g=255;
		if(b>255)	b=255;
		pal[i] = ((r>>3)<<r_shl) + ((g>>g_shr)<<5) + (b>>3);
	}
	ushort	*vs = (ushort *)virtual_screen;
	for(j=0; j<sy_size; j++)
	{
		for(i=0; i<sx_size; i++)
			*vs++ = pal[background.bm[scr_face_edge[i][0] + scr_face_edge[j][1]]];
	}
}

void	SetFont(bitmap *bm)
{
	font = *bm;
}

void	SetBackground(bitmap *bm)
{
	int		size = bm->width*bm->height;
	background = *bm;
	bumpmap.bm = (char *)realloc(bumpmap.bm, size<<1);
	phongmap.bm = (char *)realloc(phongmap.bm, 0x10000);
	//memset(bumpmap.bm, 0, size<<1);
	memset(phongmap.bm, 0, 0x10000);
	for(int y=0; y<128; y++ )
	for(int x=0; x<128; x++ )
	{
		float	a=1 - x/127.0f, b=1 - y/127.0f;
		float	intensity = 1 - a*a - b*b;
		if(intensity >= 0)
		{
			int		temp = int(intensity*intensity*127);
			phongmap.bm[x + (y<<8)] = temp;
			phongmap.bm[x + ((255-y)<<8)] = temp;
			phongmap.bm[255 - x + (y<<8)] = temp;
			phongmap.bm[255 - x + ((255-y)<<8)] = temp;
		}
	}
	/*for (int y=0; y<256; y++)
	for (int x=0; x<256; x++)
	{
		float	nX=(x-128)/128.0f;
		float	nY=(y-128)/128.0f;
		float	nZ=float(sqrt(nX*nX + nY*nY));
		//if (nZ<0) nZ=0;
		if (nZ>1) nZ=1;
		phongmap.bm[x+(y<<8)]=char(127*cos(nZ*pi/2));
	}*/
	byte	*src = (byte *)background.bm;
	byte	*dest = (byte *)bumpmap.bm;
	for(int	i=0; i<background.width; i++)
	{
		*dest++ = 127 + ((*(src-1) - *(src+1))>>1);
		*dest++ = 127;
		src++;
	}
	size -= background.width<<1;
	while(size-->0)
	{
		*dest++ = 127 + ((*(src-1) - *(src+1))>>1);
		*dest++ = 127 + ((*(src-background.width) - *(src+background.width))>>1);
		src++;
	}
	for(i=0; i<background.width; i++)
	{
		*dest++ = 127 + ((*(src-1) - *(src+1))>>1);
		*dest++ = 127;
		src++;
	}
}
int		PixelCodes(int x, int y)
{
	int		codes = 0;
	if(x<clip_rect.left)	codes |= CC_OFF_LEFT;
	if(x>clip_rect.right)	codes |= CC_OFF_RIGHT;
	if(y<clip_rect.top)		codes |= CC_OFF_TOP;
	if(y>clip_rect.bottom)	codes |= CC_OFF_BOT;
	return	codes;
}
int		ClipLine(int *x1, int *y1, int *x2, int *y2)
{
	int		codes1 = PixelCodes(*x1, *y1);
	int		codes2 = PixelCodes(*x2, *y2);
	if(codes1&codes2)	return	0;
	if(!(codes1|codes2))	return	1;
	int		dx = *x2 - *x1;
	int		dy = *y2 - *y1;
	if(!dx && !dy)	return	0;
	if(codes1&CC_OFF_LEFT)
	{
		*y1 += (clip_rect.left - *x1)*dy/dx;
		*x1 = clip_rect.left;
		codes1=PixelCodes(*x1, *y1);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	} else
	if(codes2&CC_OFF_LEFT)
	{
		*y2 += (clip_rect.left - *x2)*dy/dx;
		*x2 = clip_rect.left;
		codes2=PixelCodes(*x2, *y2);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	}
	if(codes1&CC_OFF_RIGHT)
	{
		*y1 += (clip_rect.right - *x1)*dy/dx;
		*x1 = clip_rect.right;
		codes1=PixelCodes(*x1, *y1);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	} else
	if(codes2&CC_OFF_RIGHT)
	{
		*y2 += (clip_rect.right - *x2)*dy/dx;
		*x2 = clip_rect.right;
		codes2=PixelCodes(*x2, *y2);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	}
	if(codes1&CC_OFF_TOP)
	{
		*x1 += (clip_rect.top - *y1)*dx/dy;
		*y1 = clip_rect.top;
		codes1=PixelCodes(*x1, *y1);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	} else
	if(codes2&CC_OFF_TOP)
	{
		*x2 += (clip_rect.top - *y2)*dx/dy;
		*y2 = clip_rect.top;
		codes2=PixelCodes(*x2, *y2);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	}
	if(codes1&CC_OFF_BOT)
	{
		*x1 += (clip_rect.bottom - *y1)*dx/dy;
		*y1 = clip_rect.bottom;
		codes1=PixelCodes(*x1, *y1);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	} else
	if(codes2&CC_OFF_BOT)
	{
		*x2 += (clip_rect.bottom - *y2)*dx/dy;
		*y2 = clip_rect.bottom;
		codes2=PixelCodes(*x2, *y2);
		if(codes1&codes2)	return	0;
		if(!(codes1|codes2))	return	1;
	}
	return	0;
}
void	DrawLine(int x1, int y1, int x2, int y2, color_t c)
{
	ushort	color, *dest;
	byte	alpha = c.a>>4;
	if(!alpha)	return;
	dest = (ushort *) virtual_screen;
	color = Color(c);
	if(!ClipLine(&x1,&y1,&x2,&y2))	return;
	int		sx, dx = x2 - x1;
	int		sy, dy = y2 - y1;
	if(dx<0)
	{
		dx = -dx;
		sx = -1;
	} else	sx = 1;
	if(dy<0)
	{
		dy = -dy;
		sy = -1;
	} else	sy = 1;
	if(dy<=dx)
	{
		int		d1 = dy<<1;
		int		d = d1 - dx;
		int		d2 = d - dx;
		dest[screen_row_table[y1] + x1] = color;
		for(x1+=sx; dx-->0; x1+=sx)
		{
			if(d>0)
			{
				d += d2;
				y1 += sy;
			} else
				d += d1;
			dest[screen_row_table[y1] + x1] = color;
		}
	} else
	{
		int		d1 = dx<<1;
		int		d = d1 - dy;
		int		d2 = d - dy;
		dest[screen_row_table[y1] + x1] = color;
		for(y1+=sy; dy-->0; y1+=sy)
		{
			if(d>0)
			{
				d += d2;
				x1 += sx;
			} else
				d += d1;
			dest[screen_row_table[y1] + x1] = color;
		}
	}
}
void	Spline_Calc(scrpoint_t *p, float t, float d, float *sx, float *sy)
{
	float	t2, t3, b_0, b_1, b_2, b_3;
	t2 = t*t;
	t3 = t2*t;
	b_0 = 1 - 3*t2 + 2*t3;
	b_1 = 3*t2 - 2*t3;
	b_2 = (t - 2*t2 + t3)*d;
	b_3 = (-t2 + t3)*d;
	*sx = (p[1].sx*b_0) + (p[2].sx*b_1) + ((p[2].sx-p[0].sx)*b_2) + ((p[3].sx-p[1].sx)*b_3);
	*sy = (p[1].sy*b_0) + (p[2].sy*b_1) + ((p[2].sy-p[0].sy)*b_2) + ((p[3].sy-p[1].sy)*b_3);
}
void	CatmullRomSpline_Calc(scrpoint_t *p, float t, float *sx, float *sy)
{
	float	t2, t3, b_0, b_1, b_2, b_3;
	t2 = t*t;
	t3 = t2*t;
	b_0 = -t + 2*t2 - t3;
	b_1 = 2 - 5*t2 + 3*t3;
	b_2 = t + 4*t2 - 3*t3;
	b_3 = -t2 + t3;
	*sx = ((p[0].sx*b_0) + (p[1].sx*b_1) + (p[2].sx*b_2) + (p[3].sx*b_3))*0.5f;
	*sy = ((p[0].sy*b_0) + (p[1].sy*b_1) + (p[2].sy*b_2) + (p[3].sy*b_3))*0.5f;
}
void	DrawCatmullRomSpline(int n, scrpoint_t *p, int resolution, color_t color)
{
	float	sx, sy, psx, psy, r1;
	r1 = 1.0f/resolution;
	for(int i=1; i<n; i++)
	{
		CatmullRomSpline_Calc(&p[i-1], 0, &psx, &psy);
		for(int j=0; j<resolution; j++)
		{
			//color.r = int(255*(j+1)*r1);
			CatmullRomSpline_Calc(&p[i-1], (j+1)*r1, &sx, &sy);
			//if((int(psx)>0) && (int(psx)<sx_size) && (int(psy)>0) && (int(psy)<sy_size) && (int(sx)>0) && (int(sx)<sx_size) && (int(sy)>0) && (int(sy)<sy_size))
			DrawLine(int(psx), int(psy), int(sx), int(sy), color);
			psx = sx;
			psy = sy;
		}
	}
}
void	DrawSpline(int n, scrpoint_t *p, int resolution, color_t color, float k)
{
	float	sx, sy, psx, psy, r1;
	r1 = 1.0f/resolution;
	for(int i=1; i<n; i++)
	{
		Spline_Calc(&p[i-1], 0, k, &psx, &psy);
		for(int j=0; j<resolution; j++)
		{
			Spline_Calc(&p[i-1], (j+1)*r1, k, &sx, &sy);
			DrawLine(int(psx), int(psy), int(sx), int(sy), color);
			psx = sx;
			psy = sy;
		}
	}
}
void	SetPixel(int x, int y, color_t c)
{
	ushort	color, *dest;
	byte	alpha = c.a>>4;
	if(!alpha)	return;
	if(PixelCodes(x, y))	return;
	dest = (ushort *)virtual_screen + screen_row_table[y] + x;
	color = Color(c);
	if(alpha<15)
		*dest = transparency_map[alpha][color] + transparency_map[15-alpha][*dest];
	else
		*dest = color;
}
color_t	GetPixel(int x, int y)
{
	color_t	color;
	ushort	c = *((ushort *)virtual_screen + screen_row_table[y] + x);
	if(bits==15)
	{
		color.r = (c>>10)<<3;
		color.g = (c>>5)<<3;
	} else
	{
		color.r = (c>>11)<<3;
		color.g = (c>>5)<<2;
	}
	color.b = c<<3;
	color.a = 0xFF;
	return	color;
}
void	SetFullClipRectangle()
{
	clip_rect.left = 0;
	clip_rect.top = 0;
	clip_rect.right = sx_size-1;
	clip_rect.bottom = sy_size-1;
}
void	SetClipRectangle(rect_t rect)
{
	clip_rect.left = ClipScreenX(rect.left);
	clip_rect.top = ClipScreenY(rect.top);
	clip_rect.right = ClipScreenX(rect.right);
	clip_rect.bottom = ClipScreenY(rect.bottom);
}
void	GetClipRectangle(rect_t *rect)
{
	*rect = clip_rect;
}
void	DrawTHLine(int y, int x, int dx, ushort color, byte alpha)
{
	ushort	*dest = (ushort *)virtual_screen + screen_row_table[y] + x;
	ushort	*map2 = transparency_map[15-alpha];
	while((dx--)>0)	*dest++ = color + map2[*dest];
}
void	DrawHLine(int y, int x, int dx, ushort color)
{
	ushort	*dest = (ushort *)virtual_screen + screen_row_table[y] + x;
	while((dx--)>0)	*dest++ = color;
}
void	DrawTVLine(int y, int x, int dy, ushort color, byte alpha)
{
	ushort	*dest = (ushort *)virtual_screen + screen_row_table[y] + x;
	ushort	*map2 = transparency_map[15-alpha];
	while((dy--)>0)
	{
		*dest = color + map2[*dest];
		dest += sx_size;
	}
}
void	DrawVLine(int y, int x, int dy, ushort color)
{
	ushort	*dest = (ushort *)virtual_screen + screen_row_table[y] + x;
	while((dy--)>0)
	{
		*dest = color;
		dest += sx_size;
	}
}
void	DrawBar(int x1, int y1, int x2, int y2, color_t c)
{
	ushort	color;
	int		temp, dx;
	int		ccode, ccode1, ccode2;
	byte	alpha = c.a>>4;
	if(!alpha)	return;

	if(x2<x1)
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if(y2<y1)
	{
		temp = y2;
		y2 = y1;
		y1 = temp;
	}
	ccode1 = PixelCodes(x1, y1);
	ccode2 = PixelCodes(x2, y2);
	if(ccode1&ccode2)	return;
	ccode = ccode1|ccode2;
	if(ccode&CC_OFF_LEFT)	x1 = clip_rect.left;
	if(ccode&CC_OFF_TOP)	y1 = clip_rect.top;
	if(ccode&CC_OFF_RIGHT)	x2 = clip_rect.right;
	if(ccode&CC_OFF_BOT)	y2 = clip_rect.bottom;
	dx = x2 - x1 + 1;
	if(alpha<15)
	{
		color = transparency_map[alpha][Color(c)];
		for(; y1<y2; y1++)
			DrawTHLine(y1, x1, dx, color, alpha);
	} else
	{
		color = Color(c);
		for(; y1<y2; y1++)
			DrawHLine(y1, x1, dx, color);
	}
}
void	DrawRectangle(int x1, int y1, int x2, int y2, color_t c)
{
	ushort	color;
	int		temp, dx, dy;
	int		ccode, ccode1, ccode2;
	byte	alpha = c.a>>4;
	if(!alpha)	return;
	if(x2<x1)
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if(y2<y1)
	{
		temp = y2;
		y2 = y1;
		y1 = temp;
	}
	ccode1 = PixelCodes(x1, y1);
	ccode2 = PixelCodes(x2, y2);
	if(ccode1&ccode2)	return;
	ccode = ccode1|ccode2;
	if(ccode&CC_OFF_LEFT)	x1 = clip_rect.left;
	if(ccode&CC_OFF_TOP)	y1 = clip_rect.top;
	if(ccode&CC_OFF_RIGHT)	x2 = clip_rect.right;
	if(ccode&CC_OFF_BOT)	y2 = clip_rect.bottom;
	dx = x2 - x1 + 1;
	dy = y2 - y1 - 1;
	if(alpha<15)
	{
		color = transparency_map[alpha][Color(c)];
		if(!(ccode&CC_OFF_LEFT))
			DrawTVLine(y1 + 1, x1, dy, color, alpha);
		if(!(ccode&CC_OFF_RIGHT))
			DrawTVLine(y1 + 1, x2, dy, color, alpha);
		if(!(ccode&CC_OFF_TOP))
			DrawTHLine(y1, x1, dx, color, alpha);
		if(!(ccode&CC_OFF_BOT))
			DrawTHLine(y2, x1, dx, color, alpha);
	} else
	{
		color = Color(c);
		if(!(ccode&CC_OFF_LEFT))
			DrawVLine(y1 + 1, x1, dy, color);
		if(!(ccode&CC_OFF_RIGHT))
			DrawVLine(y1 + 1, x2, dy, color);
		if(!(ccode&CC_OFF_TOP))
			DrawHLine(y1, x1, dx, color);
		if(!(ccode&CC_OFF_BOT))
			DrawHLine(y2, x1, dx, color);
	}
}
/*int		i, j;
	byte	*dest = (byte *)bumpbackground.bm + 1 + background.width;
	byte	*out = (byte *)background.bm + 1 + background.width;
	int		ly = y;
	for(j=background.height - 2; j>0; j--)
	{
		int		lx = x;
		for(i=background.width - 2; i>0; i--, lx--, out++)
		{
			int	nx = *(out+1) - *(out-1);
			int	coul1 = 127 - abs(nx + lx);
			if(coul1<0)	coul1=0;
			int	ny = *(out+background.width) - *(out-background.width);
			int	coul2 = 127 - abs(ny + ly);
			if(coul2<0)	coul2=0;
			*dest++ = (coul1*coul2) >> 9;
		}
		ly--;
		dest += 2;
		out += 2;
	}*/