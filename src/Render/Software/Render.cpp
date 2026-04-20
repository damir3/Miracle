/*
	Render.cpp
*/
#include	<Math.h>
#include	<Memory.h>

#include	"Draw_2D.h"
#include	"Draw_3D.h"
#include	"DrawEnt.h"
#include	"DrawFace.h"
#include	"DrawModel.h"
#include	"SpanBuf.h"
#include	"MapFile.h"
#include	"Math_3D.h"
#include	"Surface.h"
#include	"Portal.h"
#include	"Render.h"
#include	"Video.h"

char	vis_face1[MAX_MAP_FACES/8+1], vis_face2[MAX_MAP_FACES/8+1];
char	*vis_face = vis_face1;
char	vis_node1[MAX_MAP_NODES], vis_node2[MAX_MAP_NODES];
char	*vis_node = vis_node1;
char	*vis_sectors;

TMap	map;
int		mode, faces_p_s;

int				num_dyn_lights=0;
dynamic_light	dyn_lights[MAX_NUM_DYNAMIC_LIGHTS];
extern int				num_dlights;
extern dynamic_light	*dlights[MAX_NUM_DYNAMIC_LIGHTS];

void	FindNearestDynamicLights(int plane_num, int side)
{
	//	find nearest light sources
	t_plane	*plane = &map.planes[plane_num];
	num_dlights = 0;
	int	i = num_dyn_lights;
	if(side)
	{
		while(i--)
		{
			float	dist = -PointPlaneDist(dyn_lights[i].pos, plane);
			if((dist>0) && (dist<dyn_lights[i].max_radius))
				dlights[num_dlights++]=&dyn_lights[i];
		}
	} else
	{
		while(i--)
		{
			float	dist = PointPlaneDist(dyn_lights[i].pos, plane);
			if((dist>0) && (dist<dyn_lights[i].max_radius))
				dlights[num_dlights++]=&dyn_lights[i];
		}
	}
}
void	RenderNodeFaces(int node, int side)
{
	int		n = map.nodes[node].numfaces;
	int		face = map.nodes[node].firstface;
	FindNearestDynamicLights(map.nodes[node].planenum, side);
	while(n--)
	{
		if(vis_face[face>>3] & (1 << (face&7)))
		{
			if(map.faces[face].side == side)
			{
				/*if(!map.nodes[node].first_track)
				{*/
				face_type = map.faces[face].type;
				if(face_type>=0)
				{
					DrawFace(face);
				} else
				{
					if(face_type==-1)
						DrawMirror(face);
					else
						DrawPortal(face);
					FindNearestDynamicLights(map.nodes[node].planenum, side);
				}/*
				} else
					DrawMarkOnWall(map.nodes[node].first_track);*/
			}
			vis_face[face>>3] &= ~(1 << (face&7));
		}
		face++;
	}
}

void	RenderNode(int node)
{
	if (node>=0 && vis_node[node])
	{
		if(DistToCamera(&map.planes[map.nodes[node].planenum])<0)
		{
			RenderNode(map.nodes[node].children[1]);
			RenderNodeFaces(node, 1);
			RenderNode(map.nodes[node].children[0]);
		} else
		{
			RenderNode(map.nodes[node].children[0]);
			RenderNodeFaces(node, 0);
			RenderNode(map.nodes[node].children[1]);
		}
	}
}

int		BBoxInsidePlane(short *mins, short *maxs, t_plane *plane)
{
	short pt[3];
	for (int i=0; i < 3; i++)
		if(*(int *)&plane->normal[i]>=0)
			pt[i] = maxs[i];
		else
			pt[i] = mins[i];
	return	((plane->normal[0]*pt[0])+(plane->normal[1]*pt[1])+(plane->normal[2]*pt[2]))>=plane->dist;
}

