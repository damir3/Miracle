/*
	Move.cpp	Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdio.h>
#include	<memory.h>
#include	<math.h>

#include	"console.h"
#include	"mapfile.h"
#include	"render.h"
#include	"math_3d.h"
#include	"move.h"
#include	"light.h"
#include	"collide.h"

#define		MIN_FOV		1.0f
#define		MAX_FOV		170.0f

entity_t	entity;
char	move_forward=0, move_back=0;
char	move_right=0, move_left=0;
char	move_up=0, move_down=0;
char	jump=0, zoom=0;
char	turn_z_0 = 0, turn_z_1 = 0;
char	turn_x_0 = 0, turn_x_1 = 0;
char	turn_y_0 = 0, turn_y_1 = 0;
float	cur_fov = 90, zoomfov = 22.5f, zoomdist = 192;
vec3_t	cam_position, cam_angles;
char	can_move=1, move_type=NORMAL_MOVE;
float	forward_speed=64, back_speed=64, side_speed=64, max_speed = 256;
int		turn_speed=0x3000;//, ddtime=0;

extern float	fov;
extern float	delta_z;

#define		MAX_CLIP_PLANES	16

static int		num_clip_planes;
static plane_t	clip_planes[MAX_CLIP_PLANES];

void	ClearMoveStatus()
{
	move_forward=move_back=0;
	move_right=move_left=0;
	move_up=move_down=0;
	jump=0;
	turn_x_0 = turn_x_1 = 0;
	turn_y_0 = turn_y_1 = 0;
	turn_z_0 = turn_z_1 = 0;
}
void	FindZoom(int time)
{
	cur_fov = mgl.Getf(FOV);
	switch(zoom)
	{
	case	0:	//	нет zoom'а
		cur_fov = float(fov + (cur_fov-fov)*pow(0.5, time/64.0));
		break;
	case	1:	//	нормальный zoom
		cur_fov = float(zoomfov + (cur_fov-zoomfov)*pow(0.5, time/64.0));
		break;
	case	2:	//	автоматический zoom
		vec3_t	a, b;
		trace_t	trace;
		float	k, new_fov;

		VectorCopy(cam_position, a);
		b[0] = b[2] = 0;
		b[1] = 1;
		RotateVector1(b, cam_angles);
		VectorAddScale(a, b, 4096, b);
		memset (&trace, 0, sizeof(trace_t));
		trace.fraction = 1;
		trace.allsolid = true;
		RecursiveCheck(map.models[0].headnode[0], 0, 1, a, b, &trace);
		VectorSubtract(trace.endpos, a, b);
		k = zoomdist/VectorLength(b);
		if(k>1)	k = 1;
		new_fov = 2*float(180*atan(k)/pi);
		if(new_fov<MIN_FOV)	new_fov = MIN_FOV;
		cur_fov = float(new_fov + (cur_fov-new_fov)*pow(0.5, time/64.0));
		break;
	}
	mgl.Setf(FOV, cur_fov);
	zoom=0;
}
void	Move(int time_ms)
{
	//	проверка на нахождение в воде
	vec3_t	new_velocity, rotate_angle;
	float	time_sec = time_ms*0.001f;
	int		water = CheckWater(&entity);//InWater(pos);
	//mgl.DrawString(10,10, 0xFFFFFFFF, "WaterLevel=%d", entity.waterlevel);
	VectorCopy(vec3_null, new_velocity);
	//	проверка на нахождение в воде
	if(can_move)
	{
		if(move_right)		new_velocity[0] += side_speed;
		if(move_forward)	new_velocity[1] += forward_speed;
		if(move_up)			new_velocity[2] += side_speed;
		if(move_left)		new_velocity[0] -= side_speed;
		if(move_back)		new_velocity[1] -= back_speed;
		if(move_down)		new_velocity[2] -= side_speed;
		if(turn_x_0)	entity.angles[0] -= int(turn_speed*time_sec);
		if(turn_x_1)	entity.angles[0] += int(turn_speed*time_sec);
		if(turn_y_0)	entity.angles[1] += int(turn_speed*time_sec);
		if(turn_y_1)	entity.angles[1] -= int(turn_speed*time_sec);
		if(turn_z_0)	entity.angles[2] += int(turn_speed*time_sec);
		if(turn_z_1)	entity.angles[2] -= int(turn_speed*time_sec);
	}
	VectorCopy(entity.angles, rotate_angle);
	if((move_type&NORMAL_MOVE) && !water)
		rotate_angle[0] = rotate_angle[1] = 0;
	RotateVector1(new_velocity, rotate_angle);
	if(move_type&NORMAL_MOVE)
	{//	walk
		if((entity.flags&ENTITY_ON_GROUND))
		{
			VectorAddScale(new_velocity, entity.velocity, float(pow(0.5, time_ms/32.0)), entity.velocity);
			if(can_move && jump)	entity.velocity[2] += side_speed*2;
		} else
		{
			if(water)
			{
				VectorAddScale(new_velocity, entity.velocity, float(pow(0.5, time_ms/32.0)), entity.velocity);
				if(can_move && jump)	entity.velocity[2] += side_speed*2;
			} else
				VectorAddScale(entity.velocity, new_velocity, 0.1f, entity.velocity);
		}
		//entity.anim_time = time_ms*(4.0f - entity.waterlevel);
		MovePlayer(&entity, time_ms*(4.0f - entity.waterlevel));
	} else
	if(move_type&FLY_MOVE)
	{//	fly
		VectorCopy(new_velocity, entity.velocity);
		FlyMove(&entity, time_sec*4.0f);
	} else
	{//	no clip
		VectorCopy(new_velocity, entity.velocity);
		VectorAddScale(entity.origin, entity.velocity,  time_sec*4.0f, entity.origin);
	}
	VectorCopy(entity.origin, entity.old_origin);
	ClearMoveStatus();
}
void	ChangeFov()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%f", &fov);
		if(fov<MIN_FOV)	fov = MIN_FOV;
		if(fov>MAX_FOV)	fov = MAX_FOV;
	}
	CPrintf("Fov is \"%.2f\"", fov);
}
void	ChangeZoomFov()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%f", &zoomfov);
		if(zoomfov<MIN_FOV)	zoomfov = MIN_FOV;
		if(zoomfov>MAX_FOV)	zoomfov = MAX_FOV;
	}
	CPrintf("Zoom Fov is \"%.2f\"", zoomfov);
}
void	ChangeZoomDist()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%f", &zoomdist);
		if(zoomdist<8)	zoomdist = 8;
		//if(zoomfov>170)	zoomfov = 170;
	}
	CPrintf("Zoom Distance is \"%.2f\"", zoomdist);
}
void	ChangeForwardSpeed()
{
	if(num_cargs>1)
		sscanf(cargs[1], "%f", &forward_speed);
	CPrintf("Forward speed is \"%.2f\"", forward_speed);
}
void	ChangeBackSpeed()
{
	if(num_cargs>1)
		sscanf(cargs[1], "%f", &back_speed);
	CPrintf("Back speed is \"%.2f\"", back_speed);
}
void	ChangeSideSpeed()
{
	if(num_cargs>1)
		sscanf(cargs[1], "%f", &side_speed);
	CPrintf("Side speed is \"%.2f\"", side_speed);
}
void	ChangeTurnSpeed()
{
	if(num_cargs>1)
		sscanf(cargs[1], "%d", &turn_speed);
	CPrintf("Turn speed is \"%d\"", turn_speed);
}
void	ChangePosition()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%f %f %f", &entity.origin[0], &entity.origin[1], &entity.origin[2]);
		VectorCopy(entity.origin, entity.old_origin);
	}
	CPrintf("Player position: (%.2f, %.2f, %.2f)", entity.origin[0], entity.origin[1], entity.origin[2]);
}
void	ChangeAngles()
{
	if(num_cargs>1)
	{
		sscanf(cargs[1], "%f %f %f", &entity.angles[0], &entity.angles[1], &entity.angles[2]);
		float	koef = 0x8000/180.0f;
		entity.angles[0] *= koef;
		entity.angles[1] *= koef;
		entity.angles[2] *= koef;
	}
	float	koef = 180.0f/0x8000;
	int	t[3];
	t[0] = ((int(entity.angles[0]*koef)%360)+360)%360;
	t[1] = ((int(entity.angles[1]*koef)%360)+360)%360;
	t[2] = ((int(entity.angles[2]*koef)%360)+360)%360;
	CPrintf("Turn angles: (%d, %d, %d)", t[0], t[1], t[2]);
}
void	Walk()
{
	move_type = NORMAL_MOVE;
}
void	NoClip()
{
	if(move_type&NOCLIP_MOVE)
	{
		move_type = NORMAL_MOVE;
		CPrintf("No clip - off");
	} else
	{
		move_type = NOCLIP_MOVE;
		CPrintf("No clip - on");
	}
}
void	Fly()
{
	if(move_type&FLY_MOVE)
	{
		move_type = NORMAL_MOVE;
		CPrintf("Fly - off");
	} else
	{
		move_type = FLY_MOVE;
		CPrintf("Fly - on");
	}
}
void	MoveForward()
{
	move_forward = 1;
}
void	MoveBack()
{
	move_back = 1;
}
void	MoveRight()
{
	move_right = 1;
}
void	MoveLeft()
{
	move_left = 1;
}
void	MoveUp()
{
	move_up = 1;
}
void	MoveDown()
{
	move_down = 1;
}
extern char	cur_key_state;
void	Jump()
{
	jump = 1;
	if(entity.waterlevel<=1)	cur_key_state = 2;
}
void	TurnX0()
{	//	up
	turn_x_0 = 1;
}
void	TurnX1()
{	//	down
	turn_x_1 = 1;
}
void	TurnZ0()
{	//	right
	turn_z_0 = 1;
}
void	TurnZ1()
{	//	left
	turn_z_1 = 1;
}
void	TurnY0()
{
	turn_y_0 = 1;
}
void	TurnY1()
{
	turn_y_1 = 1;
}
void	Zoom()
{
	zoom = 1;
}
void	AutoZoom()
{
	zoom = 2;
}
void	StopMove()
{
	if(can_move)
	{
		VectorCopy(vec3_null, entity.velocity);
		entity.angles[0] = entity.angles[1] = 0;
	}
}

int		CheckPlatform(int model, vec3_t	pos)
{
	vec3_t		start, end;
	move_t	move;

	memset(&move, 0, sizeof(move_t));
	move.allsolid = 1;
	move.fraction = 1;
	VectorCopy(pos, start);
	VectorCopy(pos, end);
	end[2] -= 1;

	CheckHullCollision(map.models[model].headnode[2], start, end, 0, 1, &move);
	return	(move.fraction<1)&(move.plane.normal[2]>0.7f);
}

void	CheckEntitiesMove()
{
	vec3_t	start_l, end_l, pos, prev_pos, old_pos, push;
	move_t	move;
	int		i, model;

	if(move_type&NOCLIP_MOVE)	return;

	for(i=0; i<map.numdwalls; i++)
	{
		model = map.dwalls[i].model;
		if((model<=0) || (model>=map.nummodels))	continue;
		VectorCopy(map.dwalls[i].pos, pos);
		VectorCopy(map.dwalls[i].prev_pos, prev_pos);
		if((pos[0]==prev_pos[0]) && (pos[1]==prev_pos[1]) && (pos[2]==prev_pos[2]))	continue;
		VectorCopy(entity.origin, old_pos);
		VectorSubtract(entity.origin, prev_pos, start_l);

		if(CheckPlatform(model, start_l))
		{
			VectorCopy(map.dwalls[i].pos, map.dwalls[i].prev_pos);
			VectorSubtract(pos, prev_pos, push);
			ImpactPlayer(&entity, push);
			continue;
		}
		memset(&move, 0, sizeof(move_t));
		move.allsolid = true;
		VectorSubtract(entity.origin, pos, end_l);
		do
		{
			move.fraction = 1;
			CheckHullCollision(map.models[model].headnode[2], start_l, end_l, 0, 1, &move);
			if (move.fraction != 1)
			{
				if(move.plane.normal[2]<0.7f)
				{
					float	dist = DotProduct(move.plane.normal, end_l)-move.plane.dist-DIST_EPSILON;
					VectorCopy(move.end, start_l);
					VectorScale(move.plane.normal, -dist, push);
					VectorAdd(entity.origin, push, end_l);
					VectorSubtract(end_l, pos, end_l);
					ImpactPlayer(&entity, push);
					continue;
				} else
				{
					VectorSubtract(pos, prev_pos, push);
					ImpactPlayer(&entity, push);
					VectorCopy(map.dwalls[i].pos, map.dwalls[i].prev_pos);
				}
			}
		} while(0);
	}
	for(i=0; i<map.numdwalls; i++)
	{
		VectorCopy(map.dwalls[i].angle, map.dwalls[i].prev_angle);
		VectorCopy(map.dwalls[i].pos, map.dwalls[i].prev_pos);
		memcpy(map.dwalls[i].prev_tmatrix, map.dwalls[i].tmatrix, sizeof(map.dwalls[i].tmatrix));
	}
}
