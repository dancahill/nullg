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
#include "smtpq_main.h"

static int smtp_connect(TCP_SOCKET *smtp_sock, char *smtpserver)
{
	return tcp_connect(smtp_sock, smtpserver, 25, 0);
}

static int smtp_starttls(TCP_SOCKET *smtp_sock)
{
	return ssl_connect(smtp_sock);
}

static void smtp_disconnect(TCP_SOCKET *smtp_sock)
{
	char inbuffer[1024];

	tcp_fprintf(smtp_sock, "QUIT\r\n");
	log_access(proc->N, MODSHORTNAME, "%s:%d >> QUIT", smtp_sock->RemoteAddr, smtp_sock->RemotePort);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, smtp_sock) < 0) return;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock->RemoteAddr, smtp_sock->RemotePort, inbuffer);
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	tcp_close(smtp_sock, 1);
	return;
}

int smtp_client(FILE *fp, char *orig_msg, char *from, char *rcpt)
{
	obj_t *confobj = nsp_settable(proc->N, &proc->N->g, "CONFIG");
	char inbuffer[1024];
	char mxhost[128];
	char *ptemp1;
	char *ptemp2;
	TCP_SOCKET smtp_sock;
	int rc;
	short starttls = 0;

	/* if (strlen(from)==0) return -1; */
	if (strlen(rcpt) == 0) return -1;
	memset(inbuffer, 0, sizeof(inbuffer));
	memset((char *)&smtp_sock, 0, sizeof(smtp_sock));
	if (strlen(mod_config.smtp_relayhost) > 0) {
		if (smtp_connect(&smtp_sock, mod_config.smtp_relayhost) < 0) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "cannot connect to relay host '%s'", mod_config.smtp_relayhost);
			return -1;
		}
	}
	else {
		if ((ptemp1 = strchr(rcpt, '@')) == NULL) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "invalid recipient e-mail address '%s'", rcpt);
			return -1;
		}
		ptemp1++;
		if (*ptemp1 == '\0') {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "invalid recipient e-mail address '%s'", rcpt);
			return -1;
		}
		memset(mxhost, 0, sizeof(mxhost));
		ptemp2 = dns_getmxbyname(mxhost, sizeof(mxhost) - 1, ptemp1);
		if (ptemp2 == NULL) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "no MX entry found for '%s'", ptemp1);
			memset(mxhost, 0, sizeof(mxhost));
			snprintf(mxhost, sizeof(mxhost) - 1, "%s", ptemp1);
		}
		if ((rc = smtp_connect(&smtp_sock, mxhost)) < 0) {
			log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "cannot connect to host '%s'", mxhost);
			if (rc == -1) {
				snprintf(inbuffer, sizeof(inbuffer) - 1, "failed to resolve host '%s'", mxhost);
				bounce_send(from, rcpt, orig_msg, inbuffer);
				return -2;
			}
			return -1;
		}
	}
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
helo:
	tcp_fprintf(&smtp_sock, "EHLO %s\r\n", nsp_getstr(proc->N, confobj, "host_name"));
	log_access(proc->N, MODSHORTNAME, "%s:%d >> EHLO %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, nsp_getstr(proc->N, confobj, "host_name"));
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
		if (strstr(inbuffer, "STARTTLS") != NULL) {
			starttls = 1;
		}
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	// if EHLO fails, try HELO
	if (strncasecmp(inbuffer, "250", 3) != 0) {
		tcp_fprintf(&smtp_sock, "HELO %s\r\n", nsp_getstr(proc->N, confobj, "host_name"));
		log_access(proc->N, MODSHORTNAME, "%s:%d >> HELO %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, nsp_getstr(proc->N, confobj, "host_name"));
		do {
			memset(inbuffer, 0, sizeof(inbuffer));
			if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
			log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
		} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	}
	if (strncasecmp(inbuffer, "250", 3) != 0) goto quit;
	if (starttls && proc->ssl_is_loaded && !smtp_sock.using_tls) {
		//log_access(proc->N, MODSHORTNAME, "%s:%d -- STARTTLS is available!", smtp_sock.RemoteAddr, smtp_sock.RemotePort);
		tcp_fprintf(&smtp_sock, "STARTTLS\r\n");
		log_access(proc->N, MODSHORTNAME, "%s:%d >> STARTTLS", smtp_sock.RemoteAddr, smtp_sock.RemotePort);
		do {
			memset(inbuffer, 0, sizeof(inbuffer));
			if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
			log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
		} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
		if (strncasecmp(inbuffer, "220", 3) != 0) goto quit;
		rc = smtp_starttls(&smtp_sock);
		if (rc != 0) goto quit;
		goto helo;
	}
	tcp_fprintf(&smtp_sock, "MAIL FROM:<%s>\r\n", from);
	log_access(proc->N, MODSHORTNAME, "%s:%d >> MAIL FROM:<%s>", smtp_sock.RemoteAddr, smtp_sock.RemotePort, from);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	if (strncasecmp(inbuffer, "250", 3) != 0) goto quit;
	tcp_fprintf(&smtp_sock, "RCPT TO:<%s>\r\n", rcpt);
	log_access(proc->N, MODSHORTNAME, "%s:%d >> RCPT TO:<%s>", smtp_sock.RemoteAddr, smtp_sock.RemotePort, rcpt);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	if (strncasecmp(inbuffer, "250", 3) != 0) goto quit;
	tcp_fprintf(&smtp_sock, "DATA\r\n");
	log_access(proc->N, MODSHORTNAME, "%s:%d >> DATA", smtp_sock.RemoteAddr, smtp_sock.RemotePort);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	if (strncasecmp(inbuffer, "354", 3) != 0) goto quit;
	log_access(proc->N, MODSHORTNAME, "%s:%d >> [...]", smtp_sock.RemoteAddr, smtp_sock.RemotePort);
	while (fgets(inbuffer, sizeof(inbuffer) - 1, fp) != NULL) {
		striprn(inbuffer);
		tcp_fprintf(&smtp_sock, "%s\r\n", inbuffer);
	}
	tcp_fprintf(&smtp_sock, ".\r\n");
	log_access(proc->N, MODSHORTNAME, "%s:%d >> .", smtp_sock.RemoteAddr, smtp_sock.RemotePort);
	do {
		memset(inbuffer, 0, sizeof(inbuffer));
		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	if (strncasecmp(inbuffer, "250", 3) != 0) goto quit;
	log_access(proc->N, MODSHORTNAME, "mail sent from: '%s', to: '%s'", from, rcpt);
	smtp_disconnect(&smtp_sock);
	return 0;
quit:
	log_error(proc->N, MODSHORTNAME, __FILE__, __LINE__, 1, "remote server response: %s", inbuffer);
	smtp_disconnect(&smtp_sock);
	if (strncasecmp(inbuffer, "504", 3) != 0) {
		bounce_send(from, rcpt, orig_msg, inbuffer);
		/* unlink(filename); */
		return -2;
	}
	return -1;
}
