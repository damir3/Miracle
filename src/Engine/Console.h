#ifndef _CONSOLE_H
#define _CONSOLE_H

#include	"struct.h"

#define			MAX_NUM_PARAMS		32

extern int	num_cargs;
extern char	*cargs[MAX_NUM_PARAMS];

void	SetConsoleTextColor(byte r, byte g, byte b);
void	InitConsole();
void	DrawConsole();
void	DrawNewConsoleString(byte *str);
void	CPrintf(char *format, ...);
void	CPrintfMessage(char *format, ...);
void	SetConsoleString(char *str);
void	UpdateConsoleString();
void	MakeConsole();

void	OpenConsole();
void	CloseConsole();
void	PushConsole();

int		InitLogFile(char *name);
void	WriteLog_(char *str);
void	WriteLog(char *str);
void	CloseLogFile();

#endif