/*
	Video.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>

#include	"config.h"
#include	"console.h"
#include	"pcxfile.h"
#include	"main.h"
#include	"render.h"
#include	"model_3d.h"
#include	"video.h"
#include	"math_3d.h"

float	fov = 90, view_size = 100;
int		gamma = 0;
int		sx_size = 0, sy_size = 0, bits=16;
char	draw_render_info;
bitmap8	frame0_r, frame0_l, logo0, background0;
bitmap	font, background;
bitmap	sun;
model3d_t	mdl;

extern CPakFile	unpak;

extern float	fps;
extern int		time1, time2, time3, faces_p_s, keycode, frames, faces_p_s;
extern char		pause;
extern int		num_video_modes;
extern vm		video_modes[32];
void	DrawStatus()
{
	//mgl.DrawRectangle(5, 5, sx_size-6, sy_size-6, 0xFF000044);
	if(draw_render_info)
	{
		//mgl.DrawBar(20, 20, sx_size-20, sy_size-20, 0x00FFFF44);
		mgl.DrawString(8, sy_size-16, 0x00FFFF7F, "render=%d frame=%d faces=%d", time2-time3, time1-time3, faces_p_s);
		if(frames>64)
			mgl.DrawString(8, sy_size-26, 0x00FFFF7F, "fps=%f", fps);
	}
	if(pause)	mgl.DrawString((sx_size>>1)-20,(sy_size>>1)-4, 0x00FFFFFF, "Pause");
}

void	DrawRenderInfo()
{
	if(draw_render_info)
		draw_render_info = 0;
	else
		draw_render_info = 1;
}

void	DrawCredits()
{
	//CPrintf("**************************************");
	CPrintf("_8Miracle 3D Engine v0.22");
	CPrintf("_8   by _3Damir Sagidullin_8 (aka SpiDeR)");
	CPrintf("_8Moscow, %s %s", __DATE__, __TIME__);
	CPrintf("_8E-mail: _spider@mail.ru");
	//CPrintf("Programming:");
	//CPrintf("    Damir Sagidullin (aka SpiDeR)");
	CPrintf("_8Special thanks:");
	//CPrintf("Other programming:");
	CPrintf("_8    Vitaly Ovtchinnikov");
	//CPrintf("_8    Peter Kotov");
	//CPrintf("_8    Alexander Zhouravlev");
	//CPrintf("    Andrey Yunoshev (aka Young)");
	//CPrintf("**************************************");
}

#define	DRAW_VOLLIGHT	4
int		draw_mode=DRAW_LIGHT+DRAW_TEXTURED+DRAW_VOLLIGHT;
void	DrawMode()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%d", &draw_mode);
		mgl.SetDrawMode(draw_mode);
	}
	CPrintf("Draw mode %d:", draw_mode);
	if(draw_mode&DRAW_LIGHT)
		CPrintf("    enable light");
	else
		CPrintf("    disable light");
	if(draw_mode&DRAW_TEXTURED)
		CPrintf("    draw textured");
	else
		CPrintf("    draw flat");
	if(draw_mode&DRAW_VOLLIGHT)
		CPrintf("    enable light flares");
	else
		CPrintf("    disable light flares");
}
void	ChangeVideoMode()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%d %d", &sx_size, &sy_size);
		mgl.SetVideoMode(sx_size, sy_size);
		sx_size = mgl.Geti(SX_SIZE);
		sy_size = mgl.Geti(SY_SIZE);
		bits = mgl.Geti(BPP);
		MakeConsole();
		//	Set mouse cursor position
		ShowCursor(0);
		SetCursorPos(sx_size>>1, sy_size>>1);
	}
	CPrintf("Video mode is \"%dx%dx%d\"", sx_size, sy_size, bits);
}
void	GetVideoModes()
{
	for(int i=0; i<num_video_modes; i++)
		CPrintf("    %dx%d", video_modes[i].width, video_modes[i].height);
}
void	ChangeViewSize()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%f", &view_size);
		if(view_size<25)	view_size = 25;
		if(view_size>100)	view_size = 100;
		mgl.Setf(VIEW_SIZE, view_size);
	}
	view_size = mgl.Getf(VIEW_SIZE);
	CPrintf("View size is \"%.2f\"", view_size);
}
void	ChangeGamma()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%d", &gamma);
		mgl.Seti(GAMMA, gamma);
	}
	gamma = mgl.Geti(GAMMA);
	CPrintf("Gamma is \"%d\"", gamma);
}
int		LoadGraphics()
{
	if(!unpak.OpenPak("graphics.pak"))	return	0;
	if(LoadPCX8(&frame0_r, "console_r.pcx", &unpak)<=0)	return	0;
	if(LoadPCX8(&frame0_l, "console_l.pcx", &unpak)<=0)	return	0;
	if(LoadPCX8(&logo0, "logo.pcx", &unpak)<=0)	return	0;
	if(LoadPCX8(&background0, "backgrnd.pcx", &unpak)<=0)	return	0;
	if(LoadPCX24(&sun, "vollight.pcx", &unpak)<=0)	return	0;
	//ResizeTobitmap(&windmill, &logo0, logo0.width, logo0.height);	
	if(LoadPCX16(&font, "font.pcx", &unpak)<=0)	return	0;
	unpak.ClosePak();

	if(!LoadModel(&mdl, "model.pak"))	return	0;

	AddCommand("drawmode",	1, DrawMode);
	AddCommand("credits", 0, DrawCredits);
	AddCommand("drawfps", 0, DrawRenderInfo);
	AddCommand("videomode", 2, ChangeVideoMode);
	AddCommand("videomodelist", 0,GetVideoModes);
	AddCommand("viewsize",	1, ChangeViewSize);
	AddCommand("gamma", 1, ChangeGamma);

	return	1;
}

/*
v0.1 - добавлено цветное освещение
v0.2 - переведено под Direct Draw
v0.3 - новый формат карты + загрузка текстур из pcx файлов
v0.4 - добавлена непроходимость через стены
v0.5 - добавлены консоль и меню
v0.6 - использование порталов при рендерении
v0.7 - добавлены объекты
v0.8 - часть, которая рендерит, помещена в SoftRend.dll
v0.9 - динамическое освещение
v0.10 - зеркальные поверхности и многоуровневая прозрачность
v0.11 - спрайты
v0.12 - освещение объекта (18.07.99)
v0.13 - блики (20.07.99)
v0.14 - переделана непроходимость через стены теперь используются
clipnode-ы и теперь без глюков(22.07.99)
v0.15 - разбор entities_data, добавлены двери(31.07.99)
v0.16 - сделаны порталы(14.08.99)
v0.17 - теперь порталы и зеркала могут быть динамическими(17.08.99)
v0.18 - сделаны screen shot'ы(31.08.99)
v0.19 - трехмерная вода(08.09.99)
v0.20 - сделана поддержка сети(04.10.99)
v0.21 - переделан интерфейс SoftRend.Dll(26.10.99)
v0.22 - реализовано програмное рамытие (как в Unreal'е)
*/