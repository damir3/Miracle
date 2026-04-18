/*
	Entity.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdio.h>
#include	<Memory.h>
#include	<String.h>
#include	<Math.h>

#include	"Console.h"
#include	"MapFile.h"
#include	"Entity.h"
#include	"Math_3D.h"
#include	"Move.h"
#include	"Render.h"
#include	"Physics.h"
#include	"Video.h"

#define	MAX_NUM_ARGUMENTS	256

int		numargs;
char	*args	[MAX_NUM_ARGUMENTS][2];

static vec3_t	mins, maxs;
static char		*cur_action_script;

void	FindSectors(int n);
int		FindNextEntCommand();

int		ExtractAllArguments(char *src, int size)
{
	int		i=0, j=0, k=0;
	char	c;

	while(size-->0)
	{
		c = *src;
		if(c=='\n')	k=0;
		if(c=='"')
		{
			if(!j)
			{//	начало
				j=1;
				if(k<2)
				{//	кол-во слов в строке < 2
					if(k==0)
					{
						args[i][0] = args[i][1] = src+1;
					} else
					{
						args[i][1] = src+1;
						i++;
					}
					if(i>=MAX_NUM_ARGUMENTS)	break;
				}
				k++;
			} else
			{//	конец
				j=0;
				*src=0;
			}
		}
		src++;
	}
	numargs = i;
	return	i;
}
int		FindBrackets(char *src, int *begin, int *size, int end)
{
	int		br_open=1, x=*begin;
	while((src[x]!='{')&&(x<end))	x++;
	if(x>=end)	return 0;
	*begin = x++;
	for(;(br_open>0)&&(x<end); x++)
	{
		if (src[x] == '{')	br_open++;
		if (src[x] == '}')	br_open--;
	}
	if(x>=end)	return 0;
	*size = x - *begin;
	return	1;
}

void	LoadWall()
{
	int		i, modelnum=0;
	model_t	*model;
	for(i=0; i<numargs; i++)
	{
		if(!strcmp(args[i][0],"model"))
		{
			sscanf(args[i][1], "*%d", &modelnum);
			break;
		}
	}
	//CPrintf("Model[%d]:", modelnum);
	if((modelnum<=0) || (modelnum>=map.nummodels))	return;
	model =  &map.models[modelnum];
	memcpy(mins, model->mins, sizeof(mins));
	memcpy(maxs, model->maxs, sizeof(maxs));
	for(i=0; i<numargs; i++)
		if(!strcmp(args[i][0],"actions"))	break;
	if(i<numargs)
	{//	dynamic	wall
		//CPrintf("dynamic");
		d_wall_t	*ent = &dwalls[numdwalls++];
		memset(ent, 0, sizeof(d_wall_t));
		ent->model = modelnum;
		//	находим сектора, в которых расположена стена
		ent->first_sector = num_ent_sectors;
		FindSectors(map.models[0].headnode[0]);
		ent->num_sectors = num_ent_sectors - ent->first_sector;
		//CPrintf("Sectors: (%d-%d)", ent->first_sector, num_ent_sectors-1);
		//	разбираем свойства данной стены
		cur_action_script = NULL;
		for(i=0; i<numargs; i++)
		{
			if(!strcmp(args[i][0],"sounds"))
				sscanf(args[i][1], "%d", &ent->sounds);
			else
			if(!strcmp(args[i][0],"dist"))
				sscanf(args[i][1], "%f", &ent->dist);
			if(!strcmp(args[i][0],"targetname"))
			{
				ent->targetname = actions + actions_size;
				strcpy(actions + actions_size, args[i][1]);
				actions_size += strlen(args[i][1])+1;
			} else
			if(!strcmp(args[i][0],"actions"))
				cur_action_script = args[i][1];
		}
		//	разбираем и компилируем скрипт для данной стены
		if(cur_action_script==NULL)
		{
			CPrintf("Model[%d] script error", modelnum);
			return;
		}
		ent->actions = &actions[actions_size];
		int		index = actions_size;
		while(FindNextEntCommand())	;
		ent->cur_action = 0;
		ent->end_action = actions_size - index;
		if(ent->end_action<5)
		{
			ent->actions = NULL;
			CPrintf("Model[%d] script error", modelnum);
			return;
		}
		FindFirstCommand(ent, c_time);
		VectorsMiddle(model->mins, model->maxs, ent->turn_pos);
		ent->radius = BBoxRadius(model->mins, model->maxs, ent->turn_pos);
		//CPrintf("TurnPos=(%f,%f,%f)", ent->turn_pos[0], ent->turn_pos[1], ent->turn_pos[2]);
		//CPrintf("Radius=%.2f", ent->radius);
	} else
	{//	static wall
		//CPrintf("static");
		s_wall_t	*ent = &swalls[numswalls++];
		memset(ent, 0, sizeof(s_wall_t));
		ent->model = modelnum;
		//	находим сектора, в которых расположена стена
		ent->first_sector = num_ent_sectors;
		FindSectors(map.models[0].headnode[0]);
		ent->num_sectors = num_ent_sectors - ent->first_sector;
	}
}
void	FindSectors(int n)
{
	plane_t	*plane;
	while (n >= 0)
	{
		t_node	*node = &map.nodes[n];
		plane = &map.planes[node->planenum];
		vec3_t	new_mins, new_maxs;
		for(int i=0; i<3; i++)
		{
			if(plane->normal[i]>0)
			{
				new_mins[i] = mins[i];
				new_maxs[i] = maxs[i];
			} else
			{
				new_mins[i] = maxs[i];
				new_maxs[i] = mins[i];
			}
		}
		float	dist1 = PointPlaneDist(new_mins, plane);
		float	dist2 = PointPlaneDist(new_maxs, plane);
		if((dist1>=0) && (dist2>=0))	n = node->children[0];
		else
		if((dist1<0) && (dist2<0))		n = node->children[1];
		else
		{
			n = node->children[0];
			FindSectors(node->children[1]);
		}
	}
	n = ~n;
	if(n>0)	ent_sectors[num_ent_sectors++] = n;
}
void	LoadLight()
{
	t_light	*light = &map.lights[/*numlights++*/0];
	map.numlights++;
	memset(light, 0, sizeof(t_light));
	int		i=numargs;
	float	color[3], l=0;
	while(i--)
	{
		if(!strcmp(args[i][0],"origin"))
			sscanf(args[i][1], "%f %f %f", &light->pos[0], &light->pos[1], &light->pos[2]);
		else
		if(!strcmp(args[i][0],"color"))
		{
			sscanf(args[i][1], "%f %f %f", &color[0], &color[1], &color[2]);
		} else
		if(!strcmp(args[i][0],"light"))
		{
			sscanf(args[i][1], "%f", &l);
		} else
		if(!strcmp(args[i][0],"volume"))
		{
		}
	}
	l *= 0x1000;
	for(i=0; i<2; i++)	light->i[i] = color[i]*l;
}
void	LoadInfoPlayerStart()
{
	int		i=numargs;
	while(i--)
	{
		if(!strcmp(args[i][0],"origin"))
		{
			sscanf(args[i][1], "%f %f %f", &entity.origin[0], &entity.origin[1], &entity.origin[2]);
			entity.origin[2] += 24;
		} else
		if(!strcmp(args[i][0],"angle"))
		{
			float	angle;
			sscanf(args[i][1], "%f", &angle);
			entity.angles[0] = entity.angles[1] = 0;
			entity.angles[2] = 0x4000 - angle*0x8000/180;
		}
	}
	VectorCopy(entity.origin, entity.old_origin);
	VectorCopy(vec3_null, entity.velocity);
}
void	LoadIllusionPortal()
{
	if(num_i_portals>=256)	return;
	t_illusion_portal	*prt = &i_portals[num_i_portals++];
	memset(prt, 0, sizeof(t_illusion_portal));
	int		i=numargs;
	while(i--)
	{
		if(!strcmp(args[i][0],"origin"))
			sscanf(args[i][1], "%f %f %f", &prt->pos[0], &prt->pos[1], &prt->pos[2]);
		else
		if(!strcmp(args[i][0],"mangle"))
		{
			sscanf(args[i][1], "%f %f %f", &prt->angle[0], &prt->angle[2], &prt->angle[1]);
			float	koef = float(0x8000/180);
			prt->angle[0] *= koef;
			prt->angle[1] *= koef;
			prt->angle[2] = -prt->angle[2]*koef + 0x4000;
		} else
		if(!strcmp(args[i][0],"targetname"))
		{
			prt->targetname = actions + actions_size;
			strcpy(actions + actions_size, args[i][1]);
			actions_size += strlen(args[i][1])+1;
		} else
		if(!strcmp(args[i][0],"target"))
		{
			prt->target = actions + actions_size;
			strcpy(actions + actions_size, args[i][1]);
			actions_size += strlen(args[i][1])+1;
		}
	}
	prt->targetnum = -1;
}
void	LoadEntity(char *src, int size)
{
	if(!ExtractAllArguments(src, size))	return;
	int	i = numargs;
	while(i-->0)
	{
		if(!strcmp(args[i][0], "classname"))
			break;
	}
	if(i<0)
	{
		CPrintf("Absence necessary entity parametr \"classname\"");
		return;
	}
	if(!memcmp(args[i][1],"light", 5))		LoadLight();
	else
	if(!strcmp(args[i][1],"func_wall"))	LoadWall();
	else
	if(!strcmp(args[i][1],"func_door"))	LoadWall();
	else
	if(!strcmp(args[i][1],"func_illusionary"))	LoadWall();
	else
	if(!strcmp(args[i][1],"info_intermission"))	LoadIllusionPortal();
	else
	if(!strcmp(args[i][1],"info_player_start"))		LoadInfoPlayerStart();
	else
	if(!strcmp(args[i][1],"worldspawn"))	;
	else	CPrintf("Unknown entity class \"%s\"", args[i][1]);
}

