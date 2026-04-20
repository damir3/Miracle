/*
	ScrShot.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Windows.h>
#include	<Stdio.h>
#include	"Console.h"
#include	"Main.h"
#include	"Render.h"
#include	"ScrShot.h"
#include	"Video.h"

// TGA file header
static int	cur_scrshotnum=0;
static char header[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0};
extern char	cur_key_state;

typedef	struct
{
	short	x, y;
	char	bpp;
	char	type;
} tgadata;

void	SaveScreenShot()
{
	HANDLE	f=NULL;
	char	fullname[1280];
	char	filename[256];
	do
	{
		sprintf(filename, "shot%04d.tga", cur_scrshotnum++);
		strcpy(fullname, program_path);
		strcat(fullname, filename);
		f = CreateFile(fullname, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if(f!=INVALID_HANDLE_VALUE)	break;
		if(GetLastError()!=ERROR_FILE_EXISTS)
		{
			CPrintf("Can't create file \"%s\"", filename);
			return;
		}
	} while(cur_scrshotnum<10000);
	if(cur_scrshotnum>=10000)
	{
		CPrintf("Too many screen shots exists");
		cur_scrshotnum=0;
		return;
	}
	tgadata	data;
	int		bw;
	data.x = sx_size;
	data.y = sy_size;
	data.bpp = 0x10;
	data.type = 0x20;
	WriteFile(f, &header, sizeof(header), (DWORD *)&bw, NULL);
	WriteFile(f, &data, sizeof(tgadata), (DWORD *)&bw, NULL);
	ushort	*vs = (ushort *)mgl.GetVirtualScreen();
	if(bits==16)
	{
		int		i = sx_size*sy_size;
		while(i-->0)
		{
			ushort	c = vs[i];
			vs[i] = (c&31) + ((c>>6)<<5);
		}
	}
	WriteFile(f, vs, sx_size*sy_size*2, (DWORD *)&bw, NULL);
	CloseHandle(f);
	CPrintf("Save screen shot to \"%s\"", filename);
	cur_key_state = 2;
}