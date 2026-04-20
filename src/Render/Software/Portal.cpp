/*
	Portal.cpp
*/
#include	"DrawEnt.h"
#include	"DrawFace.h"
#include	"Math_3D.h"
#include	"Portal.h"
#include	"Render.h"
#include	"SpanBuf.h"

int		mirror=0, portal=0;
int		num_clip_planes;
t_plane	clip_planes[64];

extern char	vis_face1[MAX_MAP_FACES/8+1], vis_face2[MAX_MAP_FACES/8+1];
extern char	*vis_face;
extern char	vis_node1[MAX_MAP_NODES], vis_node2[MAX_MAP_NODES];
extern char	*vis_node;

extern int time;

void	AddClipPlane(t_plane *plane)
{
	clip_planes[num_clip_planes] = *plane;
	clip_planes[num_clip_planes++].type = 4;
}

void	DrawPortal (int face)
{
	int		i, j, edge;
	int		firstedge = map.faces[face].firstedge;
	int		numedges = map.faces[face].numedges;
	int		codes_and=0xff;
	int		num_prt = map.faces[face].texinfo;
	int		cur_mode, target;
	vec3_t	a, b, rotate_ang, sub;
	t_plane	clip_plane;
	float	*v;
	point_3d	portal_points[64];

	if((num_prt<0) || (num_prt>=map.num_i_portals))
	{
		gi.CPrintf("DrawPortal(): Error portal number");
		return;
	}
	if(mirror || portal)	return;

	for (i=0; i<numedges; i++)
	{
		edge = map.surf_edges[firstedge+i];
		if (edge < 0)	v = &map.vertices[map.edges[-edge].v[1]].point[0];
		else			v = &map.vertices[map.edges[ edge].v[0]].point[0];
		TransformPointRaw(portal_points[i].p, v);
		CodePoint(&portal_points[i]);
		codes_and &= portal_points[i].ccodes;
	}
	if(codes_and)	return;

	num_clip_planes = 0;
	for(i=0, j=numedges-1; i<numedges; j=i++)
	{//	skip if portal edge goes off screen
		if((portal_points[i].ccodes & portal_points[j].ccodes) & CC_BEHIND)	continue;
		CrossProduct (portal_points[j].p, portal_points[i].p, clip_plane.normal);
		clip_plane.dist = 0;
		AddClipPlane(&clip_plane);
	}
	//	save previous camera parameters
	SaveViewInfo();
	cur_mode = mode;
	//	set new camera parameters
	target = map.i_portals[num_prt].targetnum;
	if((target>=0) && (target<map.num_i_portals))
	{
		VectorSubtract (portal_points[3].p, portal_points[2].p, a);
		VectorSubtract (portal_points[2].p, portal_points[1].p, b);
		CrossProduct (b, a, clip_plane.normal);
		clip_plane.dist = DotProduct (portal_points[0].p, clip_plane.normal);
		AddClipPlane(&clip_plane);

		VectorSubtract (cam_pos, map.i_portals[num_prt].pos, sub);
		VectorSubtract (map.i_portals[target].angle, map.i_portals[num_prt].angle, rotate_ang);
		RotateVector(sub, rotate_ang);
		VectorAdd (map.i_portals[target].pos, sub, cam_pos);
		RotateVector(main_matrix[0], rotate_ang);
		RotateVector(main_matrix[1], rotate_ang);
		RotateVector(main_matrix[2], rotate_ang);
		ComputeViewMatrix();
		ComputeViewFrustrum();
		num_prt = target;
	} else
	{
		VectorCopy (map.i_portals[num_prt].pos, cam_pos);
		VectorAdd (rotate_ang, map.i_portals[num_prt].angle, rotate_ang);
	}

	portal = 1;
	first_num_trans_faces = num_trans_faces;
	char	*temp_vis_sectors = vis_sectors;
	FindVisibleSectors(map.i_portals[num_prt].pos);
	vis_face = vis_face2;
	vis_node = vis_node2;
	SaveSpanBuf();

	DrawDynamicEntities();

	VisitVisibleSectors();
	RenderNode((int) map.models[0].headnode[0]);

	DrawStaticEntities();
	DrawObjectsAndWater();

	num_trans_faces = first_num_trans_faces;
	first_num_trans_faces = 0;
	RestoreSpanBuf();
	vis_face = vis_face1;
	vis_node = vis_node1;
	vis_sectors = temp_vis_sectors;
	portal = 0;

	RestoreViewInfo();
	mode = cur_mode;
}
void	DrawMirror(int face)
{
	int		firstedge = map.faces[face].firstedge;
	int		numedges = map.faces[face].numedges;
	int		codes_and=0xff;
	int		cur_mode = mode;
	t_plane	*plane = &map.planes[map.faces[face].planenum];
	t_plane	clip_plane;
	int		i, j, edge;
	float	dist, *v;
	vec3_t	a, b, center;
	point_3d	portal_points[64];

	if(mirror || portal)	return;
	//	save previous camera parameters
	SaveViewInfo();
	//	set new camera parameters
	VectorAddScale (cam_pos, plane->normal, -2*DistToCamera(plane), cam_pos);
	for(i=0; i<3; i++)
	{
		dist = DotProduct (main_matrix[i], plane->normal);
		VectorAddScale (main_matrix[i], plane->normal, -2*dist, main_matrix[i]);
	}
	VectorInverse (main_matrix[0]);
	ComputeViewMatrix();
	ComputeViewFrustrum();

	VectorCopy (vec3_null, center);
	for (i=0; i<numedges; i++)
	{
		edge = map.surf_edges[firstedge+i];
		if (edge < 0)	v = &map.vertices[map.edges[-edge].v[1]].point[0];
		else			v = &map.vertices[map.edges[ edge].v[0]].point[0];
		TransformPointRaw(portal_points[i].p, v);
		CodePoint(&portal_points[i]);
		codes_and &= portal_points[i].ccodes;
		VectorAdd (center, v, center);
	}
	if(!codes_and)
	{
		mirror = 1;
		first_num_trans_faces = num_trans_faces;
		char	*temp_vis_sectors = vis_sectors;

		float	dist_epsilon, temp = 1.0f/numedges;
		if(map.faces[face].side)	dist_epsilon = -DIST_EPSILON;
		else	dist_epsilon = DIST_EPSILON;
		center[0] = (center[0]*temp) + (plane->normal[0]*dist_epsilon);
		center[1] = (center[1]*temp) + (plane->normal[1]*dist_epsilon);
		center[2] = (center[2]*temp) + (plane->normal[2]*dist_epsilon);
		FindVisibleSectors(center);
		vis_face = vis_face2;
		vis_node = vis_node2;
		SaveSpanBuf();
		num_clip_planes = 0;
		for(i=0, j=numedges-1; i<numedges; j=i++)
		{//	skip if portal edge goes off screen
			if((portal_points[i].ccodes & portal_points[j].ccodes) & CC_BEHIND)	continue;
			CrossProduct (portal_points[i].p, portal_points[j].p, clip_plane.normal);
			clip_plane.dist = 0;
			AddClipPlane(&clip_plane);
		}
		VectorSubtract (portal_points[3].p, portal_points[2].p, a);
		VectorSubtract (portal_points[2].p, portal_points[1].p, b);
		CrossProduct (a, b, clip_plane.normal);
		clip_plane.dist = DotProduct (portal_points[0].p, clip_plane.normal);
		AddClipPlane(&clip_plane);

		DrawDynamicEntities();

		VisitVisibleSectors();
		RenderNode((int) map.models[0].headnode[0]);

		DrawStaticEntities();
		DrawObjectsAndWater();
		//	restore previous camera parameters
		num_trans_faces = first_num_trans_faces;
		first_num_trans_faces = 0;
		RestoreSpanBuf();
		vis_face = vis_face1;
		vis_node = vis_node1;
		vis_sectors = temp_vis_sectors;
		mirror = 0;
	}
	RestoreViewInfo();
	mode = cur_mode;
}
extern int		cur_ent;
extern double	proj_scale_x, proj_scale_y;
void	DrawDynamicMirror(int face)
{
	int		firstedge = map.faces[face].firstedge;
	int		numedges = map.faces[face].numedges;
	int		codes_and=0xff;
	int		cur_mode = mode;
	t_plane	*plane=&map.planes[map.faces[face].planenum];
	t_plane	clip_plane;
	int		i, j, edge;
	float	dist;
	vec3_t	a, b, center, vec3_temp;
	point_3d	portal_points[64];

	if(mirror || portal)	return;
	d_wall_t	*ent = &map.dwalls[cur_ent];
	//	save previous camera parameters
	SaveViewInfo();
	//	set new camera parameters
	VectorAddScale (cam_pos, plane->normal, -2*DistToCamera(plane), cam_pos);
	for(i=0; i<3; i++)
	{
		dist = DotProduct (main_matrix[i], plane->normal);
		VectorAddScale (main_matrix[i], plane->normal, -2*dist, main_matrix[i]);
	}
	VectorInverse (main_matrix[0]);

	ComputeViewMatrix();
	//	
	VectorCopy (vec3_null, center);
	for (i=0; i<numedges; i++)
	{
		edge = map.surf_edges[firstedge+i];
		float	*v;
		if (edge < 0)	v = &map.vertices[map.edges[-edge].v[1]].point[0];
		else			v = &map.vertices[map.edges[ edge].v[0]].point[0];
		TransformPointRaw(portal_points[i].p, v);
		CodePoint(&portal_points[i]);
		codes_and &= portal_points[i].ccodes;
		VectorAdd (center, v, center);
	}
	if(!codes_and)
	{
		mirror = 1;
		first_num_trans_faces = num_trans_faces;
		char	*temp_vis_sectors = vis_sectors;

		float	dist_epsilon, temp = 1.0f/numedges;
		if(map.faces[face].side)	dist_epsilon = -DIST_EPSILON;
		else	dist_epsilon = DIST_EPSILON;
		center[0] = (center[0]*temp) + (plane->normal[0]*dist_epsilon);
		center[1] = (center[1]*temp) + (plane->normal[1]*dist_epsilon);
		center[2] = (center[2]*temp) + (plane->normal[2]*dist_epsilon);
		VectorAdd (center, ent->pos, center);
		num_clip_planes = 0;
		for(i=0, j=numedges-1; i<numedges; j=i++)
		{//	skip if portal edge goes off screen
			if((portal_points[i].ccodes&portal_points[j].ccodes)&CC_BEHIND)	continue;
			CrossProduct (portal_points[i].p, portal_points[j].p, clip_plane.normal);
			clip_plane.dist = 0;
			AddClipPlane(&clip_plane);
		}
		VectorSubtract (portal_points[3].p, portal_points[2].p, a);
		VectorSubtract (portal_points[2].p, portal_points[1].p, b);
		CrossProduct (a, b, clip_plane.normal);
		clip_plane.dist = DotProduct (portal_points[0].p, clip_plane.normal) + float(proj_scale_y*proj_scale_x*96);
		AddClipPlane(&clip_plane);

		t_plane	transform_plane;
		VectorCopy (plane->normal, transform_plane.normal);
		dist = PointPlaneDist(ent->turn_pos, plane);
		QuickTransformVector(transform_plane.normal, ent->tmatrix);
		VectorAdd (ent->turn_pos, ent->pos, vec3_temp);
		transform_plane.dist = DotProduct (transform_plane.normal, vec3_temp) - dist;
		transform_plane.type = 4;

		RestorePreviousViewInfo(1);

		VectorAddScale (cam_pos, transform_plane.normal, -2*DistToCamera(&transform_plane), cam_pos);
		for(i=0; i<3; i++)
		{
			dist = DotProduct (main_matrix[i], transform_plane.normal);
			VectorAddScale (main_matrix[i], transform_plane.normal, -2*dist, main_matrix[i]);
		}
		VectorInverse (main_matrix[0]);
		ComputeViewMatrix();
		ComputeViewFrustrum();

		FindVisibleSectors(center);
		vis_face = vis_face2;
		vis_node = vis_node2;
		SaveSpanBuf();

		DrawDynamicEntities();

		VisitVisibleSectors();
		RenderNode((int) map.models[0].headnode[0]);

		DrawStaticEntities();
		DrawObjectsAndWater();
		//	restore previous camera parameters
		num_trans_faces = first_num_trans_faces;
		first_num_trans_faces = 0;
		RestoreSpanBuf();
		vis_face = vis_face1;
		vis_node = vis_node1;
		vis_sectors = temp_vis_sectors;
		mirror = 0;
	}
	RestoreViewInfo();
	mode = cur_mode;
}
static point_3d	*point_list1[64];
static point_3d	*point_list2[64];
static point_3d	some_points[256];

int		ClipPortalPoly(int n, point_3d **vl, point_3d ***out_vl)
{
	int		i, j, k;
	int		num, p=0;
	point_3d	**cur=point_list1, *a, *b;
	for(k=0; k<num_clip_planes; k++)
	{
		num = 0;
		for(i=0; i<n; i++)
		{
			vl[i]->dist = PointPlaneDist(vl[i]->p, &clip_planes[k]);
			if(vl[i]->dist>0)	num++;
		}
		if(num==n)	return	0;
		if(!num)	continue;

		for(num=i=0, j=n-1; i<n; j=i++)
		{
			a = vl[j];
			b = vl[i];
			if(a->dist<=0) cur[num++] = a;
			if((a->dist>0) ^ (b->dist>0))
			{
				where = b->dist / (b->dist-a->dist);
				CrossPoint(&some_points[p], b, a);
				cur[num++] = &some_points[p++];
			}
		}
		if(!num)	return	0;
		n = num;
		vl = cur;
		cur = (cur == point_list1) ? point_list2 : point_list1;
	}
	*out_vl = vl;
	return	n;
}
