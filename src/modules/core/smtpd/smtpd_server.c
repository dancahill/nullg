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
#include "smtpd_main.h"

#ifdef WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

/*
	 The following are the SMTP commands:

	    HELO <SP> <domain> <CRLF>
	    MAIL <SP> FROM:<reverse-path> <CRLF>
	    RCPT <SP> TO:<forward-path> <CRLF>
	    DATA <CRLF>
	    RSET <CRLF>
	    SEND <SP> FROM:<reverse-path> <CRLF>
	    SOML <SP> FROM:<reverse-path> <CRLF>
	    SAML <SP> FROM:<reverse-path> <CRLF>
	    VRFY <SP> <string> <CRLF>
	    EXPN <SP> <string> <CRLF>
	    HELP [<SP> <string>] <CRLF>
	    NOOP <CRLF>
	    QUIT <CRLF>
	    TURN <CRLF>
*/

typedef struct {
	short int sender_can_relay;
	short int sender_is_local;
	short int sender_is_blank;
	short int num_rcpts;
	short int rcptalloc;
	int msgbodysize;
	char helo[1024];
	char ehlo[1024];
	char from[1024];
	char *msgbody;
	char **rcpt;
} MAILCONN;

typedef struct {
	char From[80];
	char Date[80];
	char MessageID[80];
} MSGHEADER;

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"


static void smtp_send(CONN *conn, char *line)
{
	tcp_fprintf(&conn->socket, "%s\r\n", line);
	log_access(proc->N, MODSHORTNAME, "%s:%d >> %s", conn->dat->RemoteAddr, conn->dat->RemotePort, line);
	return;
}

static short int allow_relay(CONN *conn)
{
	char curdate[32];
	obj_t *qptr = NULL;
	int allowed = 0;
	int denied = 0;
	int i;

	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate) - 1, time(NULL) - mod_config.popauth_window);
	sql_updatef(proc->N, "DELETE FROM gw_smtp_relayrules WHERE persistence <> 'perm' AND obj_mtime < '%s'", curdate);
	//log_access(proc->N, MODSHORTNAME, "%s:%d -- rule purge temporarily disabled!!", conn->dat->RemoteAddr, conn->dat->RemotePort);
	if (sql_queryf(proc->N, &qptr, "SELECT ipaddress, ruletype FROM gw_smtp_relayrules WHERE ipaddress = '%s' AND (persistence = 'perm' OR obj_mtime > '%s') GROUP BY ipaddress, ruletype ORDER BY ruletype DESC", conn->dat->RemoteAddr, curdate) < 0) {
		log_access(proc->N, MODSHORTNAME, "%s:%d -- failed rule query", conn->dat->RemoteAddr, conn->dat->RemotePort);
		return 0;
	}
	for (i = 0;i < sql_numtuples(proc->N, &qptr);i++) {
		if (strcmp(sql_getvaluebyname(proc->N, &qptr, i, "ruletype"), "allow") == 0) {
			log_access(proc->N, MODSHORTNAME, "%s:%d -- %s - found rule 'allowed'", conn->dat->RemoteAddr, conn->dat->RemotePort, conn->dat->RemoteAddr);
			allowed = 1;
		}
		else if (strcmp(sql_getvaluebyname(proc->N, &qptr, i, "ruletype"), "deny") == 0) {
			log_access(proc->N, MODSHORTNAME, "%s:%d -- %s - found rule 'denied'", conn->dat->RemoteAddr, conn->dat->RemotePort, conn->dat->RemoteAddr);
			denied = 1;
		}
	}
	sql_freeresult(proc->N, &qptr);
	if (denied) return -1;
	if (allowed) return 1;
	return 0;
}

