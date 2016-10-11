/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#include "pop3d_main.h"
#ifndef WIN32
#include <dirent.h>
#include <sys/types.h>
#endif

typedef struct {
	int localid;
	int deleted;
	int filesize;
	char filename[512];
	char uidl[100];
} MDIRENT;

typedef struct {
	int mboxtotal;
	int mboxcount;
	int lastmsg;
	MDIRENT **msg;
} MDIR;

static void pop3_stat(CONN *conn, MDIR *mdir)
{
	int i;
	int size;

	size = 0;
	for (i = 0;i < mdir->mboxtotal;i++) {
		if (mdir->msg[i]->deleted) continue;
		size += mdir->msg[i]->filesize;
	}
	tcp_fprintf(&conn->socket, "+OK %d %d\r\n", mdir->mboxcount, size);
	return;
}

static void pop3_list(CONN *conn, MDIR *mdir, char *line)
{
	char *ptemp;
	int msg = -1;
	int size;
	int i;

	ptemp = line + 4;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	if (isdigit(*ptemp)) msg = atoi(ptemp);
	if ((msg > 0) && (msg < mdir->mboxtotal + 1)) {
		if (mdir->msg[msg - 1]->deleted) {
			tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
			return;
		}
		tcp_fprintf(&conn->socket, "+OK %d %d\r\n", msg, mdir->msg[msg - 1]->filesize);
	}
	else if (msg == -1) {
		size = 0;
		for (i = 0;i < mdir->mboxtotal;i++) {
			if (mdir->msg[i]->deleted) continue;
			size += mdir->msg[i]->filesize;
		}
		tcp_fprintf(&conn->socket, "+OK %d messages (%d octets)\r\n", mdir->mboxcount, size);
		for (i = 0;i < mdir->mboxtotal;i++) {
			if (mdir->msg[i]->deleted) continue;
			tcp_fprintf(&conn->socket, "%d %d\r\n", i + 1, mdir->msg[i]->filesize);
		}
		tcp_fprintf(&conn->socket, ".\r\n");
	}
	else {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
	}
	return;
}

static void pop3_retr(CONN *conn, MDIR *mdir, char *line)
{
	FILE *fp;
	char *ptemp;
	char buffer[1024];
	char outbuffer[1024];
	unsigned int outlen;
	unsigned int sublen;
	int msg = 0;

	ptemp = line + 4;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	if (isdigit(*ptemp)) msg = atoi(ptemp);
	if ((msg<1) || (msg>mdir->mboxtotal)) {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
		return;
	}
	if (mdir->msg[msg - 1]->deleted) {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
		return;
	}
	if ((fp = fopen(mdir->msg[msg - 1]->filename, "rb")) == NULL) {
		tcp_fprintf(&conn->socket, "-ERR Error opening msg file\r\n");
		return;
	}
	tcp_fprintf(&conn->socket, "+OK %d octets\r\n", mdir->msg[msg - 1]->filesize);
	memset(buffer, 0, sizeof(buffer));
	memset(outbuffer, 0, sizeof(outbuffer));
	outlen = 0;
	for (;;) {
		if (fgets(buffer, sizeof(buffer) - 1, fp) == NULL) break;
		sublen = strlen(buffer);
		if (sublen + outlen > sizeof(outbuffer) - 2) {
			if (tcp_send(&conn->socket, outbuffer, outlen, 0) < 0) break;
			memset(outbuffer, 0, sizeof(outbuffer));
			outlen = 0;
		}
		strcat(outbuffer, buffer);
		outlen += sublen;
	}
	tcp_send(&conn->socket, outbuffer, outlen, 0);
	fclose(fp);
	tcp_fprintf(&conn->socket, ".\r\n");
	if (mdir->lastmsg < msg) mdir->lastmsg = msg;
	return;
}

static void pop3_dele(CONN *conn, MDIR *mdir, char *line)
{
	char *ptemp;
	int msg = 0;

	ptemp = line + 4;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	if (isdigit(*ptemp)) msg = atoi(ptemp);
	if ((msg<1) || (msg>mdir->mboxtotal)) {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
		return;
	}
	if (mdir->msg[msg - 1]->deleted) {
		tcp_fprintf(&conn->socket, "-ERR Message %d already deleted\r\n", msg);
		return;
	}
	mdir->msg[msg - 1]->deleted = 1;
	mdir->mboxcount--;
	if (mdir->lastmsg < msg) mdir->lastmsg = msg;
	tcp_fprintf(&conn->socket, "+OK Message %d deleted\r\n", msg);
	return;
}

