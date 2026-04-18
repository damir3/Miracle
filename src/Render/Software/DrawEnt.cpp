/*
	DrawEnt.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	"Draw_2D.h"	//	delete

#include	"Math_3D.h"
#include	"DrawEnt.h"
#include	"DrawFace.h"
#include	"DrawSpan.h"
#include	"MapFile.h"
#include	"Portal.h"
#include	"Render.h"

extern TMap	map;

int		cur_ent;

void	RenderEntNodeFaces(int node, int side)
{
	int		n = map.nodes[node].numfaces;
	int		face = map.nodes[node].firstface;
	FindNearestDynamicLights(map.nodes[node].planenum, side);
	while(n--)
	{
		if(map.faces[face].side == side)
		{
			face_type = map.faces[face].type;
			if(face_type>=0)
				DrawFace(face);
			else
			{
				if(face_type==-1)
				{
					if(cur_ent>=0)
						DrawDynamicMirror(face);
					else
						DrawMirror(face);
				} else
					DrawPortal(face);
				FindNearestDynamicLights(map.nodes[node].planenum, side);
			}
		}
		face++;
	}
}

void	RenderEntNode(int node)
{
	if(node>=0)
	{
		if(DistToCamera(&map.planes[map.nodes[node].planenum])<0)
		{
			RenderEntNode(map.nodes[node].children[1]);
			RenderEntNodeFaces(node, 1);
			RenderEntNode(map.nodes[node].children[0]);
		} else
		{
			RenderEntNode(map.nodes[node].children[0]);
			RenderEntNodeFaces(node, 0);
			RenderEntNode(map.nodes[node].children[1]);
		}
	}
}

void	DrawDynamicBSPEntity(int num_ent)
{
	d_wall_t	*ent = &map.dwalls[num_ent];
	int		model = ent->model;

	if(!model || (model>=map.nummodels))	return;
	vec3_t	mins, maxs;
	int		*angle = (int *)ent->angle;
	if((!angle[0]) && (!angle[1]) && (!angle[2]))
	{
		VectorAdd(map.models[model].mins, ent->pos, mins);
		VectorAdd(map.models[model].maxs, ent->pos, maxs);
		if(!BoxInFrustrum(mins, maxs))	return;
	} else
	{
		VectorAdd(ent->pos, ent->turn_pos, mins);
		VectorCopy(mins, maxs);
		for(int i=0; i<3; i++)
		{
			mins[i] -= ent->radius;
			maxs[i] += ent->radius;
		}
		if(!BoxInFrustrum(mins, maxs))	return;
	}

	int		i=ent->first_sector, n=ent->num_sectors;
	while(n--)
	{
		int		sector = map.ent_sectors[i++];
		if(vis_sectors[sector>>3] & (1<<(sector&7)))
			break;
	}
	if(n<0)	return;	//	не виден
	SaveViewInfo();
	vec3_t	sub;
	VectorSubtract(cam_pos, ent->pos, cam_pos);
	VectorSubtract(cam_pos, ent->turn_pos, sub);
	QuickRotateVector(sub, ent->tmatrix);
	VectorAdd(ent->turn_pos, sub, cam_pos);
	QuickRotateVector(main_matrix[0], ent->tmatrix);
	QuickRotateVector(main_matrix[1], ent->tmatrix);
	QuickRotateVector(main_matrix[2], ent->tmatrix);

	cur_ent = num_ent;

	ComputeViewMatrix();
	ComputeViewFrustrum();
	RenderEntNode(map.models[model].headnode[0]);
	RestoreViewInfo();
}

void	DrawStaticBSPEntity(int num_ent)
{
	s_wall_t	*ent = &map.swalls[num_ent];
	int		model = ent->model;
	//	проверка на правильность номер модели
	if(!model || (model>=map.nummodels))	return;
	//	проверка на frustrum
	if(!BoxInFrustrum(map.models[model].mins, map.models[model].maxs))	return;

	int		i=ent->first_sector, n=ent->num_sectors;
	while(n--)
	{
		int		sector = map.ent_sectors[i++];
		if(vis_sectors[sector>>3] & (1<<(sector&7)))
			break;
	}
	if(n<0)		return;	//	не виден
	cur_ent = -1;
	RenderEntNode(map.models[model].headnode[0]);
}

void	DrawDynamicEntities()
{
	int		i = map.numdwalls;
	mode = 1;
	while((i--)>0)	DrawDynamicBSPEntity(i);
}

void	DrawStaticEntities()
{
	int		i = map.numswalls;
	mode = 1;
	while((i--)>0)	DrawStaticBSPEntity(i);
}
/*
vec3_t	edge_v[8];
	edge_v[0][0] = edge_v[1][0] = edge_v[2][0] = edge_v[3][0] = mins[0] - ent->turn_pos;
	edge_v[4][0] = edge_v[5][0] = edge_v[6][0] = edge_v[7][0] = maxs[0] - ent->turn_pos;
	edge_v[0][1] = edge_v[3][1] = edge_v[7][1] = edge_v[4][1] = mins[1] - ent->turn_pos;
	edge_v[1][1] = edge_v[2][1] = edge_v[5][1] = edge_v[6][1] = maxs[1] - ent->turn_pos;
	edge_v[0][2] = edge_v[1][2] = edge_v[5][2] = edge_v[4][2] = mins[2] - ent->turn_pos;
	edge_v[2][2] = edge_v[3][2] = edge_v[7][2] = edge_v[6][2] = maxs[2] - ent->turn_pos;*/