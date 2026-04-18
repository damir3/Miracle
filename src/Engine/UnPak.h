#ifndef _UNPAK_H
#define _UNPAK_H

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include	<windows.h>

#include	"zlib.h"
#include	"zconf.h"

#define	def_max_files	1024
#define	def_tm			"Bright eyes pak"
#define	def_max_name	64

typedef	struct
{
	char	name[def_max_name];
	int		offset;
	int		size;
	int		real_size;
}str_res;

typedef struct
{
	char	id[16];
	int		res_number;
}str_header;

class	CPakFile
{
	char		pak_name[256];
	str_header	header;
	str_res		*res;
	HANDLE		fd;
	int			res_number;
public:
	int		OpenPak(char *fname);
	void	ClosePak();
	int		GetItemsNumber();
	int		GetItemName(int num, char *name);
	int		GetNameItem(char *name);
	int		GetItemRealSize(int num);
	int		Extract(int num, char *buf);
	int		ExtractByName(char *name, char **buf);
};

#endif