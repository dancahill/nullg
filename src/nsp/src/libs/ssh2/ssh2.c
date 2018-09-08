/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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
#include "nsp/nsplib.h"
#include "ssh2.h"

#ifdef HAVE_SSH2

#include <libssh2.h>
#include <libssh2_sftp.h>

typedef struct {
	/* standard header info for CDATA object */
	char      obj_type[16]; /* tell us all about yourself in 15 characters or less */
	NSP_CFREE obj_term;     /* now tell us how to kill you */
	/* now begin the stuff that's libssh-specific */
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *sh_channel;
	LIBSSH2_SFTP    *sftp_session;
	int socket;
	char LocalAddr[16];
	int  LocalPort;
	char RemoteAddr[16];
	int  RemotePort;
	char HostKey[16];
	//	short int want_close;
} SSH_CONN;

#define MAX_SFTP_READ_SIZE 65536
#define MAX_SFTP_WRITE_SIZE 65536

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <io.h>
#elif !defined( __TURBOC__)
#include <unistd.h>
#endif

#ifdef WIN32
#define snprintf _snprintf
#define sleep(x) Sleep(x*1000)
#define msleep(x) Sleep(x)
#define strcasecmp stricmp
typedef int socklen_t;
#else
#include <unistd.h>
#include <utime.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define closesocket close
#define msleep(x) usleep(x*1000)
#ifdef MISSING_SOCKLEN
typedef int socklen_t;
#endif
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXWAIT 10

#ifndef O_BINARY
#define O_BINARY 0
#endif

static int ssh_close(nsp_state *N, SSH_CONN *sshconn);

/*
 * this is the function that terminates orphans
 */
void ssh_murder(nsp_state *N, obj_t *cobj)
{
#define __FN__ __FILE__ ":ssh_murder()"
	SSH_CONN *sshconn;

	n_warn(N, __FN__, "reaper is claiming another lost soul");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn != NULL) {
		ssh_close(N, sshconn);
		n_free(N, (void *)&cobj->val->d.str, sizeof(SSH_CONN) + 1);
	}
	return;
#undef __FN__
}

static void ssh_clearerr(nsp_state *N)
{
	nsp_setstr(N, nsp_getobj(N, &N->l, "this"), "last_err", NULL, 0);
	return;
}

static void ssh_lasterr(nsp_state *N, SSH_CONN *sshconn)
{
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	char *err = NULL;

	if (sshconn && sshconn->session) {
		libssh2_session_last_error(sshconn->session, &err, NULL, 0);
		if (err) striprn(err);
		nsp_setstr(N, thisobj, "last_err", err, -1);
		//		if (err!=NULL) free(err);
	}
	return;
}

static void ssh_warn(nsp_state *N, const char *__FN__, SSH_CONN *sshconn)
{
	if (sshconn && sshconn->session && libssh2_session_last_errno(sshconn->session)) {
		//		n_warn(N, __FN__, "libssh2_session_last_errno() = %d", libssh2_session_last_errno(sshconn->session));
	}
	return;
}

static int setfiletime(nsp_state *N, char *fname, time_t ftime)
{
#ifdef WIN32
	static int isWinNT = -1;
	SYSTEMTIME st;
	FILETIME locft, modft;
	struct tm *loctm;
	HANDLE hFile;
	int result;

	loctm = localtime(&ftime);
	if (loctm == NULL) return -1;
	st.wYear = (WORD)loctm->tm_year + 1900;
	st.wMonth = (WORD)loctm->tm_mon + 1;
	st.wDayOfWeek = (WORD)loctm->tm_wday;
	st.wDay = (WORD)loctm->tm_mday;
	st.wHour = (WORD)loctm->tm_hour;
	st.wMinute = (WORD)loctm->tm_min;
	st.wSecond = (WORD)loctm->tm_sec;
	st.wMilliseconds = 0;
	if (!SystemTimeToFileTime(&st, &locft) || !LocalFileTimeToFileTime(&locft, &modft)) return -1;
	if (isWinNT < 0) isWinNT = (GetVersion() < 0x80000000) ? 1 : 0;
	hFile = CreateFile(fname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, (isWinNT ? FILE_FLAG_BACKUP_SEMANTICS : 0), NULL);
	if (hFile == INVALID_HANDLE_VALUE) return -1;
	result = SetFileTime(hFile, &modft, &modft, &modft) ? 0 : -1;
	CloseHandle(hFile);
	/*
	 * mtime on fat filesystems is only stored with a 2 second precision.
	 * thanks, microsoft.  your api didn't already have enough pitfalls.
	 */
	return result;
#else
	struct utimbuf settime;

	settime.actime = settime.modtime = ftime;
	return utime(fname, &settime);
#endif
}

