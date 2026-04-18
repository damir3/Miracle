#ifndef	_NET_H
#define	_NET_H

enum ESysEvents
{
	sysev_none=0, sysev_error, 
	sysev_enumfound, sysev_enumend, sysev_enumstart, sysev_enumstop,
	sysev_srvstart, sysev_srvstop, sysev_srvaccept, sysev_srvreset,
	sysev_clstart, sysev_clstop, sysev_clterm
};

typedef struct	StrEvent
{
	int		type;
	int		num;
	char	data[256];
} StrEvent;

typedef void (*lpOnEvent) (StrEvent *);
typedef void (*lpOnSysEvent) (StrEvent *);

typedef struct
{
	int		(*Init) (lpOnEvent, lpOnSysEvent, int, int);
	void	(*Done) (void);
	int		(*GetIP) (void);
	int		(*StartEnumeration) (void);
	int		(*StopEnumeration) (void);
	int		(*StartServer) (int);
	int		(*Stop) (void);
	int		(*StartClient) (char *);
	int		(*StartLocal) (void);
	void	(*Post) (StrEvent *);
} NetGame;

void	StartLocalServer();
int		LoadNetLibrary(char *);
void	Connect();
void	Disconnect();
void	NetMessage();
void	StartNetServer();
void	CloseNetLibrary();
void	GetIP();

#endif