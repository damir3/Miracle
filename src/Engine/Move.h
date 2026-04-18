#ifndef _MOVE_H
#define _MOVE_H

#include	"struct.h"

#define		NORMAL_MOVE	1
#define		NOCLIP_MOVE	2
#define		FLY_MOVE	4

extern entity_t	entity;
extern vec3_t	cam_position, cam_angles;
extern char		can_move;

void	InitMove();
void	CheckMove(vec3_t start, vec3_t end);
void	CheckEntitiesMove();
void	Move(int d_time);
void	ChangeForwardSpeed();
void	ChangeBackSpeed();
void	ChangeSideSpeed();
void	ChangeTurnSpeed();
void	ChangeFov();
void	ChangeZoomFov();
void	ChangeZoomDist();
void	ChangePosition();
void	ChangeAngles();
void	Walk();
void	NoClip();
void	Fly();
void	MoveForward();
void	MoveBack();
void	MoveRight();
void	MoveLeft();
void	MoveUp();
void	MoveDown();
void	Jump();
void	Zoom();
void	AutoZoom();
void	StopMove();
void	TurnX0();
void	TurnX1();
void	TurnZ0();
void	TurnZ1();
void	TurnY0();
void	TurnY1();
void	ClearMoveStatus();

void	FindZoom(int);

#endif