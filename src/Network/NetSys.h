#ifndef NETSYS_H_
#define NETSYS_H_

enum ESysEvents
{
	sysev_none=0, sysev_error,
	sysev_enumfound, sysev_enumend, sysev_enumstart, sysev_enumstop,
	sysev_srvstart, sysev_srvstop, sysev_srvaccept, sysev_srvreset,
	sysev_clstart, sysev_clstop, sysev_clterm
};

struct StrEvent
{
	int	type;
	int	num;
	char	data[256];
};

typedef void (*lpOnEvent)(StrEvent *);
typedef void (*lpOnSysEvent)(StrEvent *);
typedef	int	 (*lpInit)(lpOnEvent, lpOnSysEvent, int, int);
typedef void (*lpDone)(void);
typedef int  (*lpStartEnumeration)(void);
typedef int  (*lpStopEnumeration)(void);
typedef int (*lpStartServer)(int);
typedef int (*lpStop)(void);
typedef int (*lpStartClient)(char *);
typedef int (*lpStartLocal)(void);
typedef void (*lpPost)(StrEvent *);

#endif