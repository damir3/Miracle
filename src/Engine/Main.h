#ifndef	_MAIN_H
#define	_MAIN_H

typedef struct
{
	void	(*CPrintf) (char *format, ...);
	void	(*CPrintfMessage) (char *format, ...);
} GameExport;

extern char	program_name[];
extern char	player_name[32];
extern char	program_path[1024];

void	UpdateFrame();
void	ClearKeyBuf();
char	ReadKey();

void	ChangeMouseSpeed();
void	InvertMouse();
void	Quit();
void	InitMenu();
void	CloseMenu();
void	PushMenu();
void	Pause();
void	ChangeMap();

#endif