
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

#include "map.h"

int		time;

#define PROGRESS_ITEMS 40

//=============================================================================

static char *progress_str;
static int  value;

void	DrawProgress (int t)
{
	int		i;
	printf ("\r%s [", progress_str);
	for (i=0; i<t; i++) printf (".");
	for (i=t; i<PROGRESS_ITEMS; i++) printf (" ");
    if (t < PROGRESS_ITEMS) printf ("] - %d sec\r", (timeGetTime()-time)/1000);
    else printf ("] - %d sec\n", (timeGetTime()-time)/1000);
}

void    InitProgress (char *str)
{
    progress_str = str;
    value = 0;
    DrawProgress (0);
}

void    UpdateProgress (float t)
{
    int     i = (int)(t * PROGRESS_ITEMS);
    if (i == value) return;
    value = i;
    DrawProgress (value);
}

void	main(int argc, char* argv[])
{
	time = timeGetTime ();
	if (argc<2)
	{
		printf ("Usage: converter [options] bspfile\n");
		return;
	}
	if (!ReadMap (argv[1])) return;
	ComputeLightData ();
	WriteMap (argv[1]);
}