static int smtp_accept(CONN *conn, MAILCONN *mconn)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
	obj_t *hostnameobj;
	FILE *fp;
	struct stat sb;
	char tmpname1[256];
	char tmpname2[256];
	char tmpaddr[128];
	char *host;
	char *buf_ptr;
	int i;
	int localdomainid;
	int userid;
	obj_t *qptr = NULL;
	int is_remote;
	int err;
	struct timeval ttime;
	struct timezone tzone;
	int blocksize;
	int bytes;
	char curdate1[40];
	char curdate2[40];
	time_t t = time(NULL);
	obj_t *reqobj = nsp_getobj(conn->N, &conn->N->g, "_REQUEST");
	obj_t *recobj = nsp_getobj(conn->N, reqobj, "RECIPIENTS");

	char namebuf[MAX_OBJNAMELEN + 1];

	hostnameobj = nsp_getobj(proc->N, nsp_getobj(proc->N, nsp_getobj(proc->N, confobj, "modules"), "smtpd"), "host_name");
	if (nsp_isnull(hostnameobj)) hostnameobj = nsp_getobj(proc->N, confobj, "host_name");

	memset(curdate1, 0, sizeof(curdate1));
	memset(curdate2, 0, sizeof(curdate2));
	strftime(curdate1, sizeof(curdate1), "%a, %d %b %Y %H:%M:%S +0000", gmtime(&t));
	time_unix2sql(curdate2, sizeof(curdate2) - 1, t);

	nsp_setstr(conn->N, reqobj, "FROM", mconn->from, -1);

	for (i = 0;i < mconn->num_rcpts;i++) {
		is_remote = 0;
		snprintf(tmpaddr, sizeof(tmpaddr) - 1, "%s", mconn->rcpt[i]);

		//nsp_setstr(conn->N, recobj, n_ntoa(conn->N, namebuf, i, 10, 0), mconn->rcpt[i], -1);
		snprintf(namebuf, sizeof(namebuf) - 1, "%d", i);
		nsp_setstr(conn->N, recobj, namebuf, mconn->rcpt[i], -1);

		if ((host = strchr(tmpaddr, '@')) != NULL) {
			*host++ = '\0';
		}
		else {
			host = "localhost";
		}
		localdomainid = domain_getid(host);
		memset(tmpname1, 0, sizeof(tmpname1));
		memset(tmpname2, 0, sizeof(tmpname2));
		if (localdomainid > 0) {
			userid = 0;

			if (sql_queryf(proc->N, &qptr, "SELECT userid, aliasname FROM gw_smtp_aliases WHERE LOWER(aliasname) = LOWER('%s') AND domainid = %d", tmpaddr, localdomainid) < 0) return -1;
			if (sql_numtuples(proc->N, &qptr) == 1) {
				obj_t *qptr2 = NULL;

				if (sql_queryf(proc->N, &qptr2, "SELECT userid, username FROM gw_users WHERE userid = %d AND domainid = %d", atoi(sql_getvalue(proc->N, &qptr, 0, 0)), localdomainid) < 0) return -1;
				if (sql_numtuples(proc->N, &qptr2) == 1) {
					userid = atoi(sql_getvalue(proc->N, &qptr2, 0, 0));
					log_access(proc->N, MODSHORTNAME, "%s:%d -- Alias %s -> '%s@%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->rcpt[i], sql_getvalue(proc->N, &qptr2, 0, 1), host);
					strcpy(tmpaddr, sql_getvalue(proc->N, &qptr2, 0, 1));
				}
				sql_freeresult(proc->N, &qptr2);
			}
			sql_freeresult(proc->N, &qptr);

			if (userid < 1) {
				if (sql_queryf(proc->N, &qptr, "SELECT userid, username FROM gw_users WHERE LOWER(username) = LOWER('%s') AND domainid = %d", tmpaddr, localdomainid) < 0) return -1;
				if (sql_numtuples(proc->N, &qptr) == 1) {
					userid = atoi(sql_getvalue(proc->N, &qptr, 0, 0));
					strcpy(tmpaddr, sql_getvalue(proc->N, &qptr, 0, 1));
				}
				sql_freeresult(proc->N, &qptr);
			}
			if (userid < 1) {
				bounce_send(mconn->from, mconn->rcpt[i], NULL, "554 no such mailbox");
				log_access(proc->N, MODSHORTNAME, "%s:%d -- no such mailbox: '%s', sent from '%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->rcpt[i], mconn->from);
				continue;
			}

			snprintf(tmpname1, sizeof(tmpname1) - 1, "%s/domains/%04d/mailspool/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), localdomainid, tmpaddr);
			if (stat(tmpname1, &sb) != 0) {
#ifdef WIN32
				if (mkdir(tmpname1) != 0) {
#else
				//				if (mkdir(tmpname1, ~config->umask&0777)!=0) {
				if (mkdir(tmpname1, ~0644 & 0777) != 0) {
#endif
					log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "Error creating directory '%s'", tmpname1);
					continue;
				}
			}
		retry1:
			gettimeofday(&ttime, &tzone);
			memset(tmpname1, 0, sizeof(tmpname1));
			memset(tmpname2, 0, sizeof(tmpname2));
			snprintf(tmpname1, sizeof(tmpname1) - 1, "%s/domains/%04d/mailspool/%s/%d%03d.msg", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), localdomainid, tmpaddr, (int)ttime.tv_sec, (int)(ttime.tv_usec / 1000));
			fixslashes(tmpname1);
			if (stat(tmpname1, &sb) == 0) goto retry1;

			nsp_setstr(conn->N, reqobj, "LOCAL_DOMAIN", host, -1);
			nsp_setstr(conn->N, reqobj, "LOCAL_MAILBOX", tmpaddr, -1);
			nsp_setstr(conn->N, reqobj, "MAILFILE", tmpname1, -1);
			nsp_setnum(conn->N, reqobj, "MAILSIZE", mconn->msgbodysize);

			log_access(proc->N, MODSHORTNAME, "%s:%d -- local delivery from:'%s', for:'%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->from, mconn->rcpt[i]);
		}
		else if (mconn->sender_can_relay == 1) {
			is_remote = 1;
		retry2:
			gettimeofday(&ttime, &tzone);
			memset(tmpname1, 0, sizeof(tmpname1));
			memset(tmpname2, 0, sizeof(tmpname2));
			snprintf(tmpname1, sizeof(tmpname1) - 1, "%s/spool/mqueue/%d%03d.msg", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), (int)ttime.tv_sec, (int)(ttime.tv_usec / 1000));
			snprintf(tmpname2, sizeof(tmpname2) - 1, "%s/spool/mqinfo/%d%03d.dat", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), (int)ttime.tv_sec, (int)(ttime.tv_usec / 1000));
			fixslashes(tmpname1);
			fixslashes(tmpname2);
			if (stat(tmpname1, &sb) == 0) goto retry2;
			if (stat(tmpname2, &sb) == 0) goto retry2;

			nsp_setstr(conn->N, reqobj, "RCPT_DOMAIN", host, -1);
			nsp_setstr(conn->N, reqobj, "MAILFILE", tmpname1, -1);
			nsp_setnum(conn->N, reqobj, "MAILSIZE", mconn->msgbodysize);

			log_access(proc->N, MODSHORTNAME, "%s:%d -- queued mail from:'%s', for:'%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->from, mconn->rcpt[i]);
		}
		else {
			memset(tmpname1, 0, sizeof(tmpname1));
			memset(tmpname2, 0, sizeof(tmpname2));
			log_access(proc->N, MODSHORTNAME, "%s:%d -- relaying denied from:'%s', for:'%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->from, mconn->rcpt[i]);
			continue;
		}
		if ((fp = fopen(tmpname1, "wb")) == NULL) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "ERROR: Cannot write to file [%s].", tmpname1);
			return -1;
		}
		fprintf(fp, "Received: from %s ([%s])\r\n", mconn->helo, conn->dat->RemoteAddr);
		fprintf(fp, "	by %s for %s;\r\n", nsp_tostr(proc->N, hostnameobj), mconn->rcpt[i]);
		fprintf(fp, "	%s\r\n", curdate1);
		buf_ptr = mconn->msgbody;
		blocksize = mconn->msgbodysize;
		do {
			if ((bytes = fwrite(buf_ptr, sizeof(char), blocksize, fp)) < 0) return -1;
			blocksize -= bytes;
			buf_ptr += bytes;
		} while (blocksize > 0);
		fclose(fp);

		//err = smtp_nsp_receive(conn);
		err = smtp_nsp_postfilter(conn);
		if (err) {
			unlink(tmpname1);
			return err;
		}
		err = filter_scan(conn, tmpname1);
		if (err > 0) {
			unlink(tmpname1);
			return err;
		}
		if (is_remote) {
			fp = fopen(tmpname2, "wb");
			if (fp == NULL) {
				unlink(tmpname1);
				continue;
			}
			fprintf(fp, "MAIL: %s\r\n", mconn->from);
			fprintf(fp, "RCPT: %s\r\n", mconn->rcpt[i]);
			fprintf(fp, "DATE: %s\r\n", curdate2);
			fclose(fp);
		}
	}
	return 0;
}

