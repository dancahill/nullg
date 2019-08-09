/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "nullgw/core/mod.h"

#define MODSHORTNAME "smtpd"

typedef struct {
	char      user_username[64];
	char      user_RemoteAddr[16];
	int       user_RemotePort;
	short int user_uid;
	short int user_gid;
	short int user_did;
	short int user_mailcurrent;
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
	char *extradata;
} CONN;

/* auth.c functions */
int auth_login(CONN *sid, char *username, char *password, int mbox);
/* bounce.c functions */
int bounce_send(char *from, char *rcpt, char *orig_msg, char *reason);
/* conf.c functions */
int conf_read(void);
/* filter.c functions */
int filter_scan(CONN *sid, char *msgfilename);
/* smtp.c functions */
void smtp_dorequest(CONN *sid);

#ifdef WIN32
unsigned _stdcall smtp_spool(void *x);
#else
void *smtp_spool(void *x);
#endif

typedef struct {
	char      smtp_interface[128];
	char      smtp_relayhost[128];
	short int smtp_port;
	short int smtp_sslport;
	short int smtp_maxconn;
	short int smtp_maxidle;
	int       smtp_retrydelay;
	int       popauth_window;
	char      filter_program[256];
} MOD_CONFIG;

#ifdef SRVMOD_MAIN
	CONN *conn;
	TCP_SOCKET ListenSocketSTD;
	TCP_SOCKET ListenSocketSSL;
	pthread_t ListenThreadSTD;
	pthread_t ListenThreadSSL;
	pthread_t SpoolThread;
	pthread_mutex_t ListenerMutex;
	MOD_CONFIG mod_config;
#else
	extern CONN *conn;
	extern TCP_SOCKET ListenSocketSTD;
	extern TCP_SOCKET ListenSocketSSL;
	extern pthread_t ListenThreadSTD;
	extern pthread_t ListenThreadSSL;
	extern pthread_t SpoolThread;
	extern pthread_mutex_t ListenerMutex;
	extern MOD_CONFIG mod_config;
#endif
