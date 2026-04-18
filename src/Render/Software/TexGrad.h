#ifndef _TMAP3D_H
#define _TMAP3D_H

extern int		minmiplevel, maxmiplevel;

void	ComputeFaceMipLevel(int, point_3d **);
void	ComputeTextureGradients(int, int, float, float);
void	ComputeDynamicTextureGradients(int , int , float , float , t_plane *);
void	ComputeMarkTextureGradients(wmark_t *);
void	ComputeZGradients(t_plane *);

#endif