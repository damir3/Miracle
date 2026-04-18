/*
	UnPak.cpp		Copyright (C) 1998-1999 Vitaly Ovtchinnikov
					and Damir Sagidullin
*/
#include	<stdio.h>
#include	<malloc.h>

#include	"console.h"
#include	"main.h"
#include	"unpak.h"

int	CPakFile::OpenPak(char *fname)
{
	res = NULL;
	fd = INVALID_HANDLE_VALUE;

	char	full_name[1280];
	strcpy(pak_name, "DATA\\");
	strcat(pak_name, fname);
	strcpy(full_name, program_path);
	strcat(full_name, pak_name);

	DWORD	bytes_read;
	fd = CreateFile(full_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fd==INVALID_HANDLE_VALUE)
	{
		CPrintf("File \"%s\" not found", pak_name);
		return	0;
	}

	ReadFile(fd, &header, sizeof(header), &bytes_read, NULL);
	if(strcmp(header.id, def_tm))
	{
		CPrintf("\"%s\" is not pak file", pak_name);
		ClosePak();
		return	0;
	}

	res_number = header.res_number;

	res = (str_res *)malloc(res_number*sizeof(str_res));
	if (!res)
	{
		CPrintf("Not enough memory");
		ClosePak();
		return	0;
	}

	ReadFile(fd, res, res_number*sizeof(str_res), &bytes_read, NULL);
	if (bytes_read != res_number*sizeof(str_res))
	{
		CPrintf("Unable to read \"%s\"", pak_name);
		ClosePak();
		return	0;
	}
	return	1;
}

void	CPakFile::ClosePak()
{
	if (res!=NULL)	free(res);
	if (fd!=INVALID_HANDLE_VALUE)	CloseHandle(fd);
}

int	CPakFile::GetItemsNumber()
{
	return	res_number;
}

int	CPakFile::GetItemName(int num, char *buf)
{
	if (num<res_number)
	{
		strcpy(buf, res[num].name);
		return	1;
	}
	return	-1;
}

int	CPakFile::GetItemRealSize(int num)
{
	if (num<res_number) return res[num].real_size;
	return	-1;
}

int	CPakFile::Extract(int num, char *buf)
{
	int a=-1;
	if (num<res_number)
	{
		unsigned char	*tmp = (unsigned char *)malloc(res[num].size);
		DWORD	br;
		unsigned long	r;
		
		SetFilePointer(fd, res[num].offset + sizeof(header) + sizeof(str_res)*res_number, NULL, FILE_BEGIN);
		ReadFile(fd, tmp, res[num].size, &br, NULL);

		r = res[num].real_size;
		a = uncompress((unsigned char *)buf, &r, tmp, res[num].size);

		free(tmp);
	}
	if(a==-1)
	{
		CPrintf("Can't extract \"%s\" from \"%s\".", res[num].name, pak_name);
	}
	return	a;
}

int		CPakFile::GetNameItem(char *name)
{
	for(int x=0; x<res_number; x++)
    	if (!strcmp(res[x].name, name))	return	x;
    CPrintf("File \"%s:%s\" not found.", pak_name, name);
	return	-1;
}

int		CPakFile::ExtractByName(char *name, char **buf)
{
	/*sprintf(tmpstr, "Reading file \"%s\"", name);
	SetConsoleString(tmpstr);*/
	int		itemnumber = GetNameItem(name);
	if(itemnumber == -1)	return	-1;
	int		size = GetItemRealSize(itemnumber);
	if(!(*buf = (char *)malloc(size)))	return	-1;
	if(Extract(itemnumber, *buf)==-1)
    {
    	CPrintf("Can't extract \"%s\" from \"%s\"", name, pak_name);
    	return	-1;
	}
	return	size;
}