int		NodeInFrustrum(t_node *node)
{
	if(!BBoxInsidePlane(node->mins, node->maxs, frustrum_planes+0)
	|| !BBoxInsidePlane(node->mins, node->maxs, frustrum_planes+1)
	|| !BBoxInsidePlane(node->mins, node->maxs, frustrum_planes+2)
	|| !BBoxInsidePlane(node->mins, node->maxs, frustrum_planes+3))
		return	0;
	return	1;
}

int		SectorInFrustrum(t_sector *leaf)
{
	if(!BBoxInsidePlane(leaf->mins, leaf->maxs, frustrum_planes+0)
	|| !BBoxInsidePlane(leaf->mins, leaf->maxs, frustrum_planes+1)
	|| !BBoxInsidePlane(leaf->mins, leaf->maxs, frustrum_planes+2)
	|| !BBoxInsidePlane(leaf->mins, leaf->maxs, frustrum_planes+3))
		return	0;
	return	1;
}

void	MarkSectorFaces(int sector)
{
	int	first_surface = map.sectors[sector].firstmarksurface;
    int	num_surfaces = map.sectors[sector].nummarksurfaces;

	int	i = num_surfaces;
	while(i--)
	{
		int	face = map.marksurfaces[first_surface+i];
		face_consts	*f = &map.faces_consts[face];
		vis_face[face>>3] |= 1<<(face & 7);
	}
}

void	MarkVisibleFaces(int node)
{
	if(node<0)
	{
		node = ~node;
		if((vis_sectors[node>>3] & (1<<(node&7))) && SectorInFrustrum(&map.sectors[node]))
			MarkSectorFaces(node);
		return;
	} else
	if(vis_node[node])
	{
		if (!NodeInFrustrum(&map.nodes[node])) vis_node[node] = 0;
		else
		{
			MarkVisibleFaces(map.nodes[node].children[0]);
			MarkVisibleFaces(map.nodes[node].children[1]);
		}
	}
}

int		MarkVisibleNodes(int node)
{
	if (node >= 0)
	{
		vis_node[node] = MarkVisibleNodes(map.nodes[node].children[0]) |
			MarkVisibleNodes(map.nodes[node].children[1]);
		return vis_node[node];
	}
	else
	{
		node = ~node;
		if((vis_sectors[node>>3] & (1<<(node&7))) && SectorInFrustrum(&map.sectors[node]))
			return	1;
		else
			return	0;
	}
}

void	VisitVisibleSectors()
{
	//	Mark all visible nodes
	MarkVisibleNodes((int) map.models[0].headnode[0]);
	//	Mark all visible polygons
	MarkVisibleFaces((int) map.models[0].headnode[0]);
}

int		FindSector(vec3_t pos)
{//	find sector where camera is located
	int	n = map.models[0].headnode[0];
	while (n >= 0)
	{
		t_node *node = &map.nodes[n];
		n = node->children[PointPlaneDist(pos, &map.planes[node->planenum])<0];
	}
	return ~n;
}

int		FindVisibleSectors(vec3_t pos)
{
	//	Find sector where the camera is located
	int		sector = FindSector(pos);
    int		v = map.sectors[sector].visofs;
	vis_sectors = (char *)map.vismap + v;
	return	sector;
}

