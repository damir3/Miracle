#include "netsock.h"
#include "netsys.h"

#include <winsock2.h>
#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>


extern lpOnSysEvent OnSysEvent;
extern lpOnEvent	OnEvent;

extern int			mytype;

extern char			str_id[];

HANDLE	hAccept;
HANDLE	*hReceive;

SOCKET	srv_socket;
SOCKET	*client_socket;
int		clients_max;

SOCKET	client_sock;
HANDLE	hClient;

SOCKET	enum_sock;
HANDLE	hEnumServer;

int	iProcTerm;
int	iMyIP;


DWORD CALLBACK	AcceptProc (LPVOID);


DWORD CALLBACK	EnumProc (LPVOID p)
{
	char	sss[128];
	gethostname(sss, sizeof(sss));
	hostent *h = gethostbyname(sss);
	iMyIP = ((LPIN_ADDR)h->h_addr)->s_addr;

	enum_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	SOCKADDR_IN	addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(*(int *)p);
	addr.sin_addr.s_addr = INADDR_ANY;

	bind(enum_sock, (LPSOCKADDR)&addr, sizeof(addr));

	while(!iProcTerm)
	{
		int		addrlen = sizeof(addr);
		if(recvfrom(enum_sock, sss, 4, 0, (LPSOCKADDR)&addr, &addrlen) == 4)
		{
			sss[4] = 0;
			if (!strcmp(sss, str_id))
				sendto(enum_sock, (char *)&iMyIP, sizeof(iMyIP), 0, (LPSOCKADDR)&addr, sizeof(addr));
		}
	}
	closesocket(enum_sock);

	return 1;
}

