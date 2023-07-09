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

typedef struct {
	int socket;
	SSL *ssl;
	short int using_tls;
	char RemoteAddr[16];
	int  RemotePort;
	char ServerAddr[16];
	int  ServerPort;
	time_t ctime; /* Creation time */
	time_t mtime; /* Last Modified time */
	unsigned int bytes_in;
	unsigned int bytes_out;
	short int want_close;
	/* TCP INPUT BUFFER */
	short int recvbufsize;
	short int recvbufoffset;
	char      recvbuf[4096];
} TCP_SOCKET;

typedef struct {
	time_t starttime;
	int http_pages;
	int http_conns;
	int pop3_conns;
	int smtp_conns;
	int sql_queries;
	int sql_updates;
	int sql_handlecount;
} STATS;

typedef struct {
	char mod_name[40];
#ifdef WIN32
	HINSTANCE hinstLib;
#else
	void *hinstLib;
#endif
	void *mod_init;
	void *mod_exec;
	void *mod_cron;
	void *mod_exit;
} SRVMOD;

typedef struct {
	char modname[32];
	TCP_SOCKET *socket;
	void *fn_getconn;
	void *fn_doreq;
	int use_ssl;
} SRVLISTENER;

typedef struct {
	pthread_mutex_t SQL;
} LOCKS;

typedef struct {
	nsp_state *N;
	pthread_t DaemonThread;
	STATS stats;
	SRVMOD srvmod[MAX_MOD_FUNCTIONS+1];
	unsigned short debug;
	unsigned short ssl_is_loaded;
	SSL_CTX *ssl_ctx;
	SSL_METHOD *ssl_meth;
#ifdef WIN32
	HINSTANCE hInst;
	WSADATA wsaData;
#endif
	SRVLISTENER srvlistener[MAXLISTENERS];
} _PROC;

typedef	int (*SRVMOD_INIT)(_PROC *);
typedef	int (*SRVMOD_EXEC)(void);
