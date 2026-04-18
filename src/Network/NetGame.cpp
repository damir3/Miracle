///////////////////////////////////////////////////////////////////////////////
///  NetDll v1.05  ////////////////////////////////////////////  10.10.1999  //
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include <windows.h>
#include <crtdbg.h>

#include "netsys.h"
#include "netenum.h"
#include "netsock.h"

enum EType
{
	type_none, type_client, type_server, type_local
};

lpOnEvent		OnEvent;
lpOnSysEvent	OnSysEvent;
EType			mytype;
int				port;
int				sockinit;

int	Init(lpOnEvent oe, lpOnSysEvent ose, int iPort, int iSockInit)
{
	mytype = type_none;
	OnEvent = NULL;
	OnSysEvent = NULL;

	sockinit = iSockInit;

	if (iSockInit)
	{
		WSADATA wsaData;
		int iErr = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
		if (iErr) return iErr;
	}

	if (!oe) return 1;
	if (!ose) return 2;

	OnEvent = oe;
	OnSysEvent = ose;
	port = iPort;

	return 0;
}

int	StartServer(int max)
{
	if (mytype != type_none) return 0;
	mytype = type_server;

	if (Server(port, max) < 0) return 0;
	return 1;
}

int	Stop()
{
	switch (mytype)
	{
	case type_server: ServerStop(); break;
	case type_client: ClientStop(); break;
	case type_local:  break; // what can we do?
	}
	mytype = type_none;
	return 1;
}

void	Done()
{
	StopEnum();

	if (mytype != type_none) Stop();

	if (sockinit) WSACleanup();

	OnEvent = NULL;
	OnSysEvent = NULL;
}

int		StartClient (char *srv_ip)
{
	if (mytype != type_none) return 0;
	mytype = type_client;

	return ClientStart(port, srv_ip);	
}

int		StartLocal ()
{
	if (mytype != type_none) return 0;
	mytype = type_local;

	return 1;
}

void	Post (StrEvent *ev)
{
	switch(mytype)
	{
	case type_server: // to all
		ServerPost(ev);
		break;
	case type_client: // to server
		ClientPost(ev);
		break;
	case type_local: // to itself
		ev->num = -2; // localserver sign
		OnEvent(ev);
		break;
	}
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpReserved)
{
	return true;
}

int	GetIP()
{
	char	name[128];
	hostent	*h;

	if (!sockinit)	return	0;
	if(gethostname(name, sizeof(name))==SOCKET_ERROR)
	{
		StrEvent	ev;
		ev.type = sysev_error;
		if(WSAGetLastError () == WSAENETDOWN)
			strcpy(ev.data, "The network subsystem has failed");
		else
			strcpy(ev.data, "Can't get Host name");
		OnSysEvent(&ev);
		return	0;
	}
	h = gethostbyname(name);
	if(h==NULL)
	{
		StrEvent	ev;
		ev.type = sysev_error;
		switch(WSAGetLastError ())
		{
		case	WSAENETDOWN:
			strcpy(ev.data, "The network subsystem has failed");
			break;
		case	WSAHOST_NOT_FOUND:
			strcpy(ev.data, "Authoritative Answer Host not found");
			break;
		case	WSATRY_AGAIN:
			strcpy(ev.data, "Non-Authoritative Host not found, or server failure");
			break;
		strcpy(ev.data, "Can't get Host by name");
		return	0;
		}
		OnSysEvent(&ev);
		return	0;
	}
	return	((LPIN_ADDR)h->h_addr)->s_addr;
}