int	Server(int port, int max)
{
	client_socket = NULL;

	if (!max) return -1;

	sockaddr_in	srv_addr;

////////////////////////// to skip this later /////////////////////
	StrEvent	ev;
	ev.type = sysev_error;
///////////////////////////////////////////////////////////////////

	srv_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (srv_socket == INVALID_SOCKET) 
	{
		strcpy(ev.data, "Server():  socket() reports INVALID_SOCKET");
		OnSysEvent(&ev);

		return -2;
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(port);

	if (bind(srv_socket, (LPSOCKADDR)&srv_addr, sizeof(srv_addr)) == SOCKET_ERROR)
	{
		closesocket(srv_socket);

		strcpy(ev.data, "Server():  bind() reports SOCKET_ERROR");
		OnSysEvent(&ev);

		return -3;
	}

	if (listen(srv_socket, SOMAXCONN) == SOCKET_ERROR) 
	{
		closesocket(srv_socket);

		strcpy(ev.data, "Server():  listen() reports SOCKET_ERROR");
		OnSysEvent(&ev);

		return -4;
	}

	clients_max = max;
	client_socket = (SOCKET *)malloc(max*sizeof(SOCKET));
	hReceive = (HANDLE *)malloc(max*sizeof(HANDLE));
	for(int x=0; x<max; x++) 
	{
		hReceive[x] = NULL;
		client_socket[x] = NULL;
	}

	iProcTerm = 0;

	DWORD dwID;
	hAccept = CreateThread(NULL, 0, AcceptProc, NULL, 0, &dwID);
	if (!hAccept) 
	{
		strcpy(ev.data, "Server():  CreateThread() for AcceptProc() return NULL");
		OnSysEvent(&ev);
		return -5;
	}

	hEnumServer = CreateThread(NULL, 0, EnumProc, &port, 0, &dwID);
	if (!hEnumServer)
	{
		strcpy(ev.data, "Server():  CreateThread() for EnumProc() return NULL");
		OnSysEvent(&ev);
		return -6;
	}

	ev.type = sysev_srvstart;
	OnSysEvent(&ev);

	return 1;
}

void	ServerStop (void)
{
	iProcTerm = 1;


	if (enum_sock != INVALID_SOCKET) closesocket(enum_sock);
	if (srv_socket != INVALID_SOCKET) closesocket(srv_socket);

	if (hAccept)
	{
		if (WaitForSingleObject(hAccept, 300) == WAIT_TIMEOUT)
			TerminateThread(hAccept, 0);
		hAccept = NULL;
	}
	srv_socket = NULL;

	if (client_socket)
	{
		for(int x=0; x<clients_max; x++)
			if (client_socket[x]) closesocket(client_socket[x]);
		free(client_socket);
		client_socket = NULL;
	}

	if (hReceive) 
	{
		for(int x=0; x<clients_max; x++)
			if (hReceive[x]) TerminateThread(hReceive[x], 0);
		free(hReceive);
	}


	if (hEnumServer)
	{
		if (WaitForSingleObject(hEnumServer, 300) == WAIT_TIMEOUT)
			TerminateThread(hEnumServer, 0);
		hEnumServer = NULL;
	}

	StrEvent ev;
	ev.type = sysev_srvstop;
	OnSysEvent(&ev);
}

DWORD CALLBACK ReceiveProc(LPVOID num)
{
	StrEvent	ev;
	int	iTerm = 0, iMyPos = *(int *)num;

	WSAEVENT	hRequest = WSACreateEvent();

	while(!iProcTerm && !iTerm)
	{
		WSAEventSelect(client_socket[iMyPos], hRequest, FD_READ | FD_CLOSE);
		if (WSAWaitForMultipleEvents(1, &hRequest, FALSE, INFINITE, FALSE) == 0)
		{
			int			iRes;
			iRes = recv(client_socket[iMyPos], (char *)&ev, sizeof(ev), 0);
			if (iRes == sizeof(ev))
			{
				ev.num = iMyPos;
				OnEvent(&ev);
			}
			else
			{
				closesocket(client_socket[iMyPos]);
				client_socket[iMyPos] = NULL;
				iTerm = 1;
			}
		}
		WSAResetEvent(hRequest);
	}

	WSACloseEvent(hRequest);


	ev.type = sysev_srvreset;
	ev.num = iMyPos;
	OnSysEvent(&ev);

	hReceive[iMyPos] = NULL;

	return 0;
}


DWORD CALLBACK AcceptProc(LPVOID)
{
	WSAEVENT	hRequest = WSACreateEvent();
	StrEvent ev;
	SOCKET s;
	SOCKADDR_IN	addr;
	int addrlen = sizeof(addr);
	
	while(!iProcTerm)
	{
		WSAEventSelect(srv_socket, hRequest, FD_ACCEPT);
		if(WSAWaitForMultipleEvents(1, &hRequest, FALSE, INFINITE, FALSE) == 0)
		{
			s = accept(srv_socket, (LPSOCKADDR)&addr, &addrlen);
			if (s != INVALID_SOCKET)
			{
				for(int x=0; x<clients_max; x++)
					if (!client_socket[x])
					{
						client_socket[x] = s;

						ev.type = sysev_srvaccept;
						ev.num = x;
						OnSysEvent(&ev);

						hReceive[x] = CreateThread(NULL, 0, ReceiveProc, &x, 0, NULL);

						break;
					} 
					if (x == clients_max) closesocket(s);
			}
		}
		WSAResetEvent(hRequest);
	}

	WSACloseEvent(hRequest);
	hAccept = NULL;

	return 0;
}

void	ServerPost (StrEvent *ev)
{
	ev->num = -1;
	for(int x=0; x<clients_max; x++)
		if (client_socket[x])
		{
			int	iSend = send(client_socket[x], (char *)ev, sizeof(StrEvent), 0);
			if (iSend != sizeof(StrEvent))
			{
				closesocket(client_socket[x]);
				client_socket[x] = NULL;

				StrEvent	eev;
				eev.type = sysev_srvreset;
				eev.num = x;
				OnSysEvent(&eev);
			}
		}
	OnEvent(ev);
}

//////////////////////////// client's functions ////////////////////////////////

DWORD CALLBACK ClientProc(LPVOID)
{
	WSAEVENT	hRequest = WSACreateEvent();
	StrEvent ev;

	while(!iProcTerm)
	{
		WSAEventSelect(client_sock, hRequest, FD_READ | FD_CLOSE);
		if(WSAWaitForMultipleEvents(1, &hRequest, FALSE, INFINITE, FALSE) == 0)
		{
			if (recv(client_sock, (char *)&ev, sizeof(ev), 0) == sizeof(ev)) OnEvent(&ev);
			else  iProcTerm = 1;
		}
		else iProcTerm = 1;
		WSAResetEvent(hRequest);
	}

	WSACloseEvent(hRequest);

	if (client_sock) closesocket(client_sock);
	client_sock = NULL;

	ev.type = sysev_clterm;
	OnSysEvent(&ev);

	return 0;
}

int	ClientStart(int port, char *srv_ip)
{
	StrEvent	ev;

	client_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (client_sock == INVALID_SOCKET)
	{
		ev.type = sysev_error;
		strcpy(ev.data, "Client():  socket() reports INVALID_SOCKET");
		OnSysEvent(&ev);

		return 0;
	}

	SOCKADDR_IN	addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if ((addr.sin_addr.s_addr = inet_addr(srv_ip)) == INADDR_NONE)
	{
		hostent	*h;
		if (!(h = gethostbyname(srv_ip)))
		{
			ev.type = sysev_error;
			strcpy(ev.data, "Client(): gethostbyname() returns NULL");
			OnSysEvent(&ev);
			closesocket(client_sock);
			client_sock = NULL;
			return 0;
		}
		addr.sin_addr.s_addr = ((LPIN_ADDR)h->h_addr)->s_addr;
	}

	if (connect(client_sock, (LPSOCKADDR)&addr, sizeof(addr)))
	{
		closesocket(client_sock);
		ev.type = sysev_error;
		strcpy(ev.data, "Client():  connect() returns nonzero value");
		OnSysEvent(&ev);
		return 0;
	}

	iProcTerm = 0;

	DWORD	dwID;
	if(!(hClient = CreateThread(NULL, 0, ClientProc, NULL, 0, &dwID)))
	{
		closesocket(client_sock);

		ev.type = sysev_error;
		strcpy(ev.data, "Client(): CreateThread() for ClientProc() returns NULL");
		OnSysEvent(&ev);
		return 0;
	}

	ev.type = sysev_clstart;
	OnSysEvent (&ev);

	return 1;
}

void	ClientStop (void)
{
	StrEvent	ev;

	iProcTerm = 1;

	if (client_sock) closesocket (client_sock);
	client_socket = NULL;

	if (hClient)
	{
		if (WaitForSingleObject (hClient, 300) == WAIT_TIMEOUT)
			TerminateThread (hClient, 0);
		hClient = NULL;
	}

	ev.type = sysev_clstop;
	OnSysEvent (&ev);
}

void	ClientPost (StrEvent *ev)
{
	if (!client_sock) return;
	if (send(client_sock, (char *)ev, sizeof(StrEvent), 0) != sizeof(StrEvent)) ClientStop();
}