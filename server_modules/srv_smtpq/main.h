/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "srv_mod.h"

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#else
#define closesocket close
#endif

typedef struct {
	char      user_username[64];
//	char      user_token[64];
	short int user_uid;
	short int user_gid;
	short int user_did;
	short int user_mailcurrent;
//	short int user_maildefault;
} CONNDATA;
typedef struct {
	pthread_t handle;
#ifdef WIN32
	unsigned int id;
#else
	pthread_t id;
#endif
	short int state;
	TCP_SOCKET socket;
	CONNDATA *dat;
} CONN;

/* auth.c functions */
int auth_login(CONN *sid, char *username, char *password, int mbox);
/* smtpc.c functions */
int smtp_client(FILE *fp);
/* smtq.c functions */
#ifdef WIN32
unsigned _stdcall smtp_spool(void *x);
#else
void *smtp_spool(void *x);
#endif

#ifdef SRVMOD_MAIN
	CONN *conn;
	int ListenSocket;
	int ListenSocketSSL;
	pthread_t ListenThread;
	pthread_t ListenThreadSSL;
	pthread_t SpoolThread;
	pthread_mutex_t ListenerMutex;
#else
	extern CONN *conn;
	extern int ListenSocket;
	extern int ListenSocketSSL;
	extern pthread_t ListenThread;
	extern pthread_t ListenThreadSSL;
	extern pthread_t SpoolThread;
	extern pthread_mutex_t ListenerMutex;
#endif