static void pop3_capa(CONN *conn)
{
	tcp_fprintf(&conn->socket, "+OK Capability list follows\r\n");
	tcp_fprintf(&conn->socket, "USER\r\n");
	tcp_fprintf(&conn->socket, "TOP\r\n");
	tcp_fprintf(&conn->socket, "UIDL\r\n");
	tcp_fprintf(&conn->socket, "STLS\r\n");
	tcp_fprintf(&conn->socket, ".\r\n");
	return;
}

static void pop3_noop(CONN *conn)
{
	tcp_fprintf(&conn->socket, "+OK\r\n");
	return;
}

static void pop3_last(CONN *conn, MDIR *mdir)
{
	tcp_fprintf(&conn->socket, "+OK %d\r\n", mdir->lastmsg);
	return;
}

static void pop3_rset(CONN *conn, MDIR *mdir)
{
	int i;

	for (i = 0;i < mdir->mboxtotal;i++) {
		mdir->msg[i]->deleted = 0;
	}
	mdir->mboxcount = mdir->mboxtotal;
	tcp_fprintf(&conn->socket, "+OK\r\n");
	return;
}

static void pop3_stls(CONN *conn)
{
	if (proc->ssl_is_loaded && conn->socket.using_tls == 0) {
		tcp_fprintf(&conn->socket, "+OK\r\n");
		conn->state = 0;
		ssl_accept(&conn->socket);
	}
	else {
		tcp_fprintf(&conn->socket, "-ERR\r\n");
	}
	return;
}

static void pop3_top(CONN *conn, MDIR *mdir, char *line)
{
	FILE *fp = NULL;
	char *ptemp;
	char buffer[1024];
	int msg = 0;
	int lines = 0;
	int i;

	ptemp = line + 4;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	if (isdigit(*ptemp)) msg = atoi(ptemp);
	while (isdigit(*ptemp)) ptemp++;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	if (isdigit(*ptemp)) lines = atoi(ptemp);
	if ((msg<1) || (msg>mdir->mboxtotal)) {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
		return;
	}
	if (mdir->msg[msg - 1]->deleted) {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
		return;
	}
	if ((fp = fopen(mdir->msg[msg - 1]->filename, "rb")) == NULL) {
		tcp_fprintf(&conn->socket, "-ERR Error opening msg file\r\n");
		return;
	}
	tcp_fprintf(&conn->socket, "+OK %d octets\r\n", mdir->msg[msg - 1]->filesize);
	for (;;) {
		if (fgets(buffer, sizeof(buffer) - 1, fp) == NULL) break;
		tcp_fprintf(&conn->socket, "%s", buffer);
		striprn(buffer);
		if (strlen(buffer) == 0) break;
	}
	for (i = 0;i < lines;i++) {
		if (fgets(buffer, sizeof(buffer) - 1, fp) == NULL) break;
		tcp_fprintf(&conn->socket, "%s", buffer);
	}
	fclose(fp);
	tcp_fprintf(&conn->socket, ".\r\n");
	if (mdir->lastmsg < msg) mdir->lastmsg = msg;
	return;
}

static void pop3_uidl(CONN *conn, MDIR *mdir, char *line)
{
	char *ptemp;
	int msg = -1;
	int size;
	int i;

	ptemp = line + 4;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	if (isdigit(*ptemp)) msg = atoi(ptemp);
	if ((msg > 0) && (msg < mdir->mboxtotal + 1)) {
		if (mdir->msg[msg - 1]->deleted) {
			tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
			return;
		}
		tcp_fprintf(&conn->socket, "+OK %d %s\r\n", msg, mdir->msg[msg - 1]->uidl);
	}
	else if (msg == -1) {
		size = 0;
		for (i = 0;i < mdir->mboxtotal;i++) {
			if (mdir->msg[i]->deleted) continue;
			size += mdir->msg[i]->filesize;
		}
		tcp_fprintf(&conn->socket, "+OK %d messages (%d octets)\r\n", mdir->mboxcount, size);
		for (i = 0;i < mdir->mboxtotal;i++) {
			if (mdir->msg[i]->deleted) continue;
			tcp_fprintf(&conn->socket, "%d %s\r\n", i + 1, mdir->msg[i]->uidl);
		}
		tcp_fprintf(&conn->socket, ".\r\n");
	}
	else {
		tcp_fprintf(&conn->socket, "-ERR No such message\r\n");
	}
	return;
}

