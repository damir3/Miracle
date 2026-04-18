#ifndef _DRAWFACE_H
#define _DRAWFACE_H

#include	"Math_3D.h"

void	DrawFace(int);
void	DrawMarkOnWall(wmark_t *mark);
void	InitDefaultPointList(void);
int		ClipPoly(int n, point_3d **vl, int codes_or, point_3d ***out_vl);
void	ComputeEdge(point_3d *a, point_3d *b);

#endif