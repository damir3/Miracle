/*
	entity.c		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#include "map.h"
#include "math3d.h"

//------------------- DEFINITIONS & CONSTANTS -------------------//

#define	MAX_NUM_ARGUMENTS	256

//-------------------------- VARIABLES --------------------------//

static vector	mins, maxs;
static char		*cur_action_script;

int		numargs;
char	*args	[MAX_NUM_ARGUMENTS][2];

//	light sources
int			numlights;
light_t		lights[MAX_MAP_LIGHTS];

//-------------------------- FUNCTIONS --------------------------//

void	FindSectors (int n);
int		FindNextEntCommand ();

int		ExtractAllArguments (char *src, int size)
{
	int		i=0, j=0, k=0;
	char	c;

	while (size-->0)
	{
		c = *src;
		if (c=='\n')	k=0;
		if (c=='"')
		{
			if (!j)
			{//	start
				j=1;
				if (k<2)
				{//	number of words in string < 2
					if (k==0)
					{
						args[i][0] = args[i][1] = src+1;
					} else
					{
						args[i][1] = src+1;
						i++;
					}
					if (i>=MAX_NUM_ARGUMENTS)	break;
				}
				k++;
			} else
			{//	end
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
	while ((src[x]!='{')&&(x<end))	x++;
	if (x>=end)	return 0;
	*begin = x++;
	for (;(br_open>0)&&(x<end); x++)
	{
		if (src[x] == '{')	br_open++;
		if (src[x] == '}')	br_open--;
	}
	if (x>=end)	return 0;
	*size = x - *begin;
	return	1;
}

void	LoadLight ()
{
	int		i;
	float	color[3], l=0;
	light_t	*light = &lights[numlights++];

	memset (light, 0, sizeof(light_t));
	i = numargs;
	while (i--)
	{
		if (!strcmp (args[i][0],"origin"))
			sscanf (args[i][1], "%f %f %f", &light->pos[0], &light->pos[1], &light->pos[2]);
		else
		if (!strcmp (args[i][0],"_color"))
		{
			sscanf (args[i][1], "%f %f %f", &color[0], &color[1], &color[2]);
		} else
		if (!strcmp (args[i][0],"light"))
		{
			sscanf (args[i][1], "%f", &l);
		} else
		if (!strcmp (args[i][0],"flare"))
		{
			sscanf (args[i][1], "%c", &light->flare);
		}
	}
	l *= 0x1000;
	for (i=0; i<3; i++)	light->i[i] = color[i]*l;
	light->sector = FindSector (0, light->pos);
}
void	LoadEntity (char *src, int size)
{
	int		i;
	static char	temp_buf[0x1000];

	memcpy (temp_buf, src, size);
	if (!ExtractAllArguments (temp_buf, size))	return;
	i = numargs;
	while ((i--) > 0)
	{
		if (!strcmp (args[i][0], "classname"))
			break;
	}
	if (i<0)
	{
		printf ("Absence necessary entity parametr \"classname\"\n");
		return;
	}
	if (!strncmp (args[i][1],"light", 5))	LoadLight ();
}

int		LoadEntities (char *data, int length)
{
	int		offset = 0, size=0;
	numlights = 0;
	while (FindBrackets (data, &offset, &size, length))
	{
		LoadEntity (data + offset, size);
		offset += size;
	}
	//	lights
	map.lights = lights;
	map.numlights = numlights;
	return length;
}