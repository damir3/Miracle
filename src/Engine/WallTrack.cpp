/*
	WallTracks.cpp	Copyright (C) 1998-1999 Damir Sagidullin

	Processes tracks on walls
*/
#include	<Stdio.h>
#include	<Memory.h>

#include	"Light.h"
#include	"MapFile.h"
#include	"Math_3D.h"
#include	"Move.h"
#include	"Render.h"
#include	"Struct.h"
#include	"Walltrack.h"

static int	cur_node=-1;

void	AddWallTrack(bitmap *bm)
{
	if(cur_node>0)	map.nodes[cur_node].firstwmark = NULL;
	mgl.DrawString(10,10,0x00FFFFFF, "Node=%d (%d)", cur_node, map.numnodes);

	vec3_t	tmp, start, end;
	tmp[0] = tmp[2] = 0;
	tmp[1] = 1;
	RotateVector1(tmp, cam_angles);
	VectorCopy(cam_position, start);
	VectorAddScale(start, tmp, 512, end);

	trace_t		trace;
	memset (&trace, 0, sizeof(trace_t));
	trace.fraction = 1;
	trace.allsolid = true;
	RecursiveCheck(map.models[0].headnode[0], 0, 1, start, end, &trace);
	if(trace.fraction==1)
	{
		cur_node = -1;
		return;
	}
	int		face = map.nodes[trace.nodenum].firstface;
	int		tracknum = 0;
	map.nodes[trace.nodenum].firstwmark = &wallmarks[tracknum];
	wallmarks[tracknum].node = trace.nodenum;
	wallmarks[tracknum].tracknum = tracknum;
	wallmarks[tracknum].type = 0;
	wallmarks[tracknum].side = trace.side;
	VectorCopy(map.faces_consts[face].u, wallmarks[tracknum].u);
	VectorCopy(map.faces_consts[face].v, wallmarks[tracknum].v);
	VectorCopy(map.faces_consts[face].w, wallmarks[tracknum].w);
	wallmarks[tracknum].bm = bm;
	wallmarks[tracknum].nextwmark = NULL;
	vec3_t	p[4];
	VectorScale(wallmarks[tracknum].u, bm->width*0.5f, p[0]);
	VectorScale(wallmarks[tracknum].v, bm->height*0.5f, p[1]);
	VectorAdd(trace.endpos, p[0], p[2]);
	VectorSubtract(trace.endpos, p[0], p[3]);
	VectorAdd(p[2], p[1], wallmarks[tracknum].p[0]);
	VectorSubtract(p[2], p[1], wallmarks[tracknum].p[1]);
	VectorSubtract(p[3], p[1], wallmarks[tracknum].p[2]);
	VectorAdd(p[3], p[1], wallmarks[tracknum].p[3]);
	numwallmarks = 1;
	cur_node = trace.nodenum;
	/*mgl.DrawString(10,20,0x00FFFFFF, "Node=%d (%.3f,%.3f,%.3f)", cur_node, trace.normal[0], trace.normal[1], trace.normal[2]);
	mgl.DrawString(10,40,0xFF00FFFF, "(%.3f,%.3f,%.3f)", wallmarks[tracknum].p[0][0], wallmarks[tracknum].p[0][1], wallmarks[tracknum].p[0][2]);
	mgl.DrawString(10,50,0xFF00FFFF, "(%.3f,%.3f,%.3f)", wallmarks[tracknum].p[1][0], wallmarks[tracknum].p[1][1], wallmarks[tracknum].p[1][2]);
	mgl.DrawString(10,60,0xFF00FFFF, "(%.3f,%.3f,%.3f)", wallmarks[tracknum].p[2][0], wallmarks[tracknum].p[2][1], wallmarks[tracknum].p[2][2]);
	mgl.DrawString(10,70,0xFF00FFFF, "(%.3f,%.3f,%.3f)", wallmarks[tracknum].p[3][0], wallmarks[tracknum].p[3][1], wallmarks[tracknum].p[3][2]);*/
}
/*float	min = fabs(trace.plane.normal[0]);
	for(j=0, i=1; i<3; i++)
	{
		if(min>fabs(trace.plane.normal[i]))
		{
			j = i;
			min = fabs(trace.plane.normal[i]);
		}
	}
	memset(tmp, 0, sizeof(vec3_t));
	tmp[j] = 1;
	ComputeNormal((vector *)u, (vector *)trace.plane.normal, (vector *)tmp);
	ComputeNormal((vector *)v, (vector *)trace.plane.normal, (vector *)u);*/