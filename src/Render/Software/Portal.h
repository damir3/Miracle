#ifndef _PORTAL_H
#define _PORTAL_H

extern int	mirror, portal;

void	DrawMirror(int);
void	DrawDynamicMirror(int);
void	DrawPortal(int);
int		ClipPortalPoly(int n, point_3d **vl, point_3d ***out_vl);

#endif