int		FindNextEntCommand()
{
	int		pos=0, size=0;
	float	time;
repeat:
	while((cur_action_script[pos]!='(') && cur_action_script[pos])	pos++;
	if(!cur_action_script[pos])	return	0;
	cur_action_script[pos] = 0;
	int		tmp_pos = pos + 1;
	char	*begin = cur_action_script + tmp_pos;
	while((cur_action_script[pos]!=' ') && (pos>0))	pos--;
	if(pos)	pos++;
	char	*com_name = cur_action_script + pos;
	pos = tmp_pos;
	int		num_params = 0;
	while((cur_action_script[pos]!=')') && cur_action_script[pos])
	{
		if(cur_action_script[pos]==',')	num_params++;
		pos++;
	}
	if(!cur_action_script[pos])	return	0;
	if(pos>tmp_pos)	num_params++;
	cur_action_script += pos;

	if(!strcmp(com_name,"m"))
	{//	move
		if(num_params!=4)	goto	repeat;
		t_action	*action = (t_action *)&actions[actions_size];
		action->type[0] = 1;
		action->type[1] = 1;
		sscanf(begin, "%f,%f,%f,%f", &action->p[0], &action->p[1], &action->p[2], &time);
		action->d_time = int(time*1000);
		size = 8 + 4*3;
	} else
	if(!strcmp(com_name,"r"))
	{//	rotate
		if(num_params!=4)	goto	repeat;
		t_action	*action = (t_action *)&actions[actions_size];
		action->type[0] = 1;
		action->type[1] = 2;
		sscanf(begin, "%f,%f,%f,%f", &action->p[0], &action->p[1], &action->p[2], &time);
		float	temp = 0x8000/180.0f;
		action->p[0] *= temp;
		action->p[1] *= temp;
		action->p[2] *= temp;
		action->d_time = int(time*1000);
		size = 8 + 4*3;
	} else
	if(!strcmp(com_name,"mr"))
	{//	move and rotate
		if(num_params!=7)	goto	repeat;
		t_action	*action = (t_action *)&actions[actions_size];
		action->type[0] = 1;
		action->type[1] = 3;
		sscanf(begin, "%f,%f,%f,%f,%f,%f,%f", &action->p[0], &action->p[1], &action->p[2], &action->p[3], &action->p[4], &action->p[5], &time);
		float	temp = 0x8000/180.0f;
		action->p[3] *= temp;
		action->p[4] *= temp;
		action->p[5] *= temp;
		action->d_time = int(time*1000);
		size = 8 + 7*3;
	} else
	if(!strcmp(com_name,"w"))
	{//	wait
		if(num_params!=1)	goto	repeat;
		t_action	*action = (t_action *)&actions[actions_size];
		action->type[0] = 1;
		action->type[1] = 0;
		sscanf(begin, "%f", &time);
		action->d_time = int(time*1000);
		size = 8 + 0*3;
	} else
	if(!strcmp(com_name,"wt"))
	{//	wait touch or triggers
		if(!num_params)
		{//	wait touch
			t_action	*action = (t_action *)&actions[actions_size];
			action->type[0] = 2;
			action->type[1] = 1;
			size = 4;
		} else
		{//	wait triggers
			if(num_params!=1)	goto	repeat;
			t_wtrigger_action	*action = (t_wtrigger_action *)&actions[actions_size];
			action->type[0] = 2;
			action->type[1] = 2;
			sscanf(begin, "%d", &action->num_triggers);
			size = 8;
		}
	} else
	if(!strcmp(com_name,"b"))
	{//	back
		if(num_params!=1)	goto	repeat;
		t_action	*action = (t_action *)&actions[actions_size];
		action->type[0] = 3;
		int		type;
		sscanf(begin, "%d", &type);	//	back type
		action->type[1] = type;
		size = 4;
	} else
	{
		CPrintf("Unkonwn command name \"%s\"", com_name);
		goto	repeat;
	}
	t_action	*action = (t_action *)&actions[actions_size];
	action->size = size;
	actions_size += size;
	action = (t_action *)&actions[actions_size];
	action->prev_size = size;
	return	1;
}
/*CPrintf("next");
		int	pos = 0;
		while(pos<ent->end_action)
		{
			t_action	*action = (t_action *)&ent->actions[pos];
			CPrintf"C(%d,%d) p_size=%d size=%d", action->type[0], action->type[1], action->prev_size, action->size);
			pos += action->size;
		}*/