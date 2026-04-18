#ifndef NETSOCK_H_
#define NETSOCK_H_

#include "netsys.h"

int	Server(int port, int max);
void ServerStop(void);
void ServerPost(StrEvent *);

int	ClientStart(int, char *);
void ClientStop(void);
void ClientPost(StrEvent *);

#endif