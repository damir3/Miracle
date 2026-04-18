/*
	DrawSpan.cpp	Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdlib.h>
#include	<Math.h>

#include	"Const.h"
#include	"DrawSpan.h"
#include	"MapFile.h"
#include	"Video.h"

typedef void	( *LPMF)(int n, short *dest, int u, int v, int du, int dv);
LPMF	DrawTexturedLine;

short	*current_texture;
short	*cloud_texture;
short	face_color;
int		tex_width, tex_height;
int		tex_row_table[258];
int		face_type, time;
int		waves_table[256];
int		bluryflag, blurxflag;

#define	fix_byte(x)	(((x) >> 16) & 0xff)

float	z_gradient[3];
float	tex_gradient[9];

extern int		minmiplevel, maxmiplevel;
extern ushort	r_color_map[64][256];
extern ushort	g_color_map[64][256];
extern ushort	b_color_map[32][256];

void	DrawTexturedSkyLine(int n, short *dest, int u, int v, int du, int dv)
{
	int	k = time>>6;
	while (n--)
	{
		int iv = fix_int(v)&tex_height;
		int iu = fix_int(u)&tex_width;
		int iu1 = (fix_int(u)+k)&tex_width;
		int	pointer = tex_row_table[iv+1];
		*dest++ =	current_texture[pointer + iu]+cloud_texture[pointer + iu1];
		u += du;
		v += dv;
	}
}

void	DrawAffineTexturedLine(int n, short *dest, int u, int v, int du, int dv)
{
	int		iu, iv;
	v += 0x10000;
	while(n--)
	{
		iu = fix_int(u);
		iv = fix_int(v);
		*dest++ = current_texture[tex_row_table[iv] + iu];
		u += du;
		v += dv;
	}
}
void	DrawAffineBluredTexturedLine(int n, short *dest, int u, int v, int du, int dv)
{
	int		iu, iv, n_2;
	if(bluryflag)	v += 0x10000;
	v += 0x10000;
	n_2 = n>>1;
	if(blurxflag)
	{
		while((n_2--)>0)
		{
			iv = fix_int(v);
			iu = fix_int(u)+1;
			if (iu>tex_width)	iu = tex_width;
			*dest++ = current_texture[tex_row_table[iv] + iu];
			u += du;
			v += dv;
			iv = fix_int(v);
			iu = fix_int(u);
			*dest++ = current_texture[tex_row_table[iv] + iu];
			u += du;
			v += dv;
		}
		if(n&1)
		{
			iv = fix_int(v);
			iu = fix_int(u)+1;
			if (iu>tex_width)	iu = tex_width;
			*dest++ = current_texture[tex_row_table[iv] + iu];
			u += du;
			v += dv;
		}
	} else
	{
		while((n_2--)>0)
		{
			iv = fix_int(v);
			iu = fix_int(u);
			*dest++ = current_texture[tex_row_table[iv] + iu];
			u += du;
			v += dv;
			iv = fix_int(v);
			iu = fix_int(u)+1;
			if (iu>tex_width)	iu = tex_width;
			*dest++ = current_texture[tex_row_table[iv] + iu];
			u += du;
			v += dv;
		}
		if(n&1)
		{
			iv = fix_int(v);
			iu = fix_int(u);
			*dest++ = current_texture[tex_row_table[iv] + iu];
			u += du;
			v += dv;
		}
	}
}
void	DrawBluredTexturedTransparentLine(int n, short *dest, int u, int v, int du, int dv)
{
	if(bits==15)
	{
		int		iu, iv, n_2;
		if(bluryflag)	v += 0x10000;
		v += 0x10000;
		n_2 = n>>1;
		if(blurxflag)
		{
			while((n_2--)>0)
			{
				iv = fix_int(v);
				iu = fix_int(u)+1;
				if (iu>tex_width)	iu = tex_width;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
				iv = fix_int(v);
				iu = fix_int(u);
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iv = fix_int(v);
				iu = fix_int(u)+1;
				if (iu>tex_width)	iu = tex_width;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
		} else
		{
			while((n_2--)>0)
			{
				iv = fix_int(v);
				iu = fix_int(u);
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
				iv = fix_int(v);
				iu = fix_int(u)+1;
				if (iu>tex_width)	iu = tex_width;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iv = fix_int(v);
				iu = fix_int(u);
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
		}
	} else
	{
		int		iu, iv, n_2;
		if(bluryflag)	v += 0x10000;
		v += 0x10000;
		n_2 = n>>1;
		if(blurxflag)
		{
			while((n_2--)>0)
			{
				iv = fix_int(v);
				iu = fix_int(u)+1;
				if (iu>tex_width)	iu = tex_width;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
				iv = fix_int(v);
				iu = fix_int(u);
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iv = fix_int(v);
				iu = fix_int(u)+1;
				if (iu>tex_width)	iu = tex_width;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
		} else
		{
			while((n_2--)>0)
			{
				iv = fix_int(v);
				iu = fix_int(u);
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
				iv = fix_int(v);
				iu = fix_int(u)+1;
				if (iu>tex_width)	iu = tex_width;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iv = fix_int(v);
				iu = fix_int(u);
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
		}
	}
}
void	DrawTexturedTransparentLine(int n, short *dest, int u, int v, int du, int dv)
{
	if(bits==15)
	{
		while (n--)
		{
			int iu = fix_int(u);
			int iv = fix_int(v);
			*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
			u += du;
			v += dv;
		}
	} else
	{
		while (n--)
		{
			int iu = fix_int(u);
			int iv = fix_int(v);
			*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
			u += du;
			v += dv;
		}
	}
}
void	DrawTexturedLightLine(int n, short *dest, int u, int v, int du, int dv)
{
	while (n--)
	{
		ushort	c = *dest;
		byte	r = c>>10;
		byte	g = (c>>5)&63;
		byte	b = c&31;
		int iu = fix_int(u);
		int iv = fix_int(v);
		int	count = tex_row_table[iv] + iu;
		count += count<<1;
		*dest++ =	r_color_map[r][current_texture[count]]+
					g_color_map[g][current_texture[count+1]]+
					b_color_map[b][current_texture[count+2]];
		u += du;
		v += dv;
	}
}
void	DrawBluredTexturedWaterLine(int n, short *dest, int u, int v, int du, int dv)
{
	if(bits==15)
	{
		int		iu, iv, n_2;
		if(bluryflag)	v += 0x10000;
		v += 0x10000;
		n_2 = n>>1;
		if(blurxflag)
		{
			while((n_2--)>0)
			{
				iu = (fix_int(u+waves_table[fix_byte(v)])+1)&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
				iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iu = (fix_int(u+waves_table[fix_byte(v)])+1)&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
		} else
		{
			while((n_2--)>0)
			{
				iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
				iu = (fix_int(u+waves_table[fix_byte(v)])+1)&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0x7BDF)+(*dest&0x7BDF))>>1;
				u += du;
				v += dv;
			}
		}
	} else
	{
		int		iu, iv, n_2;
		if(bluryflag)	v += 0x10000;
		v += 0x10000;
		n_2 = n>>1;
		if(blurxflag)
		{
			while((n_2--)>0)
			{
				iu = (fix_int(u+waves_table[fix_byte(v)])+1)&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
				iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iu = (fix_int(u+waves_table[fix_byte(v)])+1)&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
		} else
		{
			while((n_2--)>0)
			{
				iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
				iu = (fix_int(u+waves_table[fix_byte(v)])+1)&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
			if(n&1)
			{
				iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
				iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
				*dest++ = ((current_texture[tex_row_table[iv] + iu]&0xF7DF)+(*dest&0xF7DF))>>1;
				u += du;
				v += dv;
			}
		}
	}
}
void	DrawTexturedWaterLine(int n, short *dest, int u, int v, int du, int dv)
{
	if(bits==15)
		while (n--)
		{
			int iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
			int iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
			*dest++ = (current_texture[tex_row_table[iv]+iu]+(*dest&0x7BDF))>>1;
			u += du;
			v += dv;
		}
	else
		while (n--)
		{
			int iu = fix_int(u+waves_table[fix_byte(v)])&tex_width;
			int iv = fix_int(v+waves_table[fix_byte(u)])&tex_height;
			*dest++ = (current_texture[tex_row_table[iv]+iu]+(*dest&0xF7DF))>>1;
			u += du;
			v += dv;
		}
}

#define	SUBDIV_SHIFT  4
#define SUBDIV        (1 << SUBDIV_SHIFT)

void	DrawSpan(int sy, int sx_start, int sx_end)
{
	if(sx_start>=sx_end)	return;
	float	u0,v0,w0, u1,v1,w1, z;
	int		length, current_sx_end, last = 0;
	int		u, v, du, dv;
	float	sy_u = tex_gradient[0] + (sy * tex_gradient[2]);
	float	sy_v = tex_gradient[3] + (sy * tex_gradient[5]);
	float	sy_w = tex_gradient[6] + (sy * tex_gradient[8]);
	u0 = sy_u + (sx_start * tex_gradient[1]);
	v0 = sy_v + (sx_start * tex_gradient[4]);
	w0 = sy_w + (sx_start * tex_gradient[7]);

	bluryflag = (sy&1);
	blurxflag = (sx_start&1);

	if(!(draw_mode&(DRAW_TEXTURED+DRAW_LIGHT)))
	{
		int	n = sx_end - sx_start;
		short	*dest = (short *) virtual_screen + screen_row_table[sy] + sx_start;
		if((face_type==2) || (face_type==4))
		{
			if(bits==15)
			{
				face_color &= 0x7BDF;
				while (n--)
					*dest++ = (face_color+(*dest&0x7BDF))>>1;
			} else
			{
				face_color &= 0xF7DF;
				while (n--)
					*dest++ = (face_color+(*dest&0xF7DF))>>1;
			}
		} else
			while(n--)	*dest++ = face_color;
		return;
	}

	z  = 1 / w0;
	u0 = u0 * z;
	v0 = v0 * z;

	while((length = sx_end - sx_start)>0)
	{
		if(length>SUBDIV) length = SUBDIV;
		else last = 1;

		u = float_to_fix(u0);
		v = float_to_fix(v0);

		if(length == 1)
		{
			DrawTexturedLine(length,(short *) virtual_screen + screen_row_table[sy] + sx_start, u, v, 0, 0);
			return;
		}

		current_sx_end = sx_start + length - last;

		u1 = sy_u + (current_sx_end * tex_gradient[1]);
		v1 = sy_v + (current_sx_end * tex_gradient[4]);
		w1 = sy_w + (current_sx_end * tex_gradient[7]);

		z = 1 / w1;
		u1 = u1 * z;
		v1 = v1 * z;

		if(length == SUBDIV)
		{
			du = (float_to_fix(u1) - u) >> SUBDIV_SHIFT;
			dv = (float_to_fix(v1) - v) >> SUBDIV_SHIFT;
            DrawTexturedLine(SUBDIV,(short *) virtual_screen + screen_row_table[sy] + sx_start, u, v, du, dv);
            sx_start += SUBDIV;
            u0 = u1;
            v0 = v1;
		}
		else
		{
			float	temp = float(1.0/(length-last));
			du = float_to_fix((u1-u0)*temp);
			dv = float_to_fix((v1-v0)*temp);
            DrawTexturedLine(length,(short *) virtual_screen + screen_row_table[sy] + sx_start, u, v, du, dv);
            return;
		}
	}
}

void	SetTexture(short *bm, int bm_width, int bm_height)
{
	int		i, row;
	if(tex_width != bm_width-1 || tex_height != bm_height-1)
	{
		tex_width = bm_width-1;
		tex_height = bm_height-1;

        tex_row_table[0] = 0;
		for(i=1, row=0; i<=bm_height; i++, row+=bm_width)
			tex_row_table[i] = row;
		tex_row_table[i] = tex_row_table[bm_height];
	}
	current_texture = bm;
	if(face_type==3)
		cloud_texture = bm + bm_height*bm_width;
}

void	SetSurfaceType(int type)
{
	switch(type)
	{
	case	1:
		if(!minmiplevel)
			DrawTexturedLine = DrawAffineBluredTexturedLine;
		else
			DrawTexturedLine = DrawAffineTexturedLine;
		break;
	case	2:
		if(!minmiplevel)
			DrawTexturedLine = DrawBluredTexturedWaterLine;
		else
			DrawTexturedLine = DrawTexturedWaterLine;
		break;
	case	3:
		DrawTexturedLine = DrawTexturedSkyLine;
		break;
	case	4:
		if(!minmiplevel)
			DrawTexturedLine = DrawBluredTexturedTransparentLine;
		else
			DrawTexturedLine = DrawTexturedTransparentLine;
		break;
	case	5:
		DrawTexturedLine = DrawTexturedLightLine;
		break;
	}
}

void	UpdateWaves()
{//	обновляем волны(для данного времени)
	double	temp = time*3.1415/(40*25);
	double	add_to_temp = 3.1415/32;
	for (int i=0; i < 256; i++)
	{
		waves_table[i] = int(sin(temp) * 0x20000);
		temp += add_to_temp;
	}
}
