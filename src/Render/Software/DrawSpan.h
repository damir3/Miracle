#ifndef _DRAWSPAN_H
#define _DRAWSPAN_H

typedef struct
{
	int		width, height;
	short	*bitmap;
} texmap;

extern short		*current_texture;
extern int			face_type, time;
extern int			tex_row_table[258];
extern short		face_color;

void	DrawAffineTexturedLine(int n, char *dest, int u, int v, int du, int dv);
void	DrawSpan(int sy, int sx_start, int sx_end);
void	DrawWaterSpan(int sy, int sx_start, int sx_end);
void	UpdateWaves();
void	SetTexture(short *bm, int bm_width, int bm_height);
void	SetSurfaceType(int type);

#endif
