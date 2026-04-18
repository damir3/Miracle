#ifndef _CONFIG_H
#define _CONFIG_H

typedef void	(*LPMF)();
typedef struct
{
	char	name[32];
	char	num_params;
	LPMF	func;
} TCommand;

extern char		keys_names[256][10];

void	AddCommand(char	*com_name, char num_params, LPMF func);
void	InitConsoleCommands();
void	LoadConfig(char *name);
void	ProcessLine(char *str_line);

#endif