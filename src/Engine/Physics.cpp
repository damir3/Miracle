/*
	Physics.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Windows.h>
#include	<Math.h>

#include	"Console.h"
#include	"MapFile.h"
#include	"Entity.h"
#include	"Math_3D.h"
#include	"Render.h"	//	delete
#include	"Move.h"

void	FindFirstCommand(d_wall_t *ent, int time)
{
	t_action	*action = (t_action *)&ent->actions[ent->cur_action];
	if(action->type[0]==1)	ent->action_time_end = time + action->d_time;
	ent->tmatrix[0][0] = ent->tmatrix[1][1] = ent->tmatrix[2][2] = 1;
	ent->prev_tmatrix[0][0] = ent->prev_tmatrix[1][1] = ent->prev_tmatrix[2][2] = 1;
}

void	FindNextCommand(d_wall_t *ent, int time)
{
repeat0:
	t_action	*action = (t_action *)&ent->actions[ent->cur_action];
	if(ent->direction)
	{
		if(ent->cur_action > 0)
			ent->cur_action -= action->prev_size;
		else
			ent->direction = 0;
	} else
	{
		if(ent->cur_action<ent->end_action)
			ent->cur_action += action->size;
	}
repeat1:
	action = (t_action *)&ent->actions[ent->cur_action];
	if(action->type[0]==1)
		ent->action_time_end = time + action->d_time;
	else
	if(action->type[0]==2)
	{
		if(ent->direction)	goto	repeat0;
	} else
	if(action->type[0]==3)
	{
		if(!action->type[1])
		{
			if(ent->cur_action>0)
			{
				ent->cur_action = 0;
				VectorCopy(vec3_null, ent->begin_pos);
				VectorCopy(vec3_null, ent->pos);
				VectorCopy(vec3_null, ent->begin_angle);
				VectorCopy(vec3_null, ent->angle);
				goto	repeat1;
			} else
			{
				ent->actions = NULL;
				return;
			}
		}
		if(action->type[1]==1)
		{
			if(ent->cur_action>0)
			{
				ent->direction = 1;
				goto	repeat0;
			} else
			{
				ent->actions = NULL;
				return;
			}
		}
	} else	ent->actions = NULL;
}
int		Touch(int num_ent)
{
	d_wall_t	*ent = &map.dwalls[num_ent];
	float	dist = ent->dist;
	if((ent->model>0) && (ent->model<map.nummodels))
	{
		model_t	*model = &map.models[ent->model];
		if(	(entity.origin[0] > model->mins[0]-dist) &&
			(entity.origin[0] < model->maxs[0]+dist) &&
			(entity.origin[1] > model->mins[1]-dist) &&
			(entity.origin[1] < model->maxs[1]+dist) &&
			(entity.origin[2] > model->mins[2]-dist) &&
			(entity.origin[2] < model->maxs[2]+dist))
			return	1;
	}
	return	0;
}
void	FindEntityState(int num_ent, int time)
{
	d_wall_t	*ent = &map.dwalls[num_ent];
	if(ent->actions==NULL)	return;
	float	dist = ent->dist;
	if(ent->direction)
	{
		t_action	*action = (t_action *)&ent->actions[ent->cur_action];
		if(action->type[0]==1)
		{
			if(Touch(num_ent))
			{
				ent->direction = 0;
				ent->action_time_end = (time<<1) - ent->action_time_end + action->d_time;
				if(action->type[1]==1)
					VectorSubtract(ent->begin_pos, action->p, ent->begin_pos);
				if(action->type[1]==2)
					VectorSubtract(ent->begin_angle, action->p, ent->begin_angle);
			}
		}
	}
	do
	{
		if(ent->actions==NULL)	return;
		t_action	*action = (t_action *)&ent->actions[ent->cur_action];
		if((action->type[0]==2) && (action->type[1]==1))
		{
			if(Touch(num_ent))
			{
				FindNextCommand(ent, time);
				continue;
			}
		}
		if(action->type[0]==1)
		{
			if(time<=ent->action_time_end)	break;
			if(!action->type[1])
			{
				FindNextCommand(ent, ent->action_time_end);
				continue;
			} else
			if(action->type[1]&1)
			{//	move
				if(ent->direction)
					VectorSubtract(ent->begin_pos, action->p, ent->begin_pos);
				else
					VectorAdd(ent->begin_pos, action->p, ent->begin_pos);
				VectorCopy(ent->begin_pos, ent->pos);
				if(action->type[1]&2)
				{//	rotate
					float	*da = action->p + 3;
					if(ent->direction)
						VectorSubtract(ent->begin_angle, da, ent->begin_angle);
					else
						VectorAdd(ent->begin_angle, da, ent->begin_angle);
					VectorCopy(ent->begin_angle, ent->angle);
				}
				FindNextCommand(ent, ent->action_time_end);
				continue;
			} else
			if(action->type[1]&2)
			{//	rotate
				if(ent->direction)
					VectorSubtract(ent->begin_angle, action->p, ent->begin_angle);
				else
					VectorAdd(ent->begin_angle, action->p, ent->begin_angle);
				VectorCopy(ent->begin_angle, ent->angle);
				FindNextCommand(ent, ent->action_time_end);
				continue;
			}
		}
	} while(0);
	if(ent->actions==NULL)	return;
	t_action	*action = (t_action *)&ent->actions[ent->cur_action];
	if(action->type[0]==1)
	{
		float	temp = 1.0f - float(ent->action_time_end - time)/float(action->d_time);
		if(ent->direction)	temp = -temp;
		if(action->type[1]&1)
		{
			ent->pos[0] = ent->begin_pos[0] + action->p[0]*temp;
			ent->pos[1] = ent->begin_pos[1] + action->p[1]*temp;
			ent->pos[2] = ent->begin_pos[2] + action->p[2]*temp;
			if(action->type[1]&2)
			{
				ent->angle[0] = ent->begin_angle[0] + action->p[3]*temp;
				ent->angle[1] = ent->begin_angle[1] + action->p[4]*temp;
				ent->angle[2] = ent->begin_angle[2] + action->p[5]*temp;
			}
		} else
		if(action->type[1]&2)
		{
			ent->angle[0] = ent->begin_angle[0] + action->p[0]*temp;
			ent->angle[1] = ent->begin_angle[1] + action->p[1]*temp;
			ent->angle[2] = ent->begin_angle[2] + action->p[2]*temp;
		}
	}
}

void	FindEntitiesState(int time)
{//	find state of functional items
	for(int i=0; i<map.numdwalls; i++)
	{
		FindEntityState(i, time);
		d_wall_t	*ent = &map.dwalls[i];
		memset(ent->tmatrix, 0,sizeof(ent->tmatrix));
		ent->tmatrix[0][0] = ent->tmatrix[1][1] = ent->tmatrix[2][2] = 1;
		RotateVector(ent->tmatrix[0], ent->angle);
		RotateVector(ent->tmatrix[1], ent->angle);
		RotateVector(ent->tmatrix[2], ent->angle);
	}
}
