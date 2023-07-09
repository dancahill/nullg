/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "nullg/core_mod.h"

#define MODSHORTNAME "smtpd"

typedef struct {
	char      username[64];
	char      RemoteAddr[16];
	int       RemotePort;
	short int uid;
	short int gid;
	short int did;
	short int mailcurrent;
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
	nsp_state *N;
	char *extradata;
} CONN;

/* smtpd_auth.c functions */
int auth_login(CONN *conn, char *username, char *password, int mbox);
/* smtpd_bounce.c functions */
int bounce_send(char *from, char *rcpt, char *orig_msg, char *reason);
/* smtpd_conf.c functions */
int conf_read(void);
/* smtpd_dns.c */
char *dns_getmxbyname(char *dest, int destlen, char *domain);
/* smtpd_filter.c functions */
int filter_scan(CONN *conn, char *msgfilename);
/* smtpd_nsp.c */
int smtp_nsp_init(CONN *conn);
int smtp_nsp_prefilter(CONN *conn);
int smtp_nsp_addfilter(CONN *conn, char *reason);
int smtp_nsp_postfilter(CONN *conn);
/* smtpd_server.c functions */
void smtp_dorequest(CONN *conn);

#ifdef WIN32
unsigned _stdcall smtp_spool(void *x);
#else
void *smtp_spool(void *x);
#endif

typedef struct {
	char      smtp_interface[128];
	char      smtp_relayhost[128];
	char      smtp_hostname[128];
	short int smtp_port;
	short int smtp_sslport;
	short int smtp_msaport;
	short int require_tls;
	short int smtp_maxconn;
	short int smtp_maxidle;
	int       smtp_retrydelay;
	int       popauth_window;
	char      filter_program[256];
} MOD_CONFIG;

#ifdef SRVMOD_MAIN
	CONN *conn;
	TCP_SOCKET ListenSocketSTD; // port 25
	TCP_SOCKET ListenSocketSSL; // port 465
	TCP_SOCKET ListenSocketMSA; // port 587
	//pthread_t ListenThreadSTD;
	//pthread_t ListenThreadSSL;
	pthread_t SpoolThread;
	pthread_mutex_t ListenerMutex;
	MOD_CONFIG mod_config;
#else
	extern CONN *conn;
	extern TCP_SOCKET ListenSocketSTD;
	extern TCP_SOCKET ListenSocketSSL;
	extern TCP_SOCKET ListenSocketMSA;
	//extern pthread_t ListenThreadSTD;
	//extern pthread_t ListenThreadSSL;
	extern pthread_t SpoolThread;
	extern pthread_mutex_t ListenerMutex;
	extern MOD_CONFIG mod_config;
#endif
