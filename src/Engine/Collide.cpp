/*
	Collide.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Memory.h>

#include	"Collide.h"
#include	"Render.h"
#include	"Console.h"
#include	"Math_3D.h"

#define STOP_EPSILON    0.1f
#define	STEP	18
#define	NUM_BUMPS       4
//static hull_t		box_hull;
//static clipnode_t	box_clipnodes[6];
static plane_t		box_planes[6];
static float		physic_frametime;

int		FindSector(int num, vec3_t p)
{
	while (num >= 0)
	{
		t_node	*node = &map.nodes[num];
		num = node->children[PointPlaneDist(p, &map.planes[node->planenum])<0];
	}
	return	~num;
}

int		HullPointContents(int num, vec3_t p)
{
	while (num >= 0)
	{
		t_clipnode	*node = &map.clipnodes[num];
		num = node->children[PointPlaneDist(p, &map.planes[node->planenum])<0];
	}
	return	num;
}

void	PushEntity(entity_t * entity, vec3_t v, move_t * move)
{
	vec3_t	end;

	VectorAdd(entity->origin, v, end);
	MoveEntity(entity, move, entity->origin, end);
	VectorCopy(move->end, entity->origin);
}
void	AddGravity(entity_t * entity, float scale)
{
	entity->velocity[2] -= scale * 9.8f * physic_frametime;
}
int		MovePlayer(entity_t * entity, float time)
{
	vec3_t	oldvel, up, down, oldorigin;
	vec3_t	nosteporigin, nostepvelocity;
	move_t	move;
	int		blocked;

	entity->flags &= ~ENTITY_ON_GROUND;
	physic_frametime = time*0.001f;
	AddGravity(entity, 8.0);

	VectorCopy(entity->origin, oldorigin);
	VectorCopy(entity->velocity, oldvel);

	blocked = FlyMove(entity, physic_frametime);

	if (! (blocked & 2))	return 0;

	VectorCopy(entity->origin, nosteporigin);
	VectorCopy(entity->velocity, nostepvelocity);

	VectorCopy(oldorigin, entity->origin);

	up[0] = up[1] = 0;
	up[2] = STEP;
	// Down
	down[0] = down[1] = 0;
	down[2] = -STEP;// + oldvel[2] * physic_frametime;

	// Sale di uno scalino
	PushEntity(entity, up, &move);
	// move forward
	entity->velocity[0] = oldvel[0];
	entity->velocity[1] = oldvel[1];
	entity->velocity[2] = 0;
	blocked = FlyMove(entity, physic_frametime);
	PushEntity(entity, down, &move);
	if (move.plane.normal[2] > 0.7)
	{
		//CPrintfMessage("CIAO");
	}
	else
	{
		VectorCopy(nosteporigin, entity->origin);
		VectorCopy(nostepvelocity, entity->velocity);
	}
	return	blocked;
}

int		ClipVelocity(vec3_t in, vec3_t  normal, vec3_t  out, float overbounce)
{
	float	backoff;
	int		i, blocked;

	blocked = 0;
	if (normal[2] > 0)	blocked |= 1;           // floor
	if (! normal[2])	blocked |= 2;           // step

	backoff = DotProduct(in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		out[i] = in[i] - normal[i]*backoff;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)	out[i] = 0;
	}

	return	blocked;
}

#define	MAX_CLIP_PLANES		5

int		FlyMove(entity_t * entity, float time)
{
	int		bumpcount, i, j;
	int		blocked=0, numplanes=0;
	float	time_left=time;
	vec3_t	end, primal_velocity, original_velocity, dir, new_velocity;
	vec3_t	planes[MAX_CLIP_PLANES];
    move_t  move;

	VectorCopy(entity->velocity, original_velocity);
	VectorCopy(entity->velocity, primal_velocity);

	for (bumpcount = 0; bumpcount < NUM_BUMPS; bumpcount++)
	{
		VectorAddScale(entity->origin, entity->velocity, time_left, end);

		MoveEntity(entity, &move, entity->origin, end);

		if (move.allsolid)
		{
			VectorCopy(vec3_null, entity->velocity);
			return 3;
		}
		if (move.fraction > 0)
		{ // actually covered some distance
			VectorCopy(move.end, entity->origin);
			VectorCopy(entity->velocity, original_velocity);
			numplanes = 0;
		}
		if (move.fraction == 1)	break;

		if (move.plane.normal[2] > 0.7)
		{// floor
			blocked |= 1;
			entity->flags |= ENTITY_ON_GROUND;
		}
		if (! move.plane.normal[2])	blocked |= 2;	//	Wall/step
		time_left -= time_left * move.fraction;

		//	cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{//	this shouldn't really happen
			VectorCopy(vec3_null, entity->velocity);
			return 3;
		}
		VectorCopy(move.plane.normal, planes[numplanes]);
		numplanes++;
		//
		//	modify original_velocity so it parallels all of the clip planes
		//
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity(original_velocity, planes[i], new_velocity, 1);
			for (j=0 ; j<numplanes ; j++)
				if (j != i)
				{
					if (DotProduct(new_velocity, planes[j]) < 0)
					break;	//	not ok
				}
			if (j == numplanes)	break;
		}
		if (i != numplanes)
		{//	go along this plane
			VectorCopy(new_velocity, entity->velocity);
		} else
		{//	go along the crease
			if (numplanes != 2)
			{
				VectorCopy(vec3_null, entity->velocity);
				return 7;
			}
			CrossProduct (planes[0], planes[1], dir);
			VectorScale(dir, DotProduct (dir, entity->velocity)/DotProduct(dir, dir), entity->velocity);
		}

		if (DotProduct(entity->velocity, primal_velocity) <= 0)
		{
			VectorCopy(vec3_null, entity->velocity);
			return	blocked;
		}
	}
	return	blocked;
}

int		CheckWater(entity_t * entity)
{
	vec3_t temp;
	int		contents;

	VectorCopy(entity->origin, temp);
	temp[2] += entity->mins[2];

	contents = map.sectors[FindSector(map.models[0].headnode[0], temp)].contents;
	entity->contents = CONTENTS_EMPTY;
	entity->waterlevel = 0;

	if (contents <= CONTENTS_WATER)
	{
		entity->waterlevel = 1;
		temp[2] = entity->origin[2] + ((entity->mins[2] + entity->maxs[2])*0.5f);
		//contents = point_leaf(&temp, nodes)->contents;
		contents = map.sectors[FindSector(map.models[0].headnode[0], temp)].contents;
		if (contents <= CONTENTS_WATER)
		{
			entity->waterlevel = 2;
			temp[2] = entity->origin[2] + entity->maxs[2];
			contents = map.sectors[FindSector(map.models[0].headnode[0], temp)].contents;
			//contents = point_leaf(&temp, nodes)->contents;
			if (contents <= CONTENTS_WATER)	entity->waterlevel = 3;
		}
	}
	return	(entity->waterlevel>1);
}
int		MoveEntity(entity_t * entity, move_t * move, vec3_t start, vec3_t end)
{
	vec3_t	from, to;

	memset(move, 0, sizeof(move_t));
	move->allsolid = 1;
	move->fraction = 1;

	VectorCopy(end, move->end);
	VectorCopy(start, from);
	VectorCopy(end, to);
	CheckCollision(from, to, 0, 1, move);

	return	1;
}


static int	curhullheadnodenum;
void	CheckCollision(vec3_t from, vec3_t to, float f1, float f2, move_t * move)
{
	int		i, model;
	float	fraction;
	vec3_t	start, end, tmp;

	curhullheadnodenum = map.models[0].headnode[2];
	CheckHullCollision(curhullheadnodenum, from, to, f1, f2, move);
	for (i=0; i<map.numswalls; i++)
	{
		s_wall_t	*ent = &map.swalls[i];
		model = ent->model;
		if((model<=0) || (model>=map.nummodels))
		{
			CPrintfMessage("Error: bad model number");
			continue;
		}
		curhullheadnodenum = map.models[model].headnode[2];
		fraction = move->fraction;
		VectorCopy(from, start);
		VectorCopy(move->end, end);
		CheckHullCollision(curhullheadnodenum, start, end, 0, fraction, move);
	}
	for (i=0; i<map.numdwalls; i++)
	{
		int			*angle;
		d_wall_t	*ent = &map.dwalls[i];

		model = ent->model;
		if((model<=0) || (model>=map.nummodels))
		{
			CPrintfMessage("Error: bad model number");
			continue;
		}
		curhullheadnodenum = map.models[model].headnode[2];
		fraction = move->fraction;

		angle = (int *)ent->prev_angle;
		if(angle[0] || angle[1] || angle[2])
		{
			//	transform start vector to model basis
			VectorSubtract(from, ent->prev_pos, tmp);
			VectorSubtract(tmp, ent->turn_pos, tmp);
			QuickRotateVector(tmp, ent->prev_tmatrix);
			VectorAdd(ent->turn_pos, tmp, start);
			//	transform end vector to model basis
			VectorSubtract(move->end, ent->prev_pos, tmp);
			VectorSubtract(tmp, ent->turn_pos, tmp);
			QuickRotateVector(tmp, ent->prev_tmatrix);
			VectorAdd(ent->turn_pos, tmp, end);

			CheckHullCollision(curhullheadnodenum, start, end, 0, fraction, move);
			if (move->fraction != fraction)
			{//	if cross with hull
				//	transfrom end to world basis
				VectorSubtract(move->end, ent->turn_pos, move->end);
				TransformVector(move->end, ent->prev_tmatrix);
				VectorAdd(move->end, ent->turn_pos, move->end);
				VectorAdd(move->end, ent->prev_pos, move->end);
				//	transfrom plane to world basis
				float	dist = DotProduct(ent->turn_pos, move->plane.normal) - move->plane.dist;
				TransformVector(move->plane.normal, ent->prev_tmatrix);
				VectorAdd(ent->turn_pos, ent->prev_pos, tmp);
				move->plane.dist = DotProduct(move->plane.normal, tmp) - dist;
			}
		} else
		{
			VectorSubtract(from, ent->prev_pos, start);
			VectorSubtract(move->end, ent->prev_pos, end);
			CheckHullCollision(curhullheadnodenum, start, end, 0, fraction, move);
			if (move->fraction != fraction)
			{
				VectorAdd(move->end, ent->prev_pos, move->end);
				move->plane.dist += DotProduct(ent->prev_pos, move->plane.normal);
			}
		}
	}
}
int		CheckHullCollision (int num, vec3_t from, vec3_t to, float f1, float f2, move_t * move)
{
	int			i, nSide;
	vec3_t		mid;
	float		front, back, frac, midf;
	plane_t		*plane;
	t_clipnode	*node;

	if (num < 0)
	{
		if (num != CONTENTS_SOLID)
		{
			move->allsolid = 0;
			if (num == CONTENTS_EMPTY)
				move->inopen = 1;
			else
				move->inwater = 1;
		} else
			move->startsolid = 1;
		return	1;	//	empty
	}
	node = &map.clipnodes[num];
	plane = &map.planes[node->planenum];
	front = PointPlaneDist(from, plane);
	back = PointPlaneDist(to, plane);

	if (front >= 0 && back >= 0) 
		return	CheckHullCollision(node->children[0], from, to, f1,f2, move);

	if (front < 0 && back < 0) 
		return	CheckHullCollision(node->children[1], from, to, f1,f2, move);

	if (front < 0)	frac = (front + DIST_EPSILON)/(front-back);
	else			frac = (front - DIST_EPSILON)/(front-back);
	if (frac < 0)	frac = 0;
	if (frac > 1)	frac = 1;

	midf = f1 + (f2 - f1) * frac;
	if (midf < 0 || midf > 1)	CPrintfMessage("Error: midf %.1f", midf);
	for (i=0 ; i<3 ; i++)	mid[i] = from[i] + frac * (to[i]-from[i]);

	nSide = front < 0;

	if (! CheckHullCollision(node->children[nSide], from, mid, f1, midf, move))
		return	0;
	if (HullPointContents(node->children[nSide ^ 1], mid) != CONTENTS_SOLID)
		return	CheckHullCollision(node->children[nSide ^ 1], mid, to, midf, f2, move);

	if (move->allsolid)	return	0; // never got out of the solid area

	if (nSide == 0)
	{//	nSide vale 0 se front e' dalla parte della normale al piano
		VectorCopy(plane->normal, move->plane.normal);
		move->plane.dist = plane->dist;
	}
	else
	{//	e' dall'altra parte, occorre invertire
		VectorSubtract(vec3_null, plane->normal, move->plane.normal);
		move->plane.dist = -plane->dist;
	}

	while (HullPointContents(curhullheadnodenum, mid) == CONTENTS_SOLID)
	{ 
		frac -= 0.1f;
		if (frac < 0)
		{
			move->fraction = - 10; //midf;
			VectorCopy(mid, move->end);
			return 0;
		}
		midf = f1 + (f2 - f1) * frac;
		for (i=0;i<3;i++)	mid[i] = from[i] + (to[i]-from[i])*frac;
	}

	move->fraction = midf;
	VectorCopy(mid, move->end);
	return	0;
}

int		ImpactPlayer (entity_t * entity, vec3_t push)
{
	int     bumpcount, i, j;
	int		blocked=0, numplanes=0;
	float	fraction = 1;
	vec3_t	end, primal_velocity, original_velocity, dir, new_velocity;
	vec3_t	planes[MAX_CLIP_PLANES];
    move_t  move;

	VectorCopy(push, original_velocity);
	VectorCopy(push, primal_velocity);

	for (bumpcount = 0; bumpcount < NUM_BUMPS; bumpcount++)
	{
		VectorAddScale(entity->origin, push, fraction, end);

		MoveEntity(entity, &move, entity->origin, end);

		if (move.allsolid)  return 3;
		if (move.fraction > 0)
		{//	actually covered some distance
			VectorCopy(move.end, entity->origin);
			VectorCopy(push, original_velocity);
			numplanes = 0;
		}
		if (move.fraction == 1)	break;

		if (move.plane.normal[2] > 0.7)
		{//	floor
			blocked |= 1;
			entity->flags |= ENTITY_ON_GROUND;
		}
		if (!move.plane.normal[2])	blocked |= 2;	//	Wall/step
		fraction -= fraction*move.fraction;

		//	cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)   return 3;   //	this shouldn't really happen
		VectorCopy(move.plane.normal, planes[numplanes]);
		numplanes++;

		//	modify original_velocity so it parallels all of the clip planes
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity(original_velocity, planes[i], new_velocity, 1);
			for (j=0 ; j<numplanes ; j++)
				if (j != i)
				{
					if (DotProduct(new_velocity, planes[j]) < 0)
					break;	//	not ok
				}
			if (j == numplanes)	break;
		}
		if (i != numplanes)
		{//	go along this plane
			VectorCopy(new_velocity, push);
		} else
		{//	go along the crease
			if (numplanes != 2) return 7;
			CrossProduct (planes[0], planes[1], dir);
			VectorScale(dir, DotProduct (dir, push)/DotProduct(dir, dir), push);
		}
		if (DotProduct(push, primal_velocity) <= 0) return	blocked;
	}
	return	blocked;
}
/*void	InitBoxHull()
{
	int		i, side;

	box_hull.clipnodes = box_clipnodes;
	box_hull.planes = box_planes;
	box_hull.firstclipnode = 0;
	box_hull.lastclipnode = 5;

	for (i=0 ; i<6 ; i++)
	{
		box_clipnodes[i].planenum = i;

		side = i&1;

		box_clipnodes[i].children[side] = CONTENTS_EMPTY;
		if (i != 5)
			box_clipnodes[i].children[side^1] = i + 1;
		else
			box_clipnodes[i].children[side^1] = CONTENTS_SOLID;

		box_planes[i].type = i>>1;
		box_planes[i].normal[i>>1] = 1;
	}
}

hull_t	*HullForBox(vec3_t mins, vec3_t maxs)
{
	box_planes[0].dist = maxs[0];
	box_planes[1].dist = mins[0];
	box_planes[2].dist = maxs[2];
	box_planes[3].dist = mins[2];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = mins[1];

	return	&box_hull;
}

hull_t	*HullForEntity(entity_t *entity, vec3_t offset)
{
	hull_t	*hull;
	vec3_t	size, hullmins, hullmaxs;

	#if 0
		VectorSubtract(entity->maxs, entity->mins, size);
		if (size[0] < 3)
		{
			hull = &bspmodels[0].hulls[0];
		} else
		if (size[0] <= 32)
		{
			hull = &bspmodels[0].hulls[1];
		} else
		{
			hull = &bspmodels[0].hulls[2];
		}
		VectorSubtract(hull->clip_mins, entity->mins, offset);
		//VectorAdd((*offset), entity->origin, (*offset));
		/*VectorSubtract(entity->mins, entity->maxs, hullmins);
		VectorSubtract(entity->maxs, entity->mins, hullmaxs);
		hull = HullForBox(&hullmins, &hullmaxs);

		VectorCopy(entity->origin, offset);
		// calculate an offset value to center the origin

		offset[0] = offset[1] = offset[2] = 0;//*//*
	#endif
	return	&map.models[0].headnode[2];
}*/