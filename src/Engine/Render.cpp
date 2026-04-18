/*
	Render.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<windows.h>
#include	<stdio.h>

#include	"console.h"
#include	"render.h"

MiracleGraphicLibrary	mgl;

static char		*DllName;
static HMODULE	hModule;

int		SetProc(FARPROC *proc, char *proc_name)
{
	*proc = GetProcAddress(hModule, proc_name);
	if(!*proc)
	{
		CPrintf("Function \"%s\" not found in \"%s\"", proc_name, DllName);
		return	0;
	}
	return	1;
}
int		LoadGraphicLibrary(char *name)
{
	DllName = name;
	strcpy(mgl.DllName, DllName);
	hModule = LoadLibrary(name);
	if(hModule==NULL)
	{
		CPrintf("Unable to load library \"%s\"", DllName);
		return 0;
	}
	//	render all
	if(!SetProc((FARPROC *)&mgl.SetRenderMap, "SetRenderMap"))	return	0;
	if(!SetProc((FARPROC *)&mgl.RenderWorld, "RenderWorld"))	return	0;
	if(!SetProc((FARPROC *)&mgl.AddDynamicLight, "AddDynamicLight"))	return	0;
	//	2d draw
	if(!SetProc((FARPROC *)&mgl.SetFullClipRectangle, "SetFullClipRectangle"))	return	0;
	if(!SetProc((FARPROC *)&mgl.SetClipRectangle, "SetClipRectangle"))	return	0;
	if(!SetProc((FARPROC *)&mgl.GetClipRectangle, "GetClipRectangle"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawBitmap, "DrawBitmap"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawString, "DrawString"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawBackground, "DrawBackground"))	return	0;
	if(!SetProc((FARPROC *)&mgl.SetFont, "SetFont"))	return	0;
	if(!SetProc((FARPROC *)&mgl.SetBackground, "SetBackground"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawLine, "DrawLine"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawSpline, "DrawSpline"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawBar, "DrawBar"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawRectangle, "DrawRectangle"))	return	0;

	if(!SetProc((FARPROC *)&mgl.SetPixel, "SetPixel"))	return	0;
	if(!SetProc((FARPROC *)&mgl.GetPixel, "GetPixel"))	return	0;
	//	3d draw
	if(!SetProc((FARPROC *)&mgl.DrawModel, "DrawModel"))	return	0;
	if(!SetProc((FARPROC *)&mgl.DrawSprite, "DrawSprite"))	return	0;
	//	video
	if(!SetProc((FARPROC *)&mgl.Init, "InitDll"))	return	0;
	if(!SetProc((FARPROC *)&mgl.Done, "CloseDll"))	return	0;

	if(!SetProc((FARPROC *)&mgl.BeginActions, "BeginActions"))	return	0;
	if(!SetProc((FARPROC *)&mgl.EndActions, "EndActions"))	return	0;

	if(!SetProc((FARPROC *)&mgl.SetVideoMode, "SetVideoMode"))	return	0;
	if(!SetProc((FARPROC *)&mgl.GetPosibleVideoModes, "GetPosibleVideoModes"))	return	0;
	if(!SetProc((FARPROC *)&mgl.SetDrawMode, "SetDrawMode"))	return	0;
	if(!SetProc((FARPROC *)&mgl.Clear_VS, "Clear_VS"))	return	0;
	if(!SetProc((FARPROC *)&mgl.GetVirtualScreen, "GetVirtualScreen"))	return	0;
	if(!SetProc((FARPROC *)&mgl.Getf, "Getf"))	return	0;
	if(!SetProc((FARPROC *)&mgl.Geti, "Geti"))	return	0;
	if(!SetProc((FARPROC *)&mgl.Setf, "Setf"))	return	0;
	if(!SetProc((FARPROC *)&mgl.Seti, "Seti"))	return	0;
	return 1;
}
void	CloseGraphicLibrary()
{
	mgl.Done();
	FreeLibrary(hModule);
	memset(&mgl, 0, sizeof(MiracleGraphicLibrary));
}
