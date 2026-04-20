/*
	Config.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdio.h>
#include	<string.h>

#include	"config.h"
#include	"console.h"
#include	"light.h"
#include	"main.h"
#include	"render.h"
#include	"move.h"
#include	"ScrShot.h"
#include	"video.h"

#define			MAX_NUM_COMMANDS	1024

extern LPMF		key_func[256];
extern char		player_name[32];

static int		num_commands=0;
static TCommand	commands[MAX_NUM_COMMANDS];
static char		tmp_str[256];

void	AddCommand(char	*com_name, char num_params,	LPMF func)
{
	strcpy(commands[num_commands].name, com_name);
	commands[num_commands].num_params	= num_params;
	commands[num_commands].func	= func;
	num_commands++;
}
int		CheckParamNum(int num)
{
	if(num<0)
	{
		if(1-num_cargs<=num)
			return	0;
		else
			CPrintf("Needs %d or more parameters", -num);
	}
	else
	{
		if(num_cargs==1)	return	0;
		if(num_cargs-1==num)
			return	0;
		else
			CPrintf("Needs %d parameters", num);
	}
	return	1;
}

int		GetAllArguments(char *str)
{
	cargs[0] = cargs[1] = str;
	char	c, j=0;
	num_cargs=0;
	while(c = *str)
	{
		if((c!=' ') && (c!=',') && (c!=9) && (c!=10) && (c!=12))
		{
			if(j==0)
			{
				cargs[num_cargs++] = str;
				if(num_cargs>=MAX_NUM_PARAMS)	break;
				j = 1;
			}
		} else
		{
			if((c==',') || ((j==1) && (num_cargs==1)))
			{//	next substring
				j = 0;
				*str = 0;
			}
		}
		str++;
	}
	if(*(str-1)=='\n')	*(str-1) = 0;
	return	num_cargs;
}

void	ProcessLine(char *str_line)
{
	if(!GetAllArguments(str_line))	return;
	if((cargs[0][0]=='/') && (cargs[0][1]=='/'))	return;
	for(int i=0; i<num_commands; i++)
	{
		if(!_stricmp(cargs[0], commands[i].name))
		{
			/*if(CheckParamNum(commands[i].num_params))
				return;*/
			commands[i].func();
			return;
		}
	}

	CPrintf("Unknown command \"%s\"", cargs[0]);
}

void	LoadConfig(char *name)
{
	char	tmp_str[1280], config_name[256];
	strcpy(config_name, "DATA\\");
	strcat(config_name, name);
	strcpy(tmp_str, program_path);
	strcat(tmp_str, config_name);
	FILE	*stream = fopen(tmp_str, "r");
	if (!stream)
	{
		CPrintf("File \"%s\" not found",config_name);
		return;
	} else
		CPrintf("Load config \"%s\"",name);
	int	i=0;
	while(!feof(stream))
	{
		if(fgets(tmp_str, 1023, stream)==NULL)	break;
		ProcessLine(tmp_str);
	}
	fclose(stream);
}
void	Bind()
{
	char	key[64]="";
	char	command[64]="";
	sscanf(cargs[1], "%s %s", key, command);
	for(int i=0; i<256; i++)
	{
		if(!_stricmp(key, keys_names[i]))
			break;
	}
	if(i==256)
	{
		CPrintf("Unknown key <%s>", key);
		return;
	}
	for(int j=0; j<num_commands; j++)
	{
		if(!_stricmp(command, commands[j].name))
		{
			key_func[i] = commands[j].func;
			break;
		}
	}
	if(j<num_commands)
	{
		CPrintf("Bind to <%s>:", keys_names[i]);
		CPrintf("    \"%s\"", commands[j].name);
	} else
	{
		CPrintf("Unknown command \"%s\"", command);
		return;
	}
	return;
}