static void smtp_data(CONN *conn, MAILCONN *mconn)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
	obj_t *hostnameobj;
	MSGHEADER header;
	char curdate[40];
	char line[1024];
	char outbuf[1024];
	char *buf_ptr;
	unsigned int buf_alloc;
	short int header_done;
	int rc;
	time_t t = time(NULL);

	if ((strlen(mconn->from) == 0) && (mconn->sender_is_blank == 0)) {
		smtp_send(conn, "503 No sender");
		return;
	}
	if (mconn->num_rcpts == 0) {
		smtp_send(conn, "503 No valid recipient");
		return;
	}
	smtp_send(conn, "354 Send the mail data, end with .");
	if (mconn->msgbody) { free(mconn->msgbody); mconn->msgbody = NULL; }
	buf_alloc = 65536;
	mconn->msgbody = calloc(buf_alloc, sizeof(char));
	buf_ptr = mconn->msgbody;
	mconn->msgbodysize = 0;
	header_done = 0;
	memset((char *)&header, 0, sizeof(header));

	hostnameobj = nsp_getobj(proc->N, nsp_getobj(proc->N, nsp_getobj(proc->N, confobj, "modules"), "smtpd"), "host_name");
	if (nsp_isnull(hostnameobj)) hostnameobj = nsp_getobj(proc->N, confobj, "host_name");

	log_access(proc->N, MODSHORTNAME, "%s:%d << [...]", conn->dat->RemoteAddr, conn->dat->RemotePort);
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) return;
		if (!header_done) {
			memset(outbuf, 0, sizeof(outbuf));
			if (strncasecmp(line, "From: ", 6) == 0) {
				strncpy(header.From, (char *)&line + 6, sizeof(header.From) - 1);
				striprn(header.From);
			}
			else if (strncasecmp(line, "Date: ", 6) == 0) {
				strncpy(header.Date, (char *)&line + 6, sizeof(header.Date) - 1);
				striprn(header.Date);
			}
			else if (strncasecmp(line, "Message-ID: ", 12) == 0) {
				strncpy(header.MessageID, (char *)&line + 12, sizeof(header.MessageID) - 1);
				striprn(header.MessageID);
			}
			else if ((strcmp(line, "\r\n") == 0) || (strcmp(line, "\n") == 0) || (strcmp(line, ".\r\n") == 0) || (strcmp(line, ".\n") == 0)) {
				header_done = 1;
				if (!strlen(header.From)) {
					strncatf(outbuf, sizeof(outbuf), "From: %s\r\n", mconn->from);
				}
				if (!strlen(header.Date)) {
					strftime(curdate, sizeof(curdate), RFC1123FMT, gmtime(&t));
					strncatf(outbuf, sizeof(outbuf), "Date: %s\r\n", curdate);
					strncatf(outbuf, sizeof(outbuf), "Date-warning: Date header was inserted by %s\r\n", nsp_tostr(proc->N, hostnameobj));
				}
				if (!strlen(header.MessageID)) {
					strncatf(outbuf, sizeof(outbuf), "Message-id: <%d@%s>\r\n", t, nsp_tostr(proc->N, hostnameobj));
				}
				if (strlen(outbuf)) {
					if ((mconn->msgbodysize + strlen(outbuf) + 2) > buf_alloc) {
						buf_alloc += 131072;
						mconn->msgbody = realloc(mconn->msgbody, buf_alloc*sizeof(char));
						if (mconn->msgbody == NULL) {
							smtp_send(conn, "451 Temporary failure - try again later");
							return;
						}
					}
					buf_ptr = mconn->msgbody + mconn->msgbodysize;
					strcat(buf_ptr, outbuf);
					mconn->msgbodysize += strlen(outbuf);
				}
			}
		}
		if ((strcmp(line, ".\r\n") == 0) || (strcmp(line, ".\n") == 0)) {
			striprn(line);
			log_access(proc->N, MODSHORTNAME, "%s:%d << %s", conn->dat->RemoteAddr, conn->dat->RemotePort, line);
			break;
		}
		if ((mconn->msgbodysize + strlen(line) + 2) > buf_alloc) {
			buf_alloc += 131072;
			mconn->msgbody = realloc(mconn->msgbody, buf_alloc*sizeof(char));
			if (mconn->msgbody == NULL) {
				smtp_send(conn, "451 Temporary failure - try again later");
				return;
			}
		}
		buf_ptr = mconn->msgbody + mconn->msgbodysize;
		strcat(buf_ptr, line);
		mconn->msgbodysize += strlen(line);
	} while (1);
	rc = smtp_accept(conn, mconn);
	if (mconn->msgbody) { free(mconn->msgbody); mconn->msgbody = NULL; }
	switch (rc) {
	case 0:
		smtp_send(conn, "250 Message accepted for delivery");
		break;
	case 1:
		smtp_send(conn, "554 Message is permanently rejected (spam)");
		break;
	case 2:
	case 3:
		smtp_send(conn, "554 Message is permanently rejected (virus)");
		break;
	default:
		smtp_send(conn, "451 Temporary failure - try again later");
		break;
	}
	return;
}

