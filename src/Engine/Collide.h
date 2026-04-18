#ifndef _COLLIDE_H
#define _COLLIDE_H

#include	"MapFile.h"

#define ENTITY_ON_GROUND	1
#define ENTITY_VISIBLE		2

#define FORWARD_MOVE 0
#define BACKWARD_MOVE 1
#define UP_MOVE 2
#define DOWN_MOVE 3
#define LEFT_MOVE 4
#define RIGHT_MOVE 5

typedef struct
{
	vec3_t	end;
	plane_t	plane;
	short	inwater, inopen;
	short	allsolid, startsolid;
	float	fraction;
} move_t;

void	PushEntity(entity_t *, vec3_t, move_t *);
int		HullPointContents(int num, vec3_t p);
int		FindSector(int num, vec3_t p);
int		FlyMove(entity_t *entity, float time);
int		ImpactPlayer(entity_t * entity, vec3_t push);
int		MovePlayer(entity_t *entity, float time);
int		MoveEntity(entity_t *entity, move_t *move, vec3_t start, vec3_t end);
int		CheckWater(entity_t *entity);
void	CheckCollision(vec3_t from, vec3_t to, float f1, float f2, move_t *);
int		CheckHullCollision(int num, vec3_t from, vec3_t to, float f1, float f2, move_t *);

#endif
