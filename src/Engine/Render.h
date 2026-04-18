#ifndef	_RENDER_H
#define	_RENDER_H

#include	"struct.h"
#include	"mapfile.h"
#include	"model_3d.h"

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

typedef struct
{
	char	DllName[256];
	//	init and close
	int		(*Init) (void *set_hwnd, void *game_export);
	void	(*Done) ();
	//	screen update
	int		(*BeginActions) ();
	void	(*EndActions) ();

	int		(*SetVideoMode) (int sx, int sy);
	int		(*GetPosibleVideoModes) (vm *v_ms);
	void	(*SetDrawMode) (int new_mode);
	void	(*Clear_VS) (char	color);
	char	*(*GetVirtualScreen) ();
	float	(*Getf) (enum GlParamf what);
	int		(*Geti) (enum GlParami what);
	void	(*Setf) (enum GlParamf what, float a);
	void	(*Seti) (enum GlParami what, int a);
	//	global render
	void	(*AddDynamicLight) (vec3_t pos, float r, float g, float b);
	void	(*SetRenderMap) (TMap *render_map);
	int		(*RenderWorld) (vec3_t cam_pos, vec3_t angle, int cur_time);
	//	draw 3d
	void	(*DrawModel) (vec3_t pos, vec3_t angle, model3d_t *model, float scale, byte alpha, byte type);
	void	(*DrawSprite) (vec3_t pos, bitmap *bitmap, float scale, byte alpha, byte type);	
	//	font and background
	void	(*SetFont) (bitmap *bm);
	void	(*SetBackground) (bitmap *bm);
	//	clip rectangle
	void	(*SetFullClipRectangle) ();
	void	(*SetClipRectangle) (rect_t rect);
	void	(*GetClipRectangle) (rect_t *rect);
	//	draw 2d
	void	(*DrawBitmap) (int x,int y, bitmap *bm, byte alpha, byte type);
	void	(*DrawString) (int x, int y, uint color, char *format, ...);
	void	(*DrawBackground) (int time);
	void	(*DrawLine) (int x1, int y1, int x2, int y2, uint color);
	void	(*DrawSpline) (int n, scrpoint_t *p, int resolution, uint color, float k);
	void	(*DrawBar) (int x1, int y1, int x2, int y2, uint c);
	void	(*DrawRectangle) (int x1, int y1, int x2, int y2, uint c);
	//	pixel
	void	(*SetPixel) (int x, int y, uint color);
	uint	(*GetPixel) (int x, int y);
} MiracleGraphicLibrary;

extern	MiracleGraphicLibrary	mgl;

int		LoadGraphicLibrary(char *name);
void	CloseGraphicLibrary();

#endif