static void smtp_from(CONN *conn, MAILCONN *mconn, char *line)
{
	char address[120];
	char *ptemp;
	char *host;
	int i;

	for (i = 0;i < mconn->num_rcpts;i++) {
		if (mconn->rcpt[i] != NULL) { free(mconn->rcpt[i]); mconn->rcpt[i] = NULL; }
	}
	memset(mconn->from, 0, sizeof(mconn->from));
	mconn->num_rcpts = 0;
	ptemp = line;
	while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
	snprintf(address, sizeof(address) - 1, "%s", ptemp);
	if ((ptemp = strchr(address, '>')) != NULL) *ptemp = '\0';
	if ((ptemp = strchr(address, '<')) != NULL) {
		ptemp++;
	}
	else {
		ptemp = address;
	}
	snprintf(mconn->from, sizeof(mconn->from) - 1, "%s", ptemp);
	if ((host = strchr(address, '@')) != NULL) {
		*host++ = '\0';
	}
	else {
		host = "localhost";
	}
	if (strlen(mconn->from)) {
		char line[256];

		snprintf(line, sizeof(line) - 1, "250 Sender '%s' OK", mconn->from);
		smtp_send(conn, line);
	}
	else {
		mconn->sender_is_blank = 1;
		smtp_send(conn, "250 Sender 'NULL' OK");
	}
	return;
}