static void pop3_local(CONN *conn)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
#ifdef WIN32
	struct  _finddata_t dentry;
	long handle;
#else
	struct dirent *dentry;
	DIR *handle;
#endif
	struct stat sb;
	MDIR mdir;
	char line[128];
	char dirname[256];
#ifdef WIN32
	char dirname2[256];
#endif
	char tmpname[256];
	int i;
	int mboxalloc;

	mdir.mboxtotal = 0;
	mboxalloc = 50;
	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname) - 1, "%s/domains/%04d/mailspool/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did, conn->dat->username);
	//	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 4, "%s - dirname='%s'", conn->dat->RemoteAddr, dirname);
	if (stat(dirname, &sb) != 0) {
#ifdef WIN32
		if (mkdir(dirname) != 0) {
#else
		//		if (mkdir(dirname, ~config->umask&0777)!=0) {
		if (mkdir(dirname, ~0644 & 0777) != 0) {
#endif
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "Error creating directory '%s'", dirname);
			return;
		}
	}
	if ((mdir.msg = calloc(mboxalloc, sizeof(MDIRENT *))) == NULL) return;
#ifdef WIN32
	snprintf(dirname2, sizeof(dirname2) - 1, "%s/domains/%04d/mailspool/%s/*.*", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did, conn->dat->username);
	if ((handle = _findfirst(dirname2, &dentry)) < 0) return;
	do {
		char *name = dentry.name;
#else
	handle = opendir(dirname);
	while ((dentry = readdir(handle)) != NULL) {
		char *name = dentry->d_name;
#endif
		if (strcmp(".", name) == 0) continue;
		if (strcmp("..", name) == 0) continue;
		memset(tmpname, 0, sizeof(tmpname));
		snprintf(tmpname, sizeof(tmpname) - 1, "%s/%s", dirname, name);
		fixslashes(tmpname);
		if (stat(tmpname, &sb) != 0) continue;
		if (sb.st_mode&S_IFDIR) continue;
		if (mdir.mboxtotal + 2 > mboxalloc) {
			mboxalloc += 50;
			mdir.msg = realloc(mdir.msg, mboxalloc*sizeof(MDIRENT *));
		}
		if ((mdir.msg[mdir.mboxtotal] = calloc(1, sizeof(MDIRENT))) == NULL) break;
		mdir.msg[mdir.mboxtotal]->localid = mdir.mboxtotal + 1;
		mdir.msg[mdir.mboxtotal]->deleted = 0;
		mdir.msg[mdir.mboxtotal]->filesize = sb.st_size;
		memset(mdir.msg[mdir.mboxtotal]->filename, 0, sizeof(mdir.msg[mdir.mboxtotal]->filename));
		snprintf(mdir.msg[mdir.mboxtotal]->filename, sizeof(mdir.msg[mdir.mboxtotal]->filename) - 1, "%s", tmpname);
		memset(mdir.msg[mdir.mboxtotal]->uidl, 0, sizeof(mdir.msg[mdir.mboxtotal]->uidl));
		snprintf(mdir.msg[mdir.mboxtotal]->uidl, sizeof(mdir.msg[mdir.mboxtotal]->uidl) - 1, "%s", name);
		mdir.mboxtotal++;
#ifdef WIN32
	} while (_findnext(handle, &dentry) == 0);
	_findclose(handle);
#else
	}
	closedir(handle);