static int ssh_connect(nsp_state *N, SSH_CONN *sshconn, char *host, unsigned short port)
{
#define __FN__ __FILE__ ":ssh_connect()"
	struct hostent *hp;
	struct sockaddr_in serv;
	struct sockaddr_in hostaddr;
	struct sockaddr_in peeraddr;
	const char *fingerprint;
	socklen_t fromlen;
	int rc;

	if ((hp = gethostbyname(host)) == NULL) {
		/* n_warn(N, __FN__, "Host lookup error for %s", host); */
		return -1;
	}
	nc_memset((char *)&serv, 0, sizeof(serv));
	nc_memcpy((char *)&serv.sin_addr, hp->h_addr, hp->h_length);
	serv.sin_family = hp->h_addrtype;
	serv.sin_port = htons(port);
	if ((sshconn->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2;
	if ((rc = connect(sshconn->socket, (struct sockaddr *)&serv, sizeof(serv))) < 0) {
		sshconn->LocalPort = 0;
		sshconn->RemotePort = 0;
		return -2;
	}
	fromlen = sizeof(hostaddr);
	getsockname(sshconn->socket, (struct sockaddr *)&hostaddr, &fromlen);
	nc_strncpy(sshconn->LocalAddr, inet_ntoa(hostaddr.sin_addr), sizeof(sshconn->LocalAddr) - 1);
	sshconn->LocalPort = ntohs(hostaddr.sin_port);
	fromlen = sizeof(peeraddr);
	getpeername(sshconn->socket, (struct sockaddr *)&peeraddr, &fromlen);
	nc_strncpy(sshconn->RemoteAddr, inet_ntoa(peeraddr.sin_addr), sizeof(sshconn->RemoteAddr) - 1);
	sshconn->RemotePort = ntohs(peeraddr.sin_port);
	sshconn->session = libssh2_session_init();
	if (!sshconn->session) {
		n_warn(N, __FN__, "libssh2_session_init failed");
		return -1;
	}
	sshconn->sftp_session = NULL;
	do {
		if ((rc = libssh2_session_startup(sshconn->session, sshconn->socket)) == 0) break;
		if (rc != LIBSSH2_ERROR_EAGAIN) {
			n_warn(N, __FN__, "libssh2_session_startup returned %d", rc);
			ssh_warn(N, __FN__, sshconn);
			/* n_warn(N, __FN__, "Failure establishing SSH session: %d", rc); */
			return -3;
		}
	} while (1);
	fingerprint = libssh2_hostkey_hash(sshconn->session, LIBSSH2_HOSTKEY_HASH_MD5);
	memcpy(sshconn->HostKey, fingerprint, 16);
	return 0;
#undef __FN__
}

static int sftp_connect(nsp_state *N, SSH_CONN *sshconn)
{
#define __FN__ __FILE__ ":sftp_connect()"
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_init()");
	if (sshconn == NULL) return -1;
	if (sshconn->sftp_session == NULL) {
		sshconn->sftp_session = libssh2_sftp_init(sshconn->session);
		if (sshconn->sftp_session == NULL) {
			n_warn(N, __FN__, "Unable to init SFTP session");
			ssh_close(N, sshconn);
			return -1;
		}
		libssh2_session_set_blocking(sshconn->session, 1);
	}
	return 0;
#undef __FN__
}

static int ssh_close(nsp_state *N, SSH_CONN *sshconn)
{
	if (sshconn == NULL) return 0;
	if (sshconn->session) {
		if (sshconn->sftp_session) {
			libssh2_sftp_shutdown(sshconn->sftp_session);
			sshconn->sftp_session = NULL;
		}
		libssh2_session_disconnect(sshconn->session, "Normal Shutdown, Thank you for playing");
		libssh2_session_free(sshconn->session);
		sshconn->session = NULL;
	}
	if (sshconn->socket > -1) {
		/* shutdown(x,0=recv, 1=send, 2=both) */
		shutdown(sshconn->socket, 2);
		closesocket(sshconn->socket);
		sshconn->socket = -1;
	}
	return 0;
}

/*
 * start ssh script functions here
 */
NSP_CLASSMETHOD(libnsp_net_ssh_open)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_open()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* host */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* port */
	unsigned short port = 22;
	SSH_CONN *sshconn;
	int rc;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type == NT_NUMBER) port = (unsigned short)cobj2->val->d.num;
	sshconn = n_alloc(N, sizeof(SSH_CONN) + 1, 1);
	if (sshconn == NULL) {
		n_warn(N, __FN__, "couldn't alloc %d bytes", sizeof(SSH_CONN) + 1);
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	}
	nc_strncpy(sshconn->obj_type, "ssh-conn", sizeof(sshconn->obj_type) - 1);
	sshconn->obj_term = (NSP_CFREE)ssh_murder;
	if ((rc = ssh_connect(N, sshconn, cobj1->val->d.str, port)) < 0) {
		n_free(N, (void *)&sshconn, sizeof(SSH_CONN) + 1);
		return -1;
	}
	ssh_warn(N, __FN__, sshconn);
	cobj = nsp_setcdata(N, thisobj, "connection", NULL, 0);
	cobj->val->d.str = (void *)sshconn;
	cobj->val->size = sizeof(SSH_CONN) + 1;
	nsp_setbool(N, &N->r, "", 1);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_close)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_close()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SSH_CONN *sshconn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) n_warn(N, __FN__, "this session is dead");
	if (sshconn->sh_channel != NULL) {
		libssh2_channel_close(sshconn->sh_channel);
		libssh2_channel_free(sshconn->sh_channel);
		sshconn->sh_channel = NULL;
	}
	ssh_close(N, sshconn);
	n_free(N, (void *)&cobj->val->d.str, sizeof(SSH_CONN) + 1);
	cobj->val->size = 0;
	nsp_setbool(N, thisobj, "connection", 0);
	nsp_setnum(N, &N->r, "", 0);
	ssh_warn(N, __FN__, sshconn);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_hostkey)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_hostkey()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SSH_CONN *sshconn;
	char fpbuf[50];
	uchar *p;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	p = (uchar *)sshconn->HostKey;
	memset(fpbuf, 0, sizeof(fpbuf));
	snprintf(fpbuf, sizeof(fpbuf) - 1, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
		p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]
	);
	nsp_setstr(N, &N->r, "", fpbuf, -1);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_auth)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_auth()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* user */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* pass or keyset */
	obj_t *cobj;
	SSH_CONN *sshconn;
	//	char rsapub[256];
	//	char rsaprv[256];
	//	char rsapass[64];
	char *userauthlist;
	int rc = -1;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0)) {
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	}
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if (!nsp_isstr(cobj1) || cobj1->val->d.str == NULL) {
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	}
	userauthlist = libssh2_userauth_list(sshconn->session, cobj1->val->d.str, cobj1->val->size);
	if (N->debug) n_warn(N, __FN__, "Available auth methods: %s", userauthlist);
	/* should userauthlist be free()d? */
	if (nsp_istable(cobj2)) {
		obj_t *pubfile = nsp_getobj(N, cobj2, "pub");
		obj_t *prvfile = nsp_getobj(N, cobj2, "prv");
		obj_t *pubdata = nsp_getobj(N, cobj2, "pubdata");
		obj_t *prvdata = nsp_getobj(N, cobj2, "prvdata");
		obj_t *passwd = nsp_getobj(N, cobj2, "pass");

		if (strstr(userauthlist, "publickey") == NULL) {
			n_warn(N, __FN__, "Authentication method \"publickey\" not available - Available methods: \"%s\"", userauthlist);
			nsp_setbool(N, &N->r, "", 0);
			return -1;
		}
		if (nsp_isstr(pubfile) && pubfile->val->size > 0 && nsp_isstr(prvfile) && prvfile->val->size > 0) {
			rc = libssh2_userauth_publickey_fromfile(sshconn->session, cobj1->val->d.str, pubfile->val->d.str, prvfile->val->d.str, passwd->val->d.str);
		}
		else if (nsp_isstr(pubdata) && pubdata->val->size > 0 && nsp_isstr(prvdata) && prvdata->val->size > 0) {
			rc = libssh2_userauth_publickey_frommemory(sshconn->session, cobj1->val->d.str, cobj1->val->size, pubdata->val->d.str, pubdata->val->size, prvdata->val->d.str, prvdata->val->size, passwd->val->d.str);
		}
		else {
			n_error(N, NE_SYNTAX, __FN__, "invalid key data");
		}
		if (!nsp_isstr(passwd) || passwd->val->size == 0) {
			n_error(N, NE_SYNTAX, __FN__, "missing private key password");
		}
		if (rc) {
			ssh_lasterr(N, sshconn);
			n_warn(N, __FN__, "Authentication by public key failed");
			nsp_setbool(N, &N->r, "", 0);
			return -1;
		}
	}
	else if (nsp_isstr(cobj2) && cobj2->val->d.str != NULL) {
		if (strstr(userauthlist, "password") == NULL) {
			n_warn(N, __FN__, "Authentication method \"password\" not available - Available methods: \"%s\"", userauthlist);
			nsp_setbool(N, &N->r, "", 0);
			return -1;
		}
		/* auth via password */
		rc = libssh2_userauth_password(sshconn->session, cobj1->val->d.str, cobj2->val->d.str);
		if (rc) {
			ssh_lasterr(N, sshconn);
			n_warn(N, __FN__, "Authentication by password failed");
			nsp_setbool(N, &N->r, "", 0);
			return -1;
		}
	}
	else {
		n_error(N, NE_SYNTAX, __FN__, "expected a string or table for arg3");
	}
	nsp_setbool(N, &N->r, "", 1);
	ssh_warn(N, __FN__, sshconn);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_authenticated)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_authenticated()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	SSH_CONN *sshconn;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	nsp_setbool(N, &N->r, "", libssh2_userauth_authenticated(sshconn->session));
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_cmd)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_cmd()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* command */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* max milliseconds*10 to wait */
	obj_t *cobj;
	SSH_CONN *sshconn;
	int rc = 0;
	char buf[512];
	unsigned short retries;
	unsigned short maxtries = 20;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type == NT_NUMBER) maxtries = (unsigned short)cobj2->val->d.num;
	cobj = nsp_setstr(N, &N->r, "", NULL, 0);
	nsp_strcat(N, cobj1, "\n", 1);
	if (sshconn->sh_channel == NULL) {
		if (!(sshconn->sh_channel = libssh2_channel_open_session(sshconn->session))) {
			n_warn(N, __FN__, "Unable to open a channel");
			ssh_close(N, sshconn);
			return -1;
		}
		/* libssh2_channel_setenv(channel, "FOO", "bar"); */
		if (libssh2_channel_request_pty(sshconn->sh_channel, "vanilla")) {
			n_warn(N, __FN__, "Failed requesting pty");
			libssh2_channel_free(sshconn->sh_channel);
			sshconn->sh_channel = NULL;
			ssh_close(N, sshconn);
			return -1;
		}
		if (libssh2_channel_shell(sshconn->sh_channel)) {
			n_warn(N, __FN__, "Unable to request shell on allocated pty");
			libssh2_channel_free(sshconn->sh_channel);
			sshconn->sh_channel = NULL;
			ssh_close(N, sshconn);
			return -1;
		}
		libssh2_session_set_blocking(sshconn->session, 0);
		/* eat the preamble */
		retries = 0;
		do {
			memset(buf, 0, sizeof(buf));
			rc = libssh2_channel_read(sshconn->sh_channel, buf, sizeof(buf) - 1);
			if (rc == LIBSSH2_ERROR_EAGAIN&&retries < maxtries) {
				msleep(MAXWAIT);
				retries++;
				continue;
			}
			if (rc < 0) break;
			retries = 0;
		} while (1);
		libssh2_session_set_blocking(sshconn->session, 1);
	}
	/* At this point the shell can be interacted with using
	 * libssh2_channel_read()
	 * libssh2_channel_read_stderr()
	 * libssh2_channel_write()
	 * libssh2_channel_write_stderr()
	 *
	 * Blocking mode may be (en|dis)abled with: libssh2_channel_set_blocking()
	 * If the server send EOF, libssh2_channel_eof() will return non-0
	 * To send EOF to the server use: libssh2_channel_send_eof()
	 * A channel can be closed with: libssh2_channel_close()
	 * A channel can be freed with: libssh2_channel_free()
	 */
	libssh2_session_set_blocking(sshconn->session, 0);
	libssh2_channel_write(sshconn->sh_channel, cobj1->val->d.str, cobj1->val->size);
	retries = 0;
	do {
		memset(buf, 0, sizeof(buf));
		rc = libssh2_channel_read(sshconn->sh_channel, buf, sizeof(buf) - 1);
		if (rc == LIBSSH2_ERROR_EAGAIN&&retries < maxtries) {
			msleep(MAXWAIT);
			retries++;
			continue;
		}
		if (rc < 0) break;
		retries = 0;
		nsp_strcat(N, cobj, buf, rc);
	} while (1);
	libssh2_session_set_blocking(sshconn->session, 1);
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_get)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_get()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote file */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* local file */
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3"); /* progress meter */
	SSH_CONN *sshconn;
	LIBSSH2_SFTP_HANDLE *handle;
	LIBSSH2_SFTP_ATTRIBUTES attrs;
	//	LIBSSH2_CHANNEL *channel;
	struct stat fileinfo;
	off_t got = 0;
	char *p;
	int rc = 0;
	short ppmeter = 0;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj3->val->type == NT_NUMBER || cobj3->val->type == NT_BOOLEAN) ppmeter = (short)cobj3->val->d.num ? 1 : 0;

	if ((rc = sftp_connect(N, sshconn)) != 0) {
		n_warn(N, __FN__, "sftp_connect failed %d", rc);
		return -1;
	}
	memset((char *)&fileinfo, 0, sizeof(fileinfo));
	rc = libssh2_sftp_stat(sshconn->sftp_session, cobj1->val->d.str, &attrs);
	if (rc == 0) {
		if (attrs.flags&LIBSSH2_SFTP_ATTR_SIZE) {
			fileinfo.st_size = (long)attrs.filesize;
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_PERMISSIONS) {
			if (attrs.permissions&LIBSSH2_SFTP_S_IFDIR) {
			}
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_ACMODTIME) {
			fileinfo.st_mtime = attrs.atime;
			fileinfo.st_mtime = attrs.mtime;
		}
	}
	handle = libssh2_sftp_open(sshconn->sftp_session, cobj1->val->d.str, LIBSSH2_FXF_READ, 0);
	if (!handle) {
		n_warn(N, __FN__, "Unable to open an sftp handle %d", libssh2_session_last_errno(sshconn->session));
		ssh_lasterr(N, sshconn);
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "SFTP session waiting to receive file");
	if ((cobj2->val->type == NT_STRING) && (cobj2->val->d.str != NULL)) {
		/* downloading to a file */
		//char dlbuf[8192];
		char *buf;
		int fd, max;
		int pp = 0, op = 5;

		if ((fd = open(cobj2->val->d.str, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE)) == -1) {
			n_warn(N, __FN__, "could not open file");
			libssh2_sftp_close(handle);
			ssh_close(N, sshconn);
			return -1;
		}
		buf = n_alloc(N, MAX_SFTP_READ_SIZE, 1);
		while (got < fileinfo.st_size) {
			max = fileinfo.st_size - got < MAX_SFTP_READ_SIZE ? fileinfo.st_size - got : MAX_SFTP_READ_SIZE;
			rc = libssh2_sftp_read(handle, buf, max);
			if (rc < 1) {
				n_warn(N, __FN__, "libssh2_channel_read() failed: %d", rc);
				libssh2_sftp_close(handle);
				ssh_close(N, sshconn);
				close(fd);
				return -1;
			}
			if (ppmeter) {
				if ((pp = (int)((float)got / (float)fileinfo.st_size*100.0F)) > op) printf("\r%d%%", (int)pp);
				op = pp;
			}
			write(fd, buf, rc);
			got += rc;
		}
		n_free(N, (void *)&buf, MAX_SFTP_READ_SIZE);
		close(fd);
		libssh2_sftp_close(handle);
		setfiletime(N, cobj2->val->d.str, fileinfo.st_mtime);
		//		printf("\r\n%s %d %d\r\n", cobj3->val->d.str, fileinfo.st_atime, fileinfo.st_mtime);
		if (ppmeter) printf("\r%d%%\r", 100);
		nsp_setnum(N, &N->r, "", fileinfo.st_size);
	}
	else {
		/* downloading to mem */
		obj_t *cobj = &N->r;

		nsp_setstr(N, cobj, "", NULL, 0);
		if ((cobj->val->d.str = n_alloc(N, fileinfo.st_size + 1, 1)) == NULL) {
			n_warn(N, __FN__, "OUT OF MEMORY");
			libssh2_sftp_close(handle);
			ssh_close(N, sshconn);
			return -1;
		}
		while (got < fileinfo.st_size) {
			p = cobj->val->d.str + got;
			rc = libssh2_sftp_read(handle, p, fileinfo.st_size - got);
			if (rc < 1) {
				n_warn(N, __FN__, "libssh2_channel_read() failed: %d", rc);
				nsp_unlinkval(N, cobj);
				break;
			}
			got += rc;
			cobj->val->size = got;
		}
	}
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_ls)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_ls()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote dir */
	obj_t *cobj;
	obj_t tobj;
	obj_t *tobj2;
	SSH_CONN *sshconn;
	LIBSSH2_SFTP_HANDLE *sftp_handle;
	int rc = 0;
	int sym;
	LIBSSH2_SFTP_ATTRIBUTES attrs;
	char rfname[512];
	char lstext[512];

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (sftp_connect(N, sshconn) != 0) {
		n_warn(N, __FN__, "sftp_connect failed");
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_opendir()");
	sftp_handle = libssh2_sftp_opendir(sshconn->sftp_session, cobj1->val->d.str);
	if (!sftp_handle) {
		n_warn(N, __FN__, "Unable to open dir with SFTP");
		libssh2_sftp_closedir(sftp_handle);
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_opendir() is done, now receive listing");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;
	do {
		if ((rc = libssh2_sftp_readdir_ex(sftp_handle, rfname, sizeof(rfname), lstext, sizeof(lstext), &attrs)) < 1) {
			if (rc == LIBSSH2_ERROR_EAGAIN) continue;
			if (rc < 0) {
				n_warn(N, __FN__, "libssh2_sftp_readdir_ex returned %d", rc);
				ssh_warn(N, __FN__, sshconn);
			}
			break;
		}
		tobj2 = nsp_settable(N, &tobj, rfname);
		nsp_setstr(N, tobj2, "name", rfname, -1);
		sym = 0;
		if (attrs.flags&LIBSSH2_SFTP_ATTR_SIZE) {
			nsp_setnum(N, tobj2, "size", (unsigned long)attrs.filesize);
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_UIDGID) {
			nsp_setnum(N, tobj2, "uid", attrs.uid);
			nsp_setnum(N, tobj2, "gid", attrs.gid);
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_PERMISSIONS) {
			if (!(~attrs.permissions&LIBSSH2_SFTP_S_IFLNK)) sym = 1;
			if (attrs.permissions&LIBSSH2_SFTP_S_IFDIR) {
				nsp_setnum(N, tobj2, "size", 0);
				nsp_setstr(N, tobj2, "type", sym ? "dirp" : "dir", sym ? 4 : 3);
			}
			else {
				nsp_setstr(N, tobj2, "type", sym ? "filep" : "file", sym ? 5 : 4);
			}
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_ACMODTIME) {
			nsp_setnum(N, tobj2, "atime", attrs.atime);
			nsp_setnum(N, tobj2, "mtime", attrs.mtime);
		}
	} while (1);
	rc = libssh2_sftp_closedir(sftp_handle);
	if (rc < 0 && rc != LIBSSH2_ERROR_EAGAIN) {
		n_warn(N, __FN__, "libssh2_sftp_closedir returned %d", rc);
		ssh_warn(N, __FN__, sshconn);
	}
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_mkdir)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_mkdir()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote dir */
	obj_t *cobj;
	SSH_CONN *sshconn;
	int rc = 0;

	//	LIBSSH2_SFTP_ATTRIBUTES attrs;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if ((rc = sftp_connect(N, sshconn)) != 0) {
		n_warn(N, __FN__, "sftp_connect failed %d", rc);
		return -1;
	}
	if (sshconn->sftp_session == NULL) {
		n_warn(N, __FN__, "this sftp_session is dead");
		return -1;
	}
	//	ssh_warn(N, __FN__, sshconn);
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_mkdir()");
	//	rc=libssh2_sftp_mkdir(sshconn->sftp_session, cobj2->val->d.str, LIBSSH2_SFTP_S_IRWXU|LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IXGRP|LIBSSH2_SFTP_S_IROTH|LIBSSH2_SFTP_S_IXOTH);


	//	rc=libssh2_sftp_stat(sshconn->sftp_session, cobj1->val->d.str, &attrs);
	//	n_warn(N, __FN__, "libssh2_sftp_stat = %d", rc);


	rc = libssh2_sftp_mkdir_ex(sshconn->sftp_session, cobj1->val->d.str, cobj1->val->size, 0755);
	if (rc == LIBSSH2_ERROR_SFTP_PROTOCOL) {
		n_warn(N, __FN__, "LIBSSH2_ERROR_SFTP_PROTOCOL");
		ssh_lasterr(N, sshconn);
	}
	else if (rc) {
		//		n_warn(N, __FN__, "%d", rc);
	}
	nsp_setnum(N, &N->r, "", rc);
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_put)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_put()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	char *fname = nsp_getstr(N, &N->l, "0");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote file */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* string */
	SSH_CONN *sshconn;
	LIBSSH2_SFTP_HANDLE *handle;
	LIBSSH2_SFTP_ATTRIBUTES attrs;
	int rc = 0;
	struct stat fileinfo;
	unsigned long int nread;
	char *ptr;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	ssh_clearerr(N);
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING)
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if ((rc = sftp_connect(N, sshconn)) != 0) {
		n_warn(N, __FN__, "sftp_connect failed %d", rc);
		return -1;
	}
	if (nc_strcmp(fname, "put_mem") == 0) {
		fileinfo.st_size = cobj2->val->size;
		fileinfo.st_mode = LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH;
		handle = libssh2_sftp_open(sshconn->sftp_session, cobj1->val->d.str, LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT, fileinfo.st_mode & 0777);
		if (!handle) {
			n_warn(N, __FN__, "Unable to open an sftp handle %d", libssh2_session_last_errno(sshconn->session));
			ssh_lasterr(N, sshconn);
			return -1;
		}
		if (N->debug) n_warn(N, __FN__, "SFTP session waiting to send file");
		nread = cobj2->val->size;
		ptr = cobj2->val->d.str;
		do {
			/* write data in a loop until we block */
			rc = libssh2_sftp_write(handle, ptr, nread);
			if (rc > 0) {
				ptr += rc;
				nread -= rc;
			}
		} while (rc > 0);
		libssh2_sftp_close(handle);
	}
	else {
		char *buf;
		int fd;

		if (stat(cobj2->val->d.str, &fileinfo) != 0) {
			nsp_setnum(N, &N->r, "", -1);
			return -1;
		}
		if ((fd = open(cobj2->val->d.str, O_RDONLY | O_BINARY)) == -1) {
			nsp_setnum(N, &N->r, "", -1);
			return -1;
		}
		fileinfo.st_mode = LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH;
		handle = libssh2_sftp_open(sshconn->sftp_session, cobj1->val->d.str, LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT, fileinfo.st_mode & 0777);
		//		printf("\r\n%s %d %d\r\n", cobj3->val->d.str, sb.st_mtime, sb.st_atime);

		if (!handle && (libssh2_session_last_errno(sshconn->session) != LIBSSH2_ERROR_EAGAIN)) {
			n_warn(N, __FN__, "libssh2_sftp_last_error = %d", libssh2_sftp_last_error(sshconn->sftp_session));
			n_warn(N, __FN__, "libssh2_session_last_errno = %d", libssh2_session_last_errno(sshconn->session));
		}

		if (!handle) {
			n_warn(N, __FN__, "Unable to open an sftp handle %d", libssh2_session_last_errno(sshconn->session));
			ssh_lasterr(N, sshconn);
			close(fd);
			return -1;
		}
		if (N->debug) n_warn(N, __FN__, "SFTP session waiting to send file");
		buf = n_alloc(N, MAX_SFTP_WRITE_SIZE, 1);
		do {
			ptr = buf;
			if ((nread = read(fd, buf, MAX_SFTP_WRITE_SIZE)) < 1) {
				break;
			}
			/* write data in a loop until we block */
			while (nread > 0) {
				rc = libssh2_sftp_write(handle, ptr, nread);
				if (rc > 0) {
					ptr += rc;
					nread -= rc;
				}
			}
		} while (1);
		n_free(N, (void *)&buf, MAX_SFTP_WRITE_SIZE);
		close(fd);
		libssh2_sftp_close(handle);

		//rc=libssh2_sftp_stat(sshconn->sftp_session, cobj2->val->d.str, &attrs);

		attrs.flags = LIBSSH2_SFTP_ATTR_ACMODTIME | LIBSSH2_SFTP_ATTR_PERMISSIONS;
		attrs.atime = (unsigned long)fileinfo.st_atime;
		attrs.mtime = (unsigned long)fileinfo.st_mtime;
		attrs.permissions = 0644;
		while ((rc = libssh2_sftp_setstat(sshconn->sftp_session, cobj1->val->d.str, &attrs)) == LIBSSH2_ERROR_EAGAIN);
	}
	nsp_setnum(N, &N->r, "", rc);
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_rename)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_rename()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* source */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* dest */
	obj_t *cobj;
	SSH_CONN *sshconn;
	int rc = 0;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (sftp_connect(N, sshconn) != 0) {
		n_warn(N, __FN__, "sftp_connect failed");
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_rename()");
	rc = libssh2_sftp_rename(sshconn->sftp_session, cobj1->val->d.str, cobj2->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_rmdir)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_rmdir()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote dir */
	SSH_CONN *sshconn;
	int rc = 0;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (sftp_connect(N, sshconn) != 0) {
		n_warn(N, __FN__, "sftp_connect failed");
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_rmdir()");
	rc = libssh2_sftp_rmdir(sshconn->sftp_session, cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_stat)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_stat()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote file/dir */
	SSH_CONN *sshconn;
	LIBSSH2_SFTP_ATTRIBUTES attrs;
	int rc = 0;
	obj_t tobj;
	int sym;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (sftp_connect(N, sshconn) != 0) {
		n_warn(N, __FN__, "sftp_connect failed");
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_stat()");
	rc = libssh2_sftp_stat(sshconn->sftp_session, cobj1->val->d.str, &attrs);
	if (rc == 0) {
		tobj.val = n_newval(N, NT_TABLE);
		nsp_setstr(N, &tobj, "name", cobj1->val->d.str, cobj1->val->size);
		sym = 0;
		if (attrs.flags&LIBSSH2_SFTP_ATTR_SIZE) {
			nsp_setnum(N, &tobj, "size", (unsigned long)attrs.filesize);
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_UIDGID) {
			nsp_setnum(N, &tobj, "uid", attrs.uid);
			nsp_setnum(N, &tobj, "gid", attrs.gid);
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_PERMISSIONS) {
			if (!(~attrs.permissions&LIBSSH2_SFTP_S_IFLNK)) sym = 1;
			if (attrs.permissions&LIBSSH2_SFTP_S_IFDIR) {
				nsp_setnum(N, &tobj, "size", 0);
				nsp_setstr(N, &tobj, "type", sym ? "dirp" : "dir", sym ? 4 : 3);
			}
			else {
				nsp_setstr(N, &tobj, "type", sym ? "filep" : "file", sym ? 5 : 4);
			}
		}
		if (attrs.flags&LIBSSH2_SFTP_ATTR_ACMODTIME) {
			nsp_setnum(N, &tobj, "atime", attrs.atime);
			nsp_setnum(N, &tobj, "mtime", attrs.mtime);
		}
		nsp_linkval(N, &N->r, &tobj);
		nsp_unlinkval(N, &tobj);
	}
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_ssh_sftp_unlink)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_sftp_unlink()"
	obj_t *thisobj = nsp_getobj(N, &N->l, "this");
	obj_t *cobj;
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* remote file */
	SSH_CONN *sshconn;
	int rc = 0;

	cobj = nsp_getobj(N, thisobj, "connection");
	if ((cobj->val->type != NT_CDATA) || (cobj->val->d.str == NULL) || (strcmp(cobj->val->d.str, "ssh-conn") != 0))
		n_error(N, NE_SYNTAX, __FN__, "expected a ssh-conn");
	sshconn = (SSH_CONN *)cobj->val->d.str;
	if (sshconn->session == NULL) {
		n_warn(N, __FN__, "this session is dead");
		return -1;
	}
	if ((cobj1->val->type != NT_STRING) || (cobj1->val->d.str == NULL))
		n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (sftp_connect(N, sshconn) != 0) {
		n_warn(N, __FN__, "sftp_connect failed");
		return -1;
	}
	if (N->debug) n_warn(N, __FN__, "libssh2_sftp_unlink()");
	rc = libssh2_sftp_unlink(sshconn->sftp_session, cobj1->val->d.str);
	nsp_setnum(N, &N->r, "", rc);
	ssh_warn(N, __FN__, sshconn);
	return rc;
#undef __FN__
}

NSP_CLASS(libnsp_net_ssh_client)
{
#define __FN__ __FILE__ ":libnsp_net_ssh_client()"
	obj_t *tobj, *cobj;

	nsp_setcfunc(N, &N->l, "open", (NSP_CFUNC)libnsp_net_ssh_open);
	nsp_setcfunc(N, &N->l, "close", (NSP_CFUNC)libnsp_net_ssh_close);
	nsp_setcfunc(N, &N->l, "hostkey", (NSP_CFUNC)libnsp_net_ssh_hostkey);
	nsp_setcfunc(N, &N->l, "auth", (NSP_CFUNC)libnsp_net_ssh_auth);
	nsp_setcfunc(N, &N->l, "authenticated", (NSP_CFUNC)libnsp_net_ssh_authenticated);
	nsp_setcfunc(N, &N->l, "cmd", (NSP_CFUNC)libnsp_net_ssh_cmd);
	nsp_setcfunc(N, &N->l, "sftp_get", (NSP_CFUNC)libnsp_net_ssh_sftp_get);
	nsp_setcfunc(N, &N->l, "sftp_ls", (NSP_CFUNC)libnsp_net_ssh_sftp_ls);
	nsp_setcfunc(N, &N->l, "sftp_mkdir", (NSP_CFUNC)libnsp_net_ssh_sftp_mkdir);
	nsp_setcfunc(N, &N->l, "sftp_put", (NSP_CFUNC)libnsp_net_ssh_sftp_put);
	nsp_setcfunc(N, &N->l, "sftp_put_mem", (NSP_CFUNC)libnsp_net_ssh_sftp_put);
	nsp_setcfunc(N, &N->l, "sftp_rename", (NSP_CFUNC)libnsp_net_ssh_sftp_rename);
	nsp_setcfunc(N, &N->l, "sftp_rmdir", (NSP_CFUNC)libnsp_net_ssh_sftp_rmdir);
	nsp_setcfunc(N, &N->l, "sftp_stat", (NSP_CFUNC)libnsp_net_ssh_sftp_stat);
	nsp_setcfunc(N, &N->l, "sftp_unlink", (NSP_CFUNC)libnsp_net_ssh_sftp_unlink);
	nsp_setstr(N, &N->l, "host", "localhost", 9);
	nsp_setnum(N, &N->l, "port", 22);
	nsp_setstr(N, &N->l, "username", "root", 4);
	nsp_setstr(N, &N->l, "password", "", 0);
	nsp_setbool(N, &N->l, "connection", 0);
	if (nsp_istable((tobj = nsp_getobj(N, &N->l, "1")))) {
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "username")))) {
			nsp_setstr(N, &N->l, "username", cobj->val->d.str, cobj->val->size);
		}
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "password")))) {
			nsp_setstr(N, &N->l, "password", cobj->val->d.str, cobj->val->size);
		}
		if (nsp_isstr((cobj = nsp_getobj(N, tobj, "host")))) {
			nsp_setstr(N, &N->l, "host", cobj->val->d.str, cobj->val->size);
		}
	}
	return 0;
#undef __FN__
}

int nspssh2_register_all(nsp_state *N)
{
	obj_t *tobj;
#ifdef WIN32
	static WSADATA wsaData;
	if (WSAStartup(0x101, &wsaData)) return -1;
#endif
	tobj = nsp_settable(N, &N->g, "net");
	tobj->val->attr |= NST_HIDDEN;
	tobj = nsp_settable(N, tobj, "ssh");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "client", (NSP_CFUNC)libnsp_net_ssh_client);
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspssh2_register_all(N);
	return 0;
}
#endif

#endif /* HAVE_SSH2 */
