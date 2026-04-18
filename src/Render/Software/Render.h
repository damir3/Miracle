#ifndef _RENDER_H
#define _RENDER_H

#include	"MapFile.h"

extern TMap	map;
extern char	*vis_sectors;
extern int	mode;
extern int	first_num_trans_faces, num_trans_faces;
extern int	trans_faces[512];

int		FindVisibleSectors(vec3_t pos);
void	VisitVisibleSectors();
void	DrawObjectsAndWater();
void	RenderNode(int);
int		FindSector(vec3_t pos);
void	FindNearestDynamicLights(int, int);

void	AddDynamicLight(vec3_t pos, float r, float g, float b);
void	SetRenderMap(TMap *render_map);
int		RenderWorld(vec3_t view_pos, vec3_t angle, int cur_time);

#endif
