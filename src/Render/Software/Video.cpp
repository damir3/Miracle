/*
	Video.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<DDraw.h>
#include	<Stdio.h>
#include	<Math.h>

#include	"Math_3d.h"
#include	"Draw_2d.h"
#include	"SpanBuf.h"
#include	"DrawFace.h"
#include	"Surface.h"
#include	"Video.h"

char	*virtual_screen = NULL;
int		screen_row_table[MAX_SY_SIZE];
ushort	red_color_map[256][256];
ushort	green_color_map[256][256];
ushort	blue_color_map[256][256];
ushort	r_color_map[64][256];
ushort	g_color_map[64][256];
ushort	b_color_map[32][256];
ushort	transparency_map[16][0x10000];
int		sx_size = 0, sy_size = 0, vs_size = 0, bits=0;
int		draw_mode = 7, gamma=0;
char	ddrawinit=0;
int		num_video_modes=0;
vm		video_modes[32];

GameImport	gi;

HWND				hwnd;
DDSURFACEDESC		ddsd;
LPDIRECTDRAW		lpDD = NULL;
LPDIRECTDRAWSURFACE lpDDSPrimary = NULL;
LPDIRECTDRAWSURFACE lpDDSBack = NULL;

extern bitmap	font, background, bumpmap, phongmap;

HRESULT CALLBACK ModeCallback(LPDDSURFACEDESC pdds, LPVOID lParam)
{
	int bpp    = pdds->ddpfPixelFormat.dwRGBBitCount;
	if((bpp==16) || (bpp==15))
	{
		video_modes[num_video_modes].width = pdds->dwWidth;
		video_modes[num_video_modes].height = pdds->dwHeight;
		video_modes[num_video_modes].bpp = bpp;
		num_video_modes++;
	}
    return S_FALSE;
}

int		DDInit()
{
	HRESULT	err;
	err = DirectDrawCreate(NULL, &lpDD, NULL);
	if (err != DD_OK)	return	FALSE;

	err = lpDD->SetCooperativeLevel(hwnd,
		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX);
	if (err != DD_OK)	return	FALSE;

	//	Перечислить все возможные видео режимы.
	lpDD->EnumDisplayModes(0,NULL,NULL,ModeCallback);

	return	TRUE;
}

int		DDSetMode (int width, int height)
{
	DDSURFACEDESC	ddsd;
	HRESULT			err;

    err = lpDD->SetDisplayMode (width, height, 16);
    if (err != DD_OK)	return	FALSE;

    // get rid of any previous surfaces.
    if (lpDDSBack)		lpDDSBack->Release(),	lpDDSBack = NULL;
    if (lpDDSPrimary)	lpDDSPrimary->Release(),lpDDSPrimary = NULL;

	memset (&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.dwBackBufferCount = 2;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                          DDSCAPS_FLIP |
                          DDSCAPS_COMPLEX |
						  DDSCAPS_VIDEOMEMORY;
    // try to get a triple buffered video memory surface.
    err = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);

	if (err != DD_OK)
    {//	try to get a double buffered video memory surface.
		ddsd.dwBackBufferCount = 1;
		err = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
    }

    if (err != DD_OK)
    {//	settle for a main memory surface.
		ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
		err = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
    }

	if (err != DD_OK)	return	FALSE;

    // get a pointer to the back buffer
    DDSCAPS caps;
    caps.dwCaps = DDSCAPS_BACKBUFFER;
    err = lpDDSPrimary->GetAttachedSurface(&caps, &lpDDSBack);
    if (err != DD_OK)	return	FALSE;

	sx_size = width;
    sy_size = height;

	return	TRUE;
}

void	Copy_VS_To_Screen(char *scr, int lPitch)
{
	for (int i=0, k=0; i<sy_size; i++, scr+=lPitch, k += (sx_size*BytesPP))
		memcpy(scr, virtual_screen+k, sx_size*BytesPP);
}

void	CreateColorMap(int gamma)
{
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
	for(int	i=0; i<256; i++)
	{
		for(int	j=0; j<64;	j++)
		{
			int	c = ((i*j)>>5) + gamma;
			if( c<0 )	c=0;
			if( c>255 )	c=255;
			int	r = (c>>3)<<r_shl;
			int	g = (c>>g_shr)<<5;
			int	b = c>>3;
			red_color_map[j][i] = r;
			green_color_map[j][i] =	g;
			blue_color_map[j][i] =	b;
		}
	}
}

void	CreateTransparencyMap()
{
	if(bits==15)
	{
		for(int alpha=0; alpha<16; alpha++)
			for(int i=0; i<0x10000; i++)
			{
				int	red = (i>>10)&31;
				int green = (i>>5)&31;
				int	blue = i&31;
				int	r = (red*alpha)>>4;
				int	g = (green*alpha)>>4;
				int	b = (blue*alpha)>>4;
				transparency_map[alpha][i] = (r<<10)+(g<<5)+b;
			}
	} else
	{
		for(int alpha=0; alpha<16; alpha++)
			for(int i=0; i<0x10000; i++)
			{
				int	red = i>>11;
				int green = (i>>5)&63;
				int	blue = i&31;
				int	r = (red*alpha)>>4;
				int	g = (green*alpha)>>4;
				int	b = (blue*alpha)>>4;
				transparency_map[alpha][i] = (r<<11)+(g<<5)+b;
			}
	}
}
void	CreateRGBColorMap()
{
	for(int j=0; j<32; j++)
	{
		int	i=256;
		while(i--)
		{
			int	c= j+(i>>3);
			if(c>31)	c=31;
			b_color_map[j][i] = c;
		}
	}
	for(j=0; j<64; j++)
	{
		int		c, i=256;
		if(bits==15)
		{
			while(i--)
			{
				c = (j&31) + (i>>3);
				if(c>31)	c=31;
				r_color_map[j][i] = c<<10;
				g_color_map[j][i] = c<<5;
			}
		} else
		{
			while(i--)
			{
                c = j + (i>>2);
				if(c>63)	c=63;
				g_color_map[j][i] = c<<5;
				r_color_map[j][i] = (c&62)<<10;
			}
		}
	}
}
void	Clear_VS(char	color)
{
	memset(virtual_screen, color, vs_size);
}

int		BeginActions()
{
	if(!ddrawinit)	return	0;
	ddsd.dwSize = sizeof(ddsd);
	int		hres;
	color_t	color = {-1,-1,-1,-1};
	while ((hres = lpDDSBack->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);
	if(hres == DDERR_SURFACELOST)
	{
		lpDDSPrimary->Restore();
		while ((hres = lpDDSBack->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);
	}
	return	1;
}

void	EndActions()
{
	Copy_VS_To_Screen((char *)ddsd.lpSurface, ddsd.lPitch);
	lpDDSBack->Unlock(NULL);
	lpDDSPrimary->Flip(NULL, 0);
}
void	ResetVideoMode(int sx, int sy)
{
	sx_size = sx;
	sy_size = sy;
	vs_size = sx_size*sy_size*BytesPP;
	virtual_screen = (char *)realloc(virtual_screen, vs_size);
	memset(screen_row_table, 0, sizeof(int)*MAX_SY_SIZE);
	for (int i=0, temp=0; i<sy_size; i++, temp += sx_size)	screen_row_table[i] = temp;
	SetSpanBufSize(sy);
	//	Считаем параметры камеры для данного режима
	CalculateCameraParameters();
	int		bpp = bits;
	//	Определяем сколько бит на пиксель
	DDSURFACEDESC	ddsd;
	ddsd.dwSize = sizeof(ddsd);
	int		hres;
	while ((hres = lpDDSPrimary->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);
	if(hres == DDERR_SURFACELOST)
	{
		lpDDSPrimary->Restore();
		while ((hres = lpDDSPrimary->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING);
	}
	if(ddsd.ddpfPixelFormat.dwRBitMask>>15)	bits = 16;
	else	bits = 15;
	lpDDSPrimary->Unlock(NULL);
	if(bpp!=bits)
	{
		//	Создаем карты цветов
		CreateColorMap(0);
		CreateTransparencyMap();
		CreateRGBColorMap();
		//	Очищаем кеш
		RefreshSurfaceCache();
	}
	SetFullClipRectangle();
}

int		SetVideoMode(int sx, int sy)
{
	if((sx!=sx_size) || (sy!=sy_size))
	{
		ddrawinit = 0;
		if(!DDSetMode(sx, sy))
		{
			sx = sx_size;
			sy = sy_size;
			if(!DDSetMode(sx, sy))	return	FALSE;
		}
		ResetVideoMode(sx, sy);
		ddrawinit = 1;
	}
	return	TRUE;
}

int		InitDll(HWND set_hwnd, GameImport *set_gi)
{
	if(!set_hwnd || !set_gi)	return	FALSE;
	hwnd = set_hwnd;
	gi = *set_gi;
	//	Инициализация Direct Draw
	if(!DDInit())	return	FALSE;
	if(!DDSetMode(640, 480))	return	FALSE;
	//	Создаем кэш для текстур
	InitCache();
	//	Инициализируем внутрение данные для данного режима
	ResetVideoMode(sx_size, sy_size);
	//	Инициализируем указатели необходимые для рендеринга
	InitDefaultPointList();
	ddrawinit = 1;
	bumpmap.bm = phongmap.bm = background.bm = font.bm = NULL;
	return	TRUE;
}
void	CloseDll()
{
	ddrawinit = 0;
	if(lpDD)
	{
		lpDD->Release();
		lpDD = NULL;
	}
	if(lpDDSPrimary)
	{
		lpDDSPrimary->Release();
		lpDDSPrimary = NULL;
	}
	if(lpDDSBack)
	{
		lpDDSBack->Release();
		lpDDSBack= NULL;
	}
	free(virtual_screen);
}
void	SetDrawMode(int new_mode)
{
	draw_mode = new_mode;
	RefreshSurfaceCache();
}
int		GetPosibleVideoModes(vm *v_ms)
{
	memcpy(v_ms, video_modes, num_video_modes*sizeof(vm));
	return	num_video_modes;
}
char	*GetVirtualScreen()
{
	return	virtual_screen;
}
void	CloseSoftRendDll()
{
	free(virtual_screen);	virtual_screen = NULL;
}

float	Getf(enum GlParamf what)
{
	switch(what)
	{
	case	FOV:
		return	view_angle;
		break;
	case	VIEW_SIZE:
		return	view_size;
		break;
	case	SCALE_X:
		return	mdl_scale[0];
		break;
	case	SCALE_Y:
		return	mdl_scale[1];
		break;
	case	SCALE_Z:
		return	mdl_scale[2];
		break;
	case	SCALE_BX:
		return	bitmap_scale[0];
		break;
	case	SCALE_BY:
		return	bitmap_scale[1];
		break;
	}
	return	0;
}
int		Geti(enum GlParami what)
{
	switch(what)
	{
	case	GAMMA:
		return	gamma;
		break;
	case	CLIP_LEFT:
		return	clip_rect.left;
		break;
	case	CLIP_TOP:
		return	clip_rect.top;
		break;
	case	CLIP_RIGHT:
		return	clip_rect.right;
		break;
	case	CLIP_BOTTOM:
		return	clip_rect.bottom;
		break;
	case	SX_SIZE:
		return	sx_size;
		break;
	case	SY_SIZE:
		return	sy_size;
		break;
	case	BPP:
		return	bits;
		break;
	}
	return	0;
}

void	Setf(enum GlParamf what, float a)
{
	switch(what)
	{
	case	FOV:
		view_angle = a;
		CalculateCameraParameters();
		break;
	case	VIEW_SIZE:
		view_size = a;
		CalculateCameraParameters();
		break;
	case	SCALE_X:
		mdl_scale[0] = a;
		break;
	case	SCALE_Y:
		mdl_scale[1] = a;
		break;
	case	SCALE_Z:
		mdl_scale[2] = a;
		break;
	case	SCALE_BX:
		bitmap_scale[0] = a;
		break;
	case	SCALE_BY:
		bitmap_scale[1] = a;
		break;
	}
}

void	Seti(enum GlParami what, int a)
{
	switch(what)
	{
	case	GAMMA:
		gamma = a;
		RefreshSurfaceCache();
		break;
	case	CLIP_LEFT:
		clip_rect.left = ClipScreenX(a);
		break;
	case	CLIP_TOP:
		clip_rect.top = ClipScreenY(a);
		break;
	case	CLIP_RIGHT:
		clip_rect.right = ClipScreenX(a);
		break;
	case	CLIP_BOTTOM:
		clip_rect.bottom = ClipScreenY(a);
		break;
	}
}