#endif


	mdir.mboxcount = mdir.mboxtotal;
	mdir.lastmsg = 0;
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) goto cleanup;
		striprn(line);
		if (strcasecmp(line, "quit") == 0) {
			tcp_fprintf(&conn->socket, "+OK Goodbye\r\n");
			break;
		}
		else if (strcasecmp(line, "capa") == 0) {
			pop3_capa(conn);
		}
		else if (strcasecmp(line, "stat") == 0) {
			pop3_stat(conn, &mdir);
		}
		else if (strncasecmp(line, "list", 4) == 0) {
			pop3_list(conn, &mdir, line);
		}
		else if (strncasecmp(line, "retr", 4) == 0) {
			pop3_retr(conn, &mdir, line);
		}
		else if (strncasecmp(line, "dele", 4) == 0) {
			pop3_dele(conn, &mdir, line);
		}
		else if (strcasecmp(line, "noop") == 0) {
			pop3_noop(conn);
		}
		else if (strcasecmp(line, "last") == 0) {
			pop3_last(conn, &mdir);
		}
		else if (strcasecmp(line, "rset") == 0) {
			pop3_rset(conn, &mdir);
		}
		else if (strncasecmp(line, "top", 3) == 0) {
			pop3_top(conn, &mdir, line);
		}
		else if (strncasecmp(line, "uidl", 4) == 0) {
			pop3_uidl(conn, &mdir, line);
		}
		else {
			tcp_fprintf(&conn->socket, "-ERR Unknown Command\r\n");
		}
	} while (1);
	for (i = 0;i < mdir.mboxtotal;i++) {
		if (mdir.msg[i]->deleted == 1) {
			unlink(mdir.msg[i]->filename);
		}
	}
cleanup:
	for (i = 0;i < mdir.mboxtotal;i++) {
		if (mdir.msg[i] != NULL) {
			free(mdir.msg[i]);
		}
	}
	free(mdir.msg);
	return;
}

static void pop3_remote(CONN *conn)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
	MDIR mdir;
	char line[128];
	obj_t *qptr = NULL;
	int i;

	if (sql_queryf(proc->N, &qptr, "SELECT mailheaderid, size, uidl FROM gw_email_headers WHERE accountid = %d AND obj_uid = %d AND folder = 1 ORDER BY hdr_date ASC", conn->dat->mailcurrent, conn->dat->uid) < 0) return;
	mdir.mboxtotal = sql_numtuples(proc->N, &qptr);
	if ((mdir.msg = calloc(mdir.mboxtotal, sizeof(MDIRENT *))) == NULL) {
		sql_freeresult(proc->N, &qptr);
		return;
	}
	for (i = 0;i < sql_numtuples(proc->N, &qptr);i++) {
		if ((mdir.msg[i] = calloc(1, sizeof(MDIRENT))) == NULL) break;
		mdir.msg[i]->localid = atoi(sql_getvalue(proc->N, &qptr, i, 0));
		mdir.msg[i]->deleted = 0;
		mdir.msg[i]->filesize = atoi(sql_getvalue(proc->N, &qptr, i, 1));
		memset(mdir.msg[i]->filename, 0, sizeof(mdir.msg[i]->filename));
		snprintf(mdir.msg[i]->filename, sizeof(mdir.msg[i]->filename) - 1, "%s/domains/%04d/%04d/%04d/%06d.msg", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), conn->dat->did, conn->dat->mailcurrent, 1, mdir.msg[i]->localid);
		fixslashes(mdir.msg[i]->filename);
		memset(mdir.msg[i]->uidl, 0, sizeof(mdir.msg[i]->uidl));
		decode_base64(mdir.msg[i]->uidl, sizeof(mdir.msg[i]->uidl) - 1, sql_getvalue(proc->N, &qptr, i, 2));
	}
	sql_freeresult(proc->N, &qptr);
	mdir.mboxcount = mdir.mboxtotal;
	mdir.lastmsg = 0;
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) goto cleanup;
		striprn(line);
		if (strcasecmp(line, "quit") == 0) {
			tcp_fprintf(&conn->socket, "+OK Goodbye\r\n");
			break;
		}
		else if (strcasecmp(line, "capa") == 0) {
			pop3_capa(conn);
		}
		else if (strcasecmp(line, "stat") == 0) {
			pop3_stat(conn, &mdir);
		}
		else if (strncasecmp(line, "list", 4) == 0) {
			pop3_list(conn, &mdir, line);
		}
		else if (strncasecmp(line, "retr", 4) == 0) {
			pop3_retr(conn, &mdir, line);
		}
		else if (strncasecmp(line, "dele", 4) == 0) {
			pop3_dele(conn, &mdir, line);
		}
		else if (strcasecmp(line, "noop") == 0) {
			pop3_noop(conn);
		}
		else if (strcasecmp(line, "last") == 0) {
			pop3_last(conn, &mdir);
		}
		else if (strcasecmp(line, "rset") == 0) {
			pop3_rset(conn, &mdir);
		}
		else if (strncasecmp(line, "top", 3) == 0) {
			pop3_top(conn, &mdir, line);
		}
		else if (strncasecmp(line, "uidl", 4) == 0) {
			pop3_uidl(conn, &mdir, line);
		}
		else {
			tcp_fprintf(&conn->socket, "-ERR Unknown Command\r\n");
		}
	} while (1);
	for (i = 0;i < mdir.mboxtotal;i++) {
		if (mdir.msg[i]->deleted == 1) {
			sql_updatef(proc->N, "UPDATE gw_email_headers SET folder = 4 WHERE accountid = %d AND obj_uid = %d AND mailheaderid = %d", conn->dat->mailcurrent, conn->dat->uid, mdir.msg[i]->localid);
		}
	}
