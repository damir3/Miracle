#include "netenum.h"
#include "netsys.h"

#include <winsock2.h>
#include <crtdbg.h>
#include <stdio.h>

char	str_id[]="mrcl";

HANDLE	hEnumeration;
int		iTerminate;
SOCKET	e_sock;

extern lpOnSysEvent OnSysEvent;
extern int	port;


DWORD WINAPI	Enumeration(LPVOID p)
{
	StrEvent ev;

	e_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	BOOL	bb = true;
	setsockopt(e_sock, SOL_SOCKET, SO_BROADCAST, (char *)&bb, sizeof(bb));

	SOCKADDR_IN	addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(*(int *)p);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

	bind(e_sock, (LPSOCKADDR)&addr, sizeof(addr));

	sendto(e_sock, str_id, strlen(str_id), 0, (LPSOCKADDR)&addr, sizeof(addr));

	while(!iTerminate)
	{
		unsigned char	s[4];
		int		addrlen = sizeof(addr);
		if(recvfrom(e_sock, (char *)s, 4, 0, (LPSOCKADDR)&addr, &addrlen) == 4)
		{
			ev.type = sysev_enumfound;
			sprintf(ev.data, "%d.%d.%d.%d", s[0], s[1], s[2], s[3]);
			OnSysEvent(&ev);
		}
	}

	ev.type = sysev_enumend;
	OnSysEvent(&ev);


	closesocket(e_sock);


	return 1;
}

int		StartEnum()
{
	iTerminate = 0;

	hEnumeration = CreateThread(NULL, 0, Enumeration, &port, 0, NULL);
	if (hEnumeration)
	{
		StrEvent ev;
		ev.type = sysev_enumstart;
		OnSysEvent(&ev);
	}
	return hEnumeration?1:0;
}

int		StopEnum()
{
	if (!hEnumeration) return 0;

	iTerminate = 1;

	if (e_sock != INVALID_SOCKET) closesocket(e_sock);

	if (WaitForSingleObject(hEnumeration, 300) == WAIT_TIMEOUT)
	{
		StrEvent ev;
		ev.type = sysev_enumstop;
		OnSysEvent(&ev);
		TerminateThread(hEnumeration, 0);
	}

	hEnumeration = NULL;

	return 1;
}