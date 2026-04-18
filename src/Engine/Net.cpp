/*
	Net.cpp		Copyright (C) 1998-1999 Vitaly Ovtchinnikov
				and Damir Sagidullin
*/
#include	<Windows.h>
#include	<Stdio.h>

#include	"Config.h"
#include	"Console.h"
#include	"Main.h"
#include	"Net.h"

#define	MAX_USERS	8

static NetGame	net;
static int		gametype;//=0;
static HMODULE	hDll;
static char		*lname;


void	OnEvent(StrEvent *ev)
{
	if(ev->type)
		CPrintfMessage("Message(from <%d>)[%d]: \"%s\"", ev->num, ev->type, ev->data);
	else
		CPrintfMessage(ev->data);
}

void	OnSysEvent(StrEvent *ev)
{
	switch(ev->type)
	{
	case sysev_none:
		{
			CPrintfMessage("System event: none");
			break;
		}
	case sysev_enumstart:
		{
			CPrintfMessage("Enumeration started");
			break;
		}
	case sysev_enumstop:
		{
			CPrintfMessage("Enumeration terminated");
			break;
		}
	case sysev_enumfound:
		{
			CPrintfMessage("Enum found (data=\"%s\")", ev->data);
			break;
		}
	case sysev_enumend:
		{
			CPrintfMessage("Enum ended");
			break;
		}
	case sysev_error:
		{
			CPrintfMessage("Error: %s", ev->data);
			break;
		}
	case sysev_srvstart:
		{
			CPrintfMessage("msg: Server started");
			break;
		}
	case sysev_srvstop:
		{
			CPrintfMessage("msg: Server stopped");
			break;
		}
	case sysev_clstart:
		{
			CPrintfMessage("Client started");
			break;
		}
	case sysev_clstop: // by Stop() function
		{
			CPrintfMessage("Client stopped");
			break;
		}
	case sysev_clterm: // by detecting any error
		{
			CPrintfMessage("Client terminated");
			break;
		}
	case sysev_srvaccept:
		{
			CPrintfMessage("Server accepted connection in %d slot", ev->num);
			break;
		}
	case sysev_srvreset:
		{
			CPrintfMessage("Server reseted connection in %d slot", ev->num);
			break;
		}
	}
}
void	StopNet()
{
	if(gametype)
	{
		net.Stop();
		gametype = 0;
	}
}

void	StartLocalServer()
{
	StopNet();
	net.StartLocal();
	CPrintf("Start local server");
	gametype = 1;
}

void	StartEnumeration()
{
	CPrintf("StartEnum=%d", net.StartEnumeration());
}

void	StopEnumeration()
{
	CPrintf("StopEnum=%d", net.StopEnumeration());
}

int		SetNetProc(FARPROC *proc, char *proc_name)
{
	*proc = GetProcAddress(hDll, proc_name);
	if(!*proc)
	{
		CPrintf("Function \"%s\" not found in \"%s\"", proc_name, lname);
		return	0;
	}
	return	1;
}
int		LoadNetLibrary(char *library_name)
{
	lname = library_name;
	hDll = LoadLibrary(library_name);
	if (!hDll)
	{
		CPrintf("Unable to load library \"%s\"", library_name);
		return	0;
	}
	if(!SetNetProc((FARPROC *)&net.Init, "Init"))	return	0;
	if(!SetNetProc((FARPROC *)&net.Done, "Done"))	return	0;
	if(!SetNetProc((FARPROC *)&net.GetIP, "GetIP"))	return	0;
	if(!SetNetProc((FARPROC *)&net.StartEnumeration, "StartEnum"))	return	0;
	if(!SetNetProc((FARPROC *)&net.StopEnumeration, "StopEnum"))	return	0;
	if(!SetNetProc((FARPROC *)&net.StartServer, "StartServer"))	return	0;
	if(!SetNetProc((FARPROC *)&net.StartClient, "StartClient"))	return	0;
	if(!SetNetProc((FARPROC *)&net.StartLocal, "StartLocal"))	return	0;
	if(!SetNetProc((FARPROC *)&net.Stop, "Stop"))	return	0;
	if(!SetNetProc((FARPROC *)&net.Post, "Post"))	return	0;

	#define	SRV_PORT	207	// select your favorite number
	net.Init(OnEvent, OnSysEvent, SRV_PORT, 1);

	AddCommand("message",	1, NetMessage);
	AddCommand("connect", 1, Connect);
	AddCommand("disconnect", 0, Disconnect);
	AddCommand("startserver", 0, StartNetServer);
	AddCommand("startlocal", 0, StartLocalServer);
	AddCommand("startenum", 0, StartEnumeration);
	AddCommand("stopenum", 0, StopEnumeration);
	AddCommand("ip", 0, GetIP);

	gametype = 0;

	return	1;
}

void	StartNetServer()
{
	StopNet();
	if(!net.StartServer(MAX_USERS))
	{
		CPrintf("Can't create server");
		gametype = -1;
		StartLocalServer();
		return;
	}
	CPrintf("Server started");
	gametype = 2;
}

void	Connect()
{
	StopNet();
	if(num_cargs>1)
	{
		if(!net.StartClient(cargs[1]))
		{
			CPrintf("Can't connect to %s", cargs[1]);
			gametype = -1;
			StartLocalServer();
			return;
		}
		gametype = 3;
	}
}

void	Disconnect()
{
	if(gametype==3)
		StartLocalServer();
}

void	CloseNetLibrary()
{
	StopNet();
	net.Done();
	FreeLibrary(hDll);
}

void	NetMessage()
{
	if(num_cargs>1)
	{
		StrEvent	ev;
		ev.type = 0;
		sprintf(ev.data, "%s: %s", player_name, cargs[1]);
		net.Post(&ev);
	}
}
void	GetIP()
{
	uint	ip_address, ip[4];
	ip_address = net.GetIP();
	if(!ip_address)	return;
	ip[0] = ip_address&255;
	ip[1] = (ip_address>>8)&255;
	ip[2] = (ip_address>>16)&255;
	ip[3] = (ip_address>>24)&255;
	CPrintf("IP address is \"%d.%d.%d.%d\"", ip[0], ip[1], ip[2], ip[3]);
}