cleanup:
	for (i = 0;i < mdir.mboxtotal;i++) {
		if (mdir.msg[i] != NULL) {
			free(mdir.msg[i]);
		}
	}
	free(mdir.msg);
	return;
}

void pop3_dorequest(CONN *conn)
{
	char curdate[32];
	char line[128];
	char username[64];
	char domain[64];
	char password[64];
	int mbox;
	char *ptemp;

	memset(username, 0, sizeof(username));
	memset(domain, 0, sizeof(domain));
	memset(password, 0, sizeof(password));
	mbox = 0;
	tcp_fprintf(&conn->socket, "+OK Welcome to %s POP3d\r\n", SERVER_NAME);
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) return;
		striprn(line);
		if (strcasecmp(line, "quit") == 0) {
			tcp_fprintf(&conn->socket, "+OK Goodbye\r\n");
			return;
		}
		else if (strcasecmp(line, "capa") == 0) {
			pop3_capa(conn);
		}
		else if (strcasecmp(line, "stls") == 0) {
			pop3_stls(conn);
		}
		else if (strncasecmp(line, "user", 4) == 0) {
			if (proc->ssl_is_loaded && conn->socket.using_tls == 0 && mod_config.require_tls == 1) {
				tcp_fprintf(&conn->socket, "-ERR TLS Required\r\n");
				continue;
			}
			ptemp = line + 4;
			while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
			snprintf(username, sizeof(username) - 1, "%s", ptemp);
			if ((ptemp = strchr(username, ':')) != NULL) {
				*ptemp++ = '\0';
				mbox = atoi(ptemp);
				if (mbox < 0) mbox = 0;
			}
			if ((ptemp = strchr(username, '@')) != NULL) {
				*ptemp++ = '\0';
				snprintf(domain, sizeof(domain) - 1, "%s", ptemp);
			}
			tcp_fprintf(&conn->socket, "+OK USER '%s' MBOX '%d'\r\n", username, mbox);
		}
		else if (strncasecmp(line, "pass", 4) == 0) {
			if (proc->ssl_is_loaded && conn->socket.using_tls == 0 && mod_config.require_tls == 1) {
				tcp_fprintf(&conn->socket, "-ERR TLS Required\r\n");
				continue;
			}
			ptemp = line + 4;
			while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
			snprintf(password, sizeof(password) - 1, "%s", ptemp);
			if (auth_login(conn, username, domain, password, mbox) == 0) {
				tcp_fprintf(&conn->socket, "+OK PASS accepted\r\n");
				break;
			}
			else {
				memset(username, 0, sizeof(username));
				memset(domain, 0, sizeof(domain));
				memset(password, 0, sizeof(password));
				mbox = 0;
				log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s - FAILED AUTH: %s @ %s", conn->dat->RemoteAddr, username, domain);
				sleep(2);
				tcp_fprintf(&conn->socket, "-ERR Bad Password\r\n");
			}
		}
		else {
			tcp_fprintf(&conn->socket, "-ERR Unknown Command\r\n");
		}
	} while (1);
	conn->state = 1;
	log_access(proc->N, "pop3d", "%s - SUCCESSFUL AUTH: %s @ %s", conn->dat->RemoteAddr, username, domain);
	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate) - 1, time(NULL));
	sql_updatef(proc->N, "INSERT INTO gw_smtp_relayrules (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, ruletype, persistence, ipaddress) values ('%s', '%s', %d, 0, %d, 'allow', 'temp', '%s')", curdate, curdate, conn->dat->did, conn->dat->did, conn->dat->RemoteAddr);
	if (mbox > 0) {
		pop3_remote(conn);
	}
	else {
		pop3_local(conn);
	}
	return;
}