void	ChangeName()
{
	if(num_cargs>1)
		strcpy(player_name, cargs[1]);
	CPrintf("Player name is \"%s\"", player_name);
}
void	Exec()
{
	LoadConfig(cargs[1]);
}
void	Help()
{
	CPrintf("Commands:");
	for(int i=0; i<num_commands; i++)
		CPrintf("    %s", commands[i].name);
}
void	InitConsoleCommands()
{
	//	main.cpp
	AddCommand("quit",			0, Quit);
	AddCommand("openmenu",		0, InitMenu);
	AddCommand("closemenu",	0, CloseMenu);
	AddCommand("pushmenu",		0, PushMenu);
	AddCommand("pause",			0, Pause);
	AddCommand("mousespeed",	1, ChangeMouseSpeed);
	AddCommand("invertmouse",	0, InvertMouse);
	//	move.cpp
	AddCommand("forwardspeed",1, ChangeForwardSpeed);
	AddCommand("backspeed",	1, ChangeBackSpeed);
	AddCommand("sidespeed",	1, ChangeSideSpeed);
	AddCommand("turnspeed",	1, ChangeTurnSpeed);
	AddCommand("fov",	1, ChangeFov);
	AddCommand("zoomfov",	1, ChangeZoomFov);
	AddCommand("zoomdist",	1, ChangeZoomDist);
	AddCommand("position",	3, ChangePosition);
	AddCommand("tangles",	3, ChangeAngles);
	AddCommand("walk",	0, Walk);
	AddCommand("noclip",	0, NoClip);
	AddCommand("fly",	0, Fly);
	AddCommand("forward",	0, MoveForward);
	AddCommand("back",	0, MoveBack);
	AddCommand("moveright",	0, MoveRight);
	AddCommand("moveleft",	0, MoveLeft);
	AddCommand("moveup",	0, MoveUp);
	AddCommand("movedown", 0, MoveDown);
	AddCommand("lookup",	0, TurnX0);
	AddCommand("lookdown",	0, TurnX1);
	AddCommand("turny0",	0, TurnY0);
	AddCommand("turny1",	0, TurnY1);
	AddCommand("right",	0, TurnZ0);
	AddCommand("left",	0, TurnZ1);
	AddCommand("jump",	0, Jump);
	AddCommand("zoom",	0, Zoom);
	AddCommand("autozoom",	0, AutoZoom);
	AddCommand("stop",	0, StopMove);
	//	console.cpp
	AddCommand("openconsole",	0, OpenConsole);
	AddCommand("closeconsole",	0, CloseConsole);
	AddCommand("pushconsole",	0, PushConsole);
	//AddCommand("console_size",	1, ConsoleSize);
	//	config.cpp
	AddCommand("bind",	-2, Bind);
	AddCommand("map",	1, ChangeMap);
	AddCommand("exec",	1, Exec);
	AddCommand("help",	0, Help);
	AddCommand("name",	1, ChangeName);
	AddCommand("screenshot",	1, SaveScreenShot);
	AddCommand("omnilight", 3, OmniLight);
}
char	keys_names[256][10] = {
		"Mouse1",
		"Mouse2",
		"Mouse3",
		"",
		"",
		"",
		"",
		"",
		"BackSpace",
		"Tab",//	9
		"",
		"",
		"[5]",
		"Enter",
		"",
		"",
		"Shift",
		"Ctrl",//	17
		"",
		"Pause",
		"CapsLock",
		"",
		"",
		"",
		"",
		"",
		"",
		"Esc",//	27
		"",
		"",
		"",
		"",
		"Space",//	32
		"PgUp",
		"PgDown",
		"End",
		"Home",
		"Left",
		"Up",
		"Right",
		"Down",//	40
		"",
		"",
		"",
		"",
		"Insert",
		"Delete",
		"",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",//	57
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"A",//	65
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",//	90
		"",
		"",
		"",
		"",
		"",
		"Num0",
		"Num1",
		"Num2",
		"Num3",
		"Num4",//	100
		"Num5",
		"Num6",
		"Num7",
		"Num8",
		"Num9",
		"*",
		"+",
		"",
		"-",
		".",//	110
		"/",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",//	120
		"F10",
		"F11",
		"F12",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	130
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	140
		"",
		"",
		"",
		"NumLock",
		"ScrLock",
		"",
		"",
		"",
		"",
		"",//	150
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	160
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	170
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	180
		"",
		"",
		"",
		"",
		"",
		";",
		"=",
		"<",
		"_",
		">",//	190
		"?",
		"~",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	200
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	210
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"[",
		"\\",	//	220
		"]",
		"'",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	230
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	240
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",//	250
		"",
		"",
		"",
		"",
		""};