void	SetRenderMap(TMap *render_map)
{
	map = *render_map;
	memset(vis_face1, 0, (map.numfaces>>3)+1);
	memset(vis_face2, 0, (map.numfaces>>3)+1);
	memset(vis_node1, 0, map.numnodes);
	memset(vis_node2, 0, map.numnodes);
	RemakeCache();
}
void	AddDynamicLight(vec3_t pos, float r, float g, float b)
{	//	adding dynamic light source to scene
	//	(must be added for each frame before rendering)
	if(num_dyn_lights<MAX_NUM_DYNAMIC_LIGHTS)
	{
		VectorCopy(pos, dyn_lights[num_dyn_lights].pos);
		dyn_lights[num_dyn_lights].i[0]=r*0x1000;
		dyn_lights[num_dyn_lights].i[1]=g*0x1000;
		dyn_lights[num_dyn_lights].i[2]=b*0x1000;
		dyn_lights[num_dyn_lights].max_radius2 = (r+g+b)*192;
		dyn_lights[num_dyn_lights].max_radius = float(sqrt(dyn_lights[num_dyn_lights].max_radius2));
		num_dyn_lights++;
	}
}
int		first_num_trans_faces, num_trans_faces;
int		trans_faces[512];
//	render of objects, sprites and water in a specific order
void	DrawObjectsAndWater()
{
	mode = 3;
	t_plane	plane;
	for(int i=0; i<numobjects; i++)	DrawModel_(&vis_objects[i]);
	Transform3DBitmapsPoints();

	for(i=first_num_trans_faces; i<num_trans_faces; i++)
	{
		int		face = trans_faces[i];
		face_type = map.faces[face].type;
		int		plane_num = map.faces[face].planenum;
		if(map.faces[face].side)
		{
			VectorSubtract(vec3_null, map.planes[plane_num].normal, plane.normal);
			plane.dist = -map.planes[plane_num].dist;
		} else
			memcpy(&plane, &map.planes[plane_num], sizeof(plane));
		FindAndDrawEnts(&plane);
		DrawFace(face);
	}
	DrawOtherEnts();
}
extern int		sx_min, sx_max, sy_min, sy_max;
//	render of the entire scene
int		RenderWorld(vec3_t view_pos, vec3_t angle, int cur_time)
{
	if(!map.map_already_loaded)	return	0;
	time = cur_time;

	faces_p_s = 0;
	cur_mark_num = 0;
	num_trans_faces = 0;

	UpdateWaves();
	SetViewInfo(view_pos, angle);
	ComputeViewFrustrum();
	ClearSpanBuf();
	Clear_VS(0);
	int		sector = FindVisibleSectors(cam_pos);
	//if((!n) || (view_size<100))
	DrawDynamicEntities();	//	draw dynamic items

	//	render scene (without objects and transparent surfaces)
	mode = 0;
	MoveEnd0End1();
	VisitVisibleSectors();	//	find visible sectors
	RenderNode((int) map.models[0].headnode[0]);

	DrawStaticEntities();
	DrawObjectsAndWater();
	if(draw_mode&DRAW_VOLLIGHT)
	{
		int	i = map.numlights;
		while(i--)	DrawLightFlare(&map.lights[i], 1);
	}

	numobjects=numsprites=0;
	num_dyn_lights=0;

	color_t	c={128, 128, 128, 128};
	int		x=sx_size>>1, y=sy_size>>1;
	DrawLine(x-3, y, x+3, y, c);
	DrawLine(x, y-3, x, y+3, c);
	switch(map.sectors[sector].contents)
	{
	case	CONTENTS_WATER:
		c.a = 64;
		c.r = 64;
		c.g = c.b = 255;
		DrawBar(sx_min, sy_min, sx_max, sy_max, c);
		break;
	case	CONTENTS_LAVA:
		c.a = 192;
		c.r = 255;
		c.g = 128;
		c.b = 0;
		DrawBar(sx_min, sy_min, sx_max, sy_max, c);
		break;
	}
	/*t_scrpoint	p[16];
	p[0].sx = 200;
	p[0].sy = 20;
	p[1].sx = 313;
	p[1].sy = 215;
	p[2].sx = 87;
	p[2].sy = 215;
	p[3].sx = 200;
	p[3].sy = 20;
	p[4].sx = 313;
	p[4].sy = 215;
	p[5].sx = 87;
	p[5].sy = 215;
	DrawCatmullRomSpline(4, p, 20, color);
	color.r = 0;
	color.g = -1;
	DrawSpline(4, p, 20, color, (time&0x1F9F)/512.0f-8);
	color.b = color.g = -1;
	for(int i=0; i<32; i++)
	{
		int	x = int(200 + 130*sin(i*pi/16));
		int	y = int(150 + 130*cos(i*pi/16));
		DrawLine(x-2, y, x+2, y, color);
		DrawLine(x, y-2, x, y+2, color);
	}*/
	return	faces_p_s;
}
