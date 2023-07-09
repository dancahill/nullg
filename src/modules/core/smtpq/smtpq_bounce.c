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
#include "smtpq_main.h"

int bounce_send(char *from, char *rcpt, char *orig_msg, char *reason)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
	FILE *fp;
	FILE *fp2;
	struct stat sb;
	char inbuffer[1024];
	char tmpname1[256];
	char tmpname2[256];
	char tmpaddr[128];
	char *host;
	int br;
	int localdomainid;
	int userid;
	obj_t *qptr = NULL;
	int is_remote;
	struct timeval ttime;
	struct timezone tzone;
	char curdate[40];
	time_t t = time(NULL);

	if (strlen(from) == 0) return 0;
	snprintf(tmpaddr, sizeof(tmpaddr) - 1, "%s", from);
	if ((host = strchr(tmpaddr, '@')) != NULL) {
		*host++ = '\0';
	}
	else {
		host = "localhost";
	}
	is_remote = 0;
	localdomainid = domain_getid(host);
	if (localdomainid > 0) {
		userid = 0;
		if (sql_queryf(proc->N, &qptr, "SELECT userid FROM gw_users WHERE username = '%s' AND domainid = %d", tmpaddr, localdomainid) < 0) return -1;
		if (sql_numtuples(proc->N, &qptr) == 1) userid = atoi(sql_getvalue(proc->N, &qptr, 0, 0));
		sql_freeresult(proc->N, &qptr);
		if (userid < 1) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "no such mailbox: '%s', sent from MAILER-DAEMON", from);
			return -1;
		}
		memset(tmpname1, 0, sizeof(tmpname1));
		memset(tmpname2, 0, sizeof(tmpname2));
		snprintf(tmpname1, sizeof(tmpname1) - 1, "%s/domains/%04d/mailspool/%s", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), localdomainid, tmpaddr);
		if (stat(tmpname1, &sb) != 0) {
#ifdef WIN32
			if (mkdir(tmpname1) != 0) {
#else
			//			if (mkdir(tmpname1, ~config->umask&0777)!=0) {
			if (mkdir(tmpname1, ~0644 & 0777) != 0) {
#endif
				log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "Error creating directory '%s'", tmpname1);
				return -1;
			}
			}
	retry1:
		gettimeofday(&ttime, &tzone);
		memset(tmpname1, 0, sizeof(tmpname1));
		snprintf(tmpname1, sizeof(tmpname1) - 1, "%s/domains/%04d/mailspool/%s/%d%03d.msg", nsp_getstr(proc->N, nsp_settable(proc->N, confobj, "paths"), "var"), localdomainid, tmpaddr, (int)ttime.tv_sec, (int)(ttime.tv_usec / 1000));
		fixslashes(tmpname1);
		if (stat(tmpname1, &sb) == 0) goto retry1;
		log_access(proc->N, "smtpd", "local delivery from: MAILER-DAEMON, to: '%s'", from);
		}
	else {
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
		log_access(proc->N, "smtpd", "remote delivery from: MAILER-DAEMON, to: '%s'", from);
	}
	if ((fp = fopen(tmpname1, "wb")) != NULL) {
		strftime(curdate, sizeof(curdate), "%a, %d %b %Y %H:%M:%S", gmtime(&t));
		fprintf(fp, "Date: %s\r\n", curdate);
		fprintf(fp, "From: MAILER-DAEMON <mailer-daemon@%s>\r\n", nsp_getstr(proc->N, confobj, "host_name"));
		memset(curdate, 0, sizeof(curdate));
		fprintf(fp, "Subject: failure notice\r\n");
		fprintf(fp, "To: <%s>\r\n", from);
		fprintf(fp, "\r\n");
		fprintf(fp, "Your e-mail was undeliverable.  Sorry.\r\n");
		fprintf(fp, "\r\n");
		fprintf(fp, "Original recipient: <%s>\r\n", rcpt);
		fprintf(fp, "[%s]\r\n", reason);
		if (orig_msg != NULL) {
			if ((fp2 = fopen(orig_msg, "rb")) != NULL) {
				fprintf(fp, "\r\n--- Below this line is a copy of the original message.\r\n\r\n");
				br = 0;
				while (fgets(inbuffer, sizeof(inbuffer) - 1, fp2) != NULL) {
					br += strlen(inbuffer);
					if (br > 32768) break;
					fprintf(fp, "%s", inbuffer);
				}
				fclose(fp2);
			}
		}
		fclose(fp);
		if (is_remote) {
			fp = fopen(tmpname2, "wb");
			if (fp == NULL) {
				unlink(tmpname1);
				return -1;
			}
			fprintf(fp, "MAIL:\r\n");
			fprintf(fp, "RCPT: %s\r\n", from);
			memset(curdate, 0, sizeof(curdate));
			time_unix2sql(curdate, sizeof(curdate) - 1, t);
			fprintf(fp, "DATE: %s\r\n", curdate);
			fclose(fp);
		}
	}
	else {
		log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 0, "ERROR: Cannot write to file [%s].", tmpname1);
		return -1;
	}
	return 0;
	}
