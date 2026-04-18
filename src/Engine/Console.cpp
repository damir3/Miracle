/*
	Console.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdio.h>
#include	<math.h>
#include	<windows.h>
#include	<time.h>

#include	"config.h"
#include	"console.h"
#include	"move.h"
#include	"main.h"
#include	"pcxfile.h"
#include	"render.h"
#include	"video.h"
#include	"unpak.h"

//	время открытия и закрытия консоли
#define		CONSOLE_WORK_TIME		500
//	максимальное кол-во запомненых комманд набранных на консоли
#define		MAX_NUM_CACH_COMMANDS	32
//	максимальное кол-во последних запомненых строк консоли
//	(надо при перересовке консоли)
#define		MAX_NUM_SAVED_STRING	256

static FILE		*log_file;

static int		numsavedstr=0;
static fpos_t	savedstrpos[MAX_NUM_SAVED_STRING];
//	комманды консоли
static char		commands[MAX_NUM_CACH_COMMANDS][256];
static int		num_commands=0, cur_command=0;
//	converted console string
int		num_cargs;
char	*cargs[MAX_NUM_PARAMS];

static char		candrawconsole = 0;
static int		init_time=-1, close_time=-1;

extern bitmap8	frame0_r, frame0_l, logo0, background0;
static bitmap	frame_r, frame_l, console_screen, logo;
static int		frameh, framew, cwidth=0, cheight=0, console_size=100;

static int		cursor, max_len=0, str_len;
static char		console_str[128]=">";

extern int		time1;
extern char		enable_menu, cur_key_state;

void	AddCommandToCache(char	*str)
{
	if(!str[1] || (num_commands && !strcmp(commands[num_commands-1], str)))
	{
		cur_command = num_commands;
		return;
	}
	if(num_commands==MAX_NUM_CACH_COMMANDS)
	{
		num_commands = MAX_NUM_CACH_COMMANDS-1;
		for(int i=1; i<MAX_NUM_CACH_COMMANDS; i++)
			strcpy(commands[i-1], commands[i]);
	}
	strcpy(commands[num_commands++], str);
	cur_command = num_commands;
}
void	OpenConsole()
{
	if ((init_time>=0) || (enable_menu))	return;

	int	time = timeGetTime();
	if(close_time<0)
		init_time = time;
	else
		init_time = (time<<1) - CONSOLE_WORK_TIME - close_time;
	can_move = 0;

	if(candrawconsole)
		memset((short *)console_screen.bm+cwidth*(cheight-11), 1, cwidth*16);
	console_str[1] = 0;
	cursor = 1;
	str_len = 2;
	cur_command = num_commands;

	ClearKeyBuf();
	cur_key_state = 2;
}
void	CloseConsole()
{
	if (init_time<0)	return;

	int	time = timeGetTime();
	if((time - init_time) > CONSOLE_WORK_TIME)
		close_time = time;
	else
		close_time = (time<<1) - CONSOLE_WORK_TIME - init_time;

	init_time = -1;

	can_move = 1;
	cur_key_state = 2;
	ClearMoveStatus();
}
void	PushConsole()
{
	if(init_time<0)
		OpenConsole();
	else
		CloseConsole();
}
ushort	GetColor(char n)
{
	byte	r, g, b;
	switch(n)
	{
	case	'0':	r=g=b=0;	break;
	case	'1':	b=255;	r=g=0;	break;
	case	'2':	g=255;	r=b=0;	break;
	case	'3':	r=0;	g=b=255;	break;
	case	'4':	r=255;	g=b=0;	break;
	case	'5':	g=0;	r=b=255;	break;
	case	'6':	b=0;	r=g=255;	break;
	case	'7':	b=0;	r=255;	g=128;	break;
	case	'8':	r=g=b=192;	break;
	case	'9':	r=g=b=128;	break;
	}
	if(bits==15)
		return	((r>>3)<<10) + ((g>>3)<<5) + (b>>3);
	return	((r>>3)<<11) + ((g>>2)<<5) + (b>>3);
}
void	DrawNewConsoleString(byte *str)
{
	ushort	color = GetColor('2');
	while(*str)
	{
		memcpy((short *)console_screen.bm+cwidth*4, (short *)console_screen.bm+cwidth*14, cwidth*(cheight-34)*sizeof(short));
		memset((short *)console_screen.bm+cwidth*(cheight-28), 1, cwidth*16);
		int	x=4;
		while((*str) && (x<cwidth-4))
		{
			if(*str!=' ')
			{
				if((*str=='_') && (*(str+1)>='0') && (*(str+1)<='9'))
				{
					color = GetColor(*(str+1));
					str +=2;
				}
				ushort	*dest = (ushort *)console_screen.bm + cwidth*(cheight-28)+x;
				ushort	*out = (ushort *)font.bm + ((*str&15)<<3) + ((*str>>4)<<10);
				int		j=8;
				while(j--)
				{
					int	i=8;
					while(i--)
					{
						if(*out++)
							*dest++ = color;
						else
							dest++;
					}
					out += 15*8;
					dest += cwidth-8;
				}
			}
			str++;
			x += 8;
		}
	}
}

void	CPrintf(char *format, ...)
{
	char	tmpstr[1024];
	va_list	(args);
	va_start(args, format);
	vsprintf(tmpstr, format, args);
	va_end(args);
	if(!candrawconsole)
	{
		MessageBox(NULL, tmpstr, program_name, MB_OK);
	} else
	{
		DrawNewConsoleString((byte *)tmpstr);
		if((init_time>0) || ((close_time>0) && ((time1-close_time)<CONSOLE_WORK_TIME)))
			UpdateFrame();
	}
	WriteLog(tmpstr);
}
void	CPrintfMessage(char *format, ...)
{
	char	tmpstr[1024];
	va_list	(args);
	va_start(args, format);
	vsprintf(tmpstr, format, args);
	va_end(args);
	if(candrawconsole)
		DrawNewConsoleString((byte *)tmpstr);
	WriteLog(tmpstr);
}
void	ReDrawConsoleString(byte *str)
{
	ushort	color = GetColor('2');
	memset((short *)console_screen.bm+cwidth*(cheight-11), 1, cwidth*16);
	int	x=4;
	while((*str) && (x<cwidth-4))
	{
		ushort	*dest = (ushort *)console_screen.bm + cwidth*(cheight-11)+x;
		ushort	*out = (ushort *)font.bm + ((*str&15)<<3) + ((*str>>4)<<10);
		int		j=8;
		while(j--)
		{
			int	i=8;
			while(i--)
			{
				if(*out++)
					*dest++ = color;
				else
					dest++;
			}
			out += 15*8;
			dest += cwidth-8;
		}
		str++;
		x += 8;
	}
}
void	SetConsoleString(char *str)
{
	if(!candrawconsole)	return;
	if(init_time<0)	return;
	strcpy(console_str+1, str);
	UpdateFrame();
}
void	UpdateConsoleString()
{
	if(init_time<=0)	return;
	char	c;
	while(c = ReadKey())
	{
		if(c==8)
		{
			if(cursor>1)
			{
				strcpy(console_str+cursor-1, console_str+cursor);
				cursor--;
				str_len--;
			}
			continue;
		}
		if(c==13)
		{
			ClearKeyBuf();
			AddCommandToCache(console_str);
			CPrintf(console_str);
			ProcessLine(console_str+1);
			console_str[1] = 0;
			cursor = 1;
			str_len = 2;
			continue;
		}
		if(c==19)
		{
			cursor = str_len - 1;
			continue;
		}
		if(c==20)
		{
			cursor = 1;
			continue;
		}
		if(c==22)	//	up
		{
			if(cur_command>0)
			{
				strcpy(console_str, commands[--cur_command]);
				cursor = strlen(console_str);
				str_len = cursor+1;
			} else
			{
				cur_command = -1;
				console_str[1] = 0;
				cursor = 1;
				str_len = 2;
			}
		}
		if(c==24)	//	down
		{
			if(cur_command<num_commands-1)
			{
				strcpy(console_str, commands[++cur_command]);
				cursor = strlen(console_str);
				str_len = cursor+1;
			} else
			{
				cur_command = num_commands;
				console_str[1] = 0;
				cursor = 1;
				str_len = 2;
			}

		}
		if(c==21)	//	left
		{
			if(cursor>1)	cursor--;
			continue;
		}
		if(c==23)	//	right
		{
			if(cursor<str_len-1)	cursor++;
			continue;
		}
		if(c==30)
		{
			if(cursor+1<str_len)
			{
				strcpy(console_str+cursor, console_str+cursor+1);
				str_len--;
			}
			continue;
		}
		if((c>=32) && (str_len<max_len))
		{
			char	*dest = console_str+str_len;
			char	*out = console_str+str_len-1;
			int		i = str_len - cursor;
			while(i--)	*dest-- = *out--;
			console_str[cursor++] = c;
			console_str[str_len++] = 0;
		}
	}
}
void	DrawConsole()
{
	if(!candrawconsole)	return;
	//int	alpha=int(cos(time1*3.1415/2000)*127)+128;
	//DrawSprite(sx_size-logo.width, sy_size-logo.height, &logo, alpha);
	int		pos;
	if(init_time<0)
	{
		if(close_time<0)	return;
		if((time1-close_time)<CONSOLE_WORK_TIME)
			pos = (close_time - time1)*frameh/CONSOLE_WORK_TIME;
		else
		{
			close_time = -1;
			return;
		}
	} else
	{
		if((time1-init_time)<CONSOLE_WORK_TIME)
			pos = (frameh*(time1-init_time)/CONSOLE_WORK_TIME) - frameh;
		else
			pos = 0;
	}
	if(pos+console_screen.height+int(sy_size*0.05)>0)
	{
		ReDrawConsoleString((byte *)console_str);
		memset((short *)console_screen.bm + cwidth*(cheight-4)+(cursor<<3)+4, 3, 16);
		mgl.DrawBitmap((sx_size-cwidth)>>1, pos+int(sy_size*0.05), &console_screen, (128+pos*128/frameh), 0);
	}
	if(pos+frameh>0)
	{
		mgl.DrawBitmap(sx_size - framew, pos, &frame_r, 255, 1);
		mgl.DrawBitmap(0, pos, &frame_l, 255, 1);
	}
}

void	InitConsole()
{
	logo.bm = NULL;
	logo.width = 0;
	logo.height = 0;
	frame_r.bm = NULL;
	frame_r.width = 0;
	frame_r.height = 0;
	frame_l.bm = NULL;
	frame_l.width = 0;
	frame_l.height = 0;
	console_screen.bm = NULL;
	console_screen.width = 0;
	console_screen.height = 0;
	//ResizeToBitmap16(&background, &background0, background0.width, background0.height);
	ConvertToGrayscale(&background, &background0);
	free(background0.bm);
}
void	MakeConsole()
{
	candrawconsole = 0;
	free(frame_r.bm);
	free(frame_l.bm);
	free(logo.bm);

	cheight = ((sy_size/20)*10)+2;
	cwidth = int(0.96*sx_size);

	framew = int(sx_size*0.045);
	frameh = int(sy_size*0.65);

	max_len = (cwidth>>3)-1;

	if((console_screen.width!=cwidth) || (console_screen.height!=cheight))
	{
		console_screen.bm = (char *)realloc(console_screen.bm, cwidth*cheight*sizeof(short));
		console_screen.width = cwidth;
		console_screen.height = cheight;
		console_screen.bpp = bits;
		memset(console_screen.bm, 1, cwidth*cheight*sizeof(short));
		memset((short *)console_screen.bm+cwidth*(cheight-17)+4, 3, (cwidth-8)*2);
		memset((short *)console_screen.bm+cwidth*(cheight-16)+4, 3, (cwidth-8)*2);
	}
	ResizeToBitmap16(&frame_r, &frame0_r, framew, frameh);
	ResizeToBitmap16(&frame_l, &frame0_l, framew, frameh);
	ResizeToBitmap16(&logo, &logo0, logo0.width*sx_size/640, logo0.height*sy_size/480);
	candrawconsole = 1;
}
void	ConvertStringToLogString(char *str)
{
	for(int i=0, j=0; str[i]; i++)
	{
		if((str[i]=='_') && (str[i+1]>='0') && (str[i+1]<='9'))
			i++;
		else
			str[j++] = str[i];
	}
	str[j] = '\n';
	str[j+1] = 0;
}
int		InitLogFile(char *name)
{
	char	log_file_name[1024];
	strcpy(log_file_name, program_path);
	strcat(log_file_name, name);
	log_file = fopen(log_file_name, "w");
	if(log_file==NULL)
	{
		CPrintf("Can't create file \"%s\"", name);
		return	0;
	}
	time_t	ltime;
	time( &ltime );
	if(!fprintf(log_file, "Miracle 3D Engine log file - %s\n", ctime(&ltime)))
	{
		CPrintf("Can't write to file \"%s\"", name);
		fclose(log_file);
		return	0;
	}
	fflush(log_file);
	return	1;
}
void	WriteLog(char *str)
{
	if(log_file==NULL)	return;
	fgetpos(log_file, &savedstrpos[(numsavedstr++)%MAX_NUM_SAVED_STRING]);
	ConvertStringToLogString(str);
	fprintf(log_file, str);
	fflush(log_file);
}
void	CloseLogFile()
{
	if(log_file==NULL)	return;
	fclose(log_file);
}