static void smtp_rcpt(CONN *conn, MAILCONN *mconn, char *line)
{
	char address[120];
	char *ptemp1;
	char *ptemp2;
	int domainid;

	if ((strlen(mconn->from) == 0) && (mconn->sender_is_blank == 0)) {
		smtp_send(conn, "503 No sender");
		return;
	}
	ptemp1 = line;
	while ((*ptemp1 == ' ') || (*ptemp1 == '\t')) ptemp1++;
	snprintf(address, sizeof(address) - 1, "%s", ptemp1);
	if ((ptemp1 = strchr(address, '>')) != NULL) *ptemp1 = '\0';
	if ((ptemp1 = strchr(address, '<')) != NULL) {
		ptemp1++;
	}
	else {
		ptemp1 = address;
	}
	ptemp2 = strchr(ptemp1, '@');
	if (ptemp2 == NULL) {
		log_access(proc->N, MODSHORTNAME, "%s:%d -- recipient invalid from:'%s', to:'%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->from, ptemp1);
		smtp_send(conn, "553 Recipient Invalid");
		return;
	}
	ptemp2++;
	domainid = domain_getid(ptemp2);
	if (domainid < 1) {
		/* rcpt domain is not local.  can sender relay? */
		//if (!mconn->sender_can_relay == 0) {
		if (mconn->sender_can_relay != 1) {
			log_access(proc->N, MODSHORTNAME, "%s:%d -- relaying denied from:'%s', to:'%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn->from, ptemp1);
			log_access(proc->N, MODSHORTNAME, "%s:%d -- sender is not local, and neither is the recipient domain '%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, ptemp2);
			smtp_send(conn, "553 Relaying denied");
			return;
		}
	}
	mconn->rcpt[mconn->num_rcpts] = calloc(128, sizeof(char));
	snprintf(mconn->rcpt[mconn->num_rcpts], 127, "%s", ptemp1);
	mconn->num_rcpts++;
	{
		char line[256];

		snprintf(line, sizeof(line) - 1, "250 Recipient '%s' OK", mconn->rcpt[mconn->num_rcpts - 1]);
		smtp_send(conn, line);
	}
	return;
}

static void smtp_rset(CONN *conn, MAILCONN *mconn, short quiet)
{
	int i;

	for (i = 0;i < mconn->num_rcpts;i++) {
		if (mconn->rcpt[i] != NULL) { free(mconn->rcpt[i]); mconn->rcpt[i] = NULL; }
	}
	memset(mconn->from, 0, sizeof(mconn->from));
	mconn->sender_can_relay = allow_relay(conn);
	mconn->sender_is_blank = 0;
	mconn->num_rcpts = 0;
	if (!quiet) smtp_send(conn, "250 Reset OK");
	return;
}

static void smtp_noop(CONN *conn)
{
	smtp_send(conn, "220 OK");
	return;
}

static void smtp_turn(CONN *conn)
{
	smtp_send(conn, "502 No");
	return;
}

void smtp_dorequest(CONN *conn)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
	obj_t *hostnameobj;
	MAILCONN mconn;
	char line[256];
	char *ptemp;
	int i;

	log_access(proc->N, MODSHORTNAME, "%s:%d -- NEW CONNECTION", conn->dat->RemoteAddr, conn->dat->RemotePort);
	if (smtp_nsp_init(conn) < 0) {
		conn->N = nsp_endstate(conn->N);
		return;
	}
	memset((char *)&mconn, 0, sizeof(mconn));
	if (smtp_nsp_prefilter(conn)) goto cleanup;
	mconn.sender_can_relay = allow_relay(conn);
	if (mconn.sender_can_relay == -1) {
		//log_access(proc->N, MODSHORTNAME, "%s:%d -- This IP is banned", conn->dat->RemoteAddr, conn->dat->RemotePort);
		snprintf(line, sizeof(line) - 1, "550 5.3.2 Service currently unavailable; client %s temporarily blocked", conn->dat->RemoteAddr);
		smtp_send(conn, line);
		goto cleanup;
	}
	hostnameobj = nsp_getobj(proc->N, nsp_getobj(proc->N, nsp_getobj(proc->N, confobj, "modules"), "smtpd"), "host_name");
	if (nsp_isnull(hostnameobj)) hostnameobj = nsp_getobj(proc->N, confobj, "host_name");
	{
		snprintf(line, sizeof(line) - 1, "220 %s - %s SMTPd", nsp_tostr(proc->N, hostnameobj), SERVER_NAME);
		smtp_send(conn, line);
	}
	mconn.rcptalloc = 50;
	if ((mconn.rcpt = (char **)calloc(mconn.rcptalloc, sizeof(char *))) == NULL) return;
	do {
		memset(line, 0, sizeof(line));
		if (tcp_fgets(line, sizeof(line) - 1, &conn->socket) < 0) goto cleanup;
		striprn(line);
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", conn->dat->RemoteAddr, conn->dat->RemotePort, line);
		if (strcasecmp(line, "quit") == 0) {
			smtp_send(conn, "221 Goodbye");
			goto cleanup;
		}
		else if ((strncasecmp(line, "helo", 4) == 0) || (strncasecmp(line, "ehlo", 4) == 0)) {
			ptemp = line + 4;
			while ((*ptemp == ' ') || (*ptemp == '\t')) ptemp++;
			snprintf(mconn.helo, sizeof(mconn.helo) - 1, "%s", ptemp);
			{
				snprintf(line, sizeof(line) - 1, "250-%s Hello, %s [%s]", nsp_tostr(proc->N, hostnameobj), mconn.helo, conn->dat->RemoteAddr);
				smtp_send(conn, line);
			}
			if (proc->ssl_is_loaded && conn->socket.using_tls == 0) {
				smtp_send(conn, "250 STARTTLS");
			}
			else {
				smtp_send(conn, "250 AUTH PLAIN");
			}
			//log_access(proc->N, MODSHORTNAME, "%s:%d - HELO '%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, mconn.helo);
			if (strlen(mconn.helo)) {
				conn->state = 1;
				continue;
			}
		}
		else if (strncasecmp(line, "starttls", 8) == 0 && proc->ssl_is_loaded && conn->socket.using_tls == 0) {
			smtp_send(conn, "220 Ready to start TLS");
			smtp_rset(conn, &mconn, 1);
			conn->state = 0;
			ssl_accept(&conn->socket);
		}
		else if (conn->state == 1) {
			if (strncasecmp(line, "auth", 4) == 0) {
				smtp_send(conn, "502 Unknown Command");
			}
			else if (strncasecmp(line, "mail from:", 10) == 0) {
				smtp_from(conn, &mconn, line + 10);
			}
			else if (strncasecmp(line, "rcpt to:", 8) == 0) {
				smtp_rcpt(conn, &mconn, line + 8);
			}
			else if (strcasecmp(line, "data") == 0) {
				smtp_data(conn, &mconn);
			}
			else if (strcasecmp(line, "rset") == 0) {
				smtp_rset(conn, &mconn, 0);
			}
			else if (strcasecmp(line, "turn") == 0) {
				smtp_turn(conn);
			}
			else if (strcasecmp(line, "noop") == 0) {
				smtp_noop(conn);
			}
			else {
				log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "%s:%d UNKNOWN COMMAND: '%s'", conn->dat->RemoteAddr, conn->dat->RemotePort, line);
				smtp_send(conn, "502 Unknown Command");
			}
		}
		else {
			smtp_send(conn, "500 Some people still say hello");
		}
	} while (1);
cleanup:
	for (i = 0;i < mconn.num_rcpts;i++) {
		if (mconn.rcpt[i] != NULL) {
			free(mconn.rcpt[i]);
		}
	}
	free(mconn.rcpt);
	conn->N = nsp_endstate(conn->N);
	log_access(proc->N, MODSHORTNAME, "%s:%d -- END CONNECTION", conn->dat->RemoteAddr, conn->dat->RemotePort);
	return;
}
