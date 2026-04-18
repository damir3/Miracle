#ifndef _VIDEO_H
#define _VIDEO_H

#include	"struct.h"

#define	MAX_SY_SIZE		2048

#define	DRAW_LIGHT		1
#define	DRAW_TEXTURED	2
#define	DRAW_VOLLIGHT	4
#define	BytesPP		2

enum GlParamf
{
	FOV,
	VIEW_SIZE,
	SCALE_X,
	SCALE_Y,
	SCALE_Z,
	SCALE_BX,
	SCALE_BY
};

enum GlParami
{
	GAMMA,
	CLIP_LEFT,
	CLIP_TOP,
	CLIP_RIGHT,
	CLIP_BOTTOM,
	SX_SIZE,
	SY_SIZE,
	BPP
};

extern char		*virtual_screen;
extern int		screen_row_table[MAX_SY_SIZE];
extern int		sx_size, sy_size, vs_size, bits;
extern int		draw_mode;

void	CreateColorMap(int gamma);

int		SetVideoMode(int sx, int sy);
void	Clear_VS(char color);
int		BeginActions();
void	EndActions();
int		GetPosibleVideoModes(vm *v_ms);
void	SetDrawMode(int new_mode);
char	*GetVirtualScreen();
float	Getf(enum GlParamf what);
int		Geti(enum GlParami what);
void	Setf(enum GlParamf what, float a);
void	Seti(enum GlParami what, int a);

#endif
