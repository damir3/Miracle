/*
	ReadPCX.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdlib.h>
#include	<stdio.h>

#include	"unpak.h"
#include	"console.h"
#include	"pcxfile.h"
#include	"video.h"

static char	*filedata;

int		LoadPCX8(bitmap8 *bm, char *name, CPakFile *pf)
{
	int		size = pf->ExtractByName(name, &filedata);
	if(size<=0)	return	-1;
	pcx_header	*header = (pcx_header *)filedata;
	if(header->bits_per_pixel!=8)
	{
		//CPrintf("\"%s\" is not 8 bits picture",name);
		free(filedata);
		return	-1;
	}
	int		width = bm->width = header->xmax+1;
	int		height = bm->height = header->ymax+1;
	int		bmsize = height*width;

	memcpy(bm->pal, (char *)filedata + size - 768, 768);
	byte	*dest = (byte *)malloc(bmsize);
	byte	*out = (byte *)filedata + 128;
	bm->bm = dest;

	size -= 768+128;
	int		i = bmsize;
	int		c;
	while((size-->0) && (i>0))
	{
		c = *out++;
		if(c<=192)
		{
			*dest++ = c;
			i--;
		}
		else
		{
			int	n = c-192;
			c = *out++;
			size--;
			i -= n;		if(i<0)	break;
			while(n--)	*dest++ = c;
		}
	}
	/*if(size || i);
		CPrintf("Pcx file \"%s\" contain errors.",name);*/
	free(filedata);
	return	bmsize;
}

int		LoadPCX16(bitmap *bm, char *name, CPakFile *pf)
{
	bitmap8	temp_bm;
	int		bm_size = LoadPCX8(&temp_bm, name, pf);
	if(bm_size<=0)	return	-1;
	bm->bpp = bits;
	bm->width = temp_bm.width;
	bm->height = temp_bm.height;
	ushort	*dest = (ushort *)malloc(bm_size*2);
	byte	*out = temp_bm.bm;
	bm->bm = (char *)dest;
	int		r_shl, g_shr;
	if(bits==15)
	{
		r_shl = 10;
		g_shr  = 3;
	} else
	{
		r_shl = 11;
		g_shr = 2;
	}
	int		r=0, g=0, b=0;
	int		i = bm_size;
	while(i--)
	{
		int		col = *out++;
		int		c = col*3;
		*dest++ =	((temp_bm.pal[c]>>3)<<r_shl) +
					((temp_bm.pal[c+1]>>g_shr)<<5) +
					(temp_bm.pal[c+2]>>3);
		r += temp_bm.pal[c];
		g += temp_bm.pal[c+1];
		b += temp_bm.pal[c+2];
	}
	free(temp_bm.bm);
	bm->r = r/bm_size;
	bm->g = g/bm_size;
	bm->b = b/bm_size;
	return	bm_size;
}

int		LoadPCX24(bitmap *bm, char *name, CPakFile *pf)
{
	bitmap8	temp_bm;
	int		bm_size = LoadPCX8(&temp_bm, name, pf);
	if(bm_size<=0)	return	-1;
	bm->bpp = 24;
	bm->width = temp_bm.width;
	bm->height = temp_bm.height;
	byte	*dest = (byte *)malloc(bm_size*3);
	byte	*out = temp_bm.bm;
	bm->bm = (char *)dest;
	int		r=0, g=0, b=0;
	int		i = bm_size;
	while(i--)
	{
		int		c = *out++;
		int		col = (c<<1) + c;
		*dest++ = temp_bm.pal[col];
		*dest++ = temp_bm.pal[col+1];
		*dest++ = temp_bm.pal[col+2];
		r += temp_bm.pal[col];
		g += temp_bm.pal[col+1];
		b += temp_bm.pal[col+2];
	}
	free(temp_bm.bm);
	bm->r = r/bm_size;
	bm->g = g/bm_size;
	bm->b = b/bm_size;
	return	bm_size;
}
void	ResizeToBitmap16(bitmap *bm_dest, bitmap8 *bm_src, int new_width, int new_height)
{
	if(!new_width || !new_height)	return;
	bm_dest->bm = (char *)malloc(new_width*new_height*2);
	ushort	*dest = (ushort *)bm_dest->bm;
	byte	*src = (byte *)bm_src->bm;
	byte	*pal = bm_src->pal;
	bm_dest->bpp = bits;
	bm_dest->width = new_width;
	bm_dest->height = new_height;
	int		x, y, r_shl, g_shr;
	if(bits==15)
	{
		r_shl = 10;
		g_shr  = 3;
	} else
	{
		r_shl = 11;
		g_shr = 2;
	}
	for(int j=0; j<new_height; j++)
	{
		y = (j*bm_src->height)/new_height;
		int		ofs0 = y*bm_src->width;
		for(int i=0; i<new_width; i++)
		{
			x = (i*bm_src->width)/new_width;
			int		c = src[ofs0 + x];
			c += c<<1;
			*dest++ = ((pal[c]>>3)<<r_shl) + ((pal[c+1]>>g_shr)<<5) + (pal[c+2]>>3);
		}
	}
}
void	ResizeToBitmap24(bitmap *bm_dest, bitmap8 *bm_src, int new_width, int new_height)
{
	if(!new_width || !new_height)	return;
	bm_dest->bm = (char *)malloc(new_width*new_height*3);
	byte	*dest = (byte *)bm_dest->bm;
	byte	*src = (byte *)bm_src->bm;
	byte	*pal = bm_src->pal;
	bm_dest->bpp = 24;
	bm_dest->width = new_width;
	bm_dest->height = new_height;
	int		x, y;
	for(int j=0; j<new_height; j++)
	{
		y = (j*bm_src->height)/new_height;
		int		ofs0 = y*bm_src->width;
		for(int i=0; i<new_width; i++)
		{
			x = (i*bm_src->width)/new_width;
			int		c = src[ofs0 + x];
			c += c<<1;
			*dest++ = pal[c];
			*dest++ = pal[c+1];
			*dest++ = pal[c+2];
		}
	}
}
void	ConvertToGrayscale(bitmap *bm_dest, bitmap8 *bm_src)
{
	int		i = bm_src->width*bm_src->height;
	bm_dest->bm = (char *)malloc(i);
	byte	*dest = (byte *)bm_dest->bm;
	byte	*src = (byte *)bm_src->bm;
	byte	*pal = bm_src->pal;
	bm_dest->bpp = 8;
	bm_dest->width = bm_src->width;
	bm_dest->height = bm_src->height;
	while(i-->0)
	{
		int		c = *src++;
		c += c<<1;
		int		rgb = pal[c] + pal[c+1] + pal[c+2];
		*dest++ = rgb/3;
	}
}
/*int		dg = gamma;
			int		r = pal[c];
			int		g = pal[c+1];
			int		b = pal[c+2];
			if((r>4) || (g>4) || (b>4))
			{
				if( r+dg<0 )	dg=-r;
				if( g+dg<=0)	dg=-g+1;
				if( b+dg<0 )	dg=-b;
				if( r+dg>255 )	dg=255-r;
				if( g+dg>255 )	dg=255-g;
				if( b+dg>255 )	dg=255-b;
				*dest++ = (((r+dg)>>3)<<r_shl) + (((g+dg)>>g_shr)<<5) + ((b+dg)>>3);
			} else
				*dest++ = 0;*/