/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
#include "net.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#define snprintf _snprintf
#endif

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int _base64_encode_send(nsp_state *N, TCP_SOCKET *sock, obj_t *cobj)
{
#define __FN__ __FILE__ ":_base64_encode_send()"
	char dest[80];
	uchar a, b, c, d, *cp;
	int dst, i, enclen, remlen, linelen, maxline = 0;

	if (cobj->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj->val->size < 1) return 0;
	cp = (uchar *)cobj->val->d.str;
	dst = 0;
	linelen = 0;
	maxline = 76;
	enclen = cobj->val->size / 3;
	remlen = cobj->val->size - 3 * enclen;
	/* should actually be about 4/3 the size of cobj1->val->size */
	i = maxline ? (((cobj->val->size + 3) / 3 * 4) / maxline) : 5;
	i = i * 2 + 5;
	for (i = 0; i < enclen; i++) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		b |= (cp[1] >> 4);
		c = (cp[1] << 2) & 0x3c;
		c |= (cp[2] >> 6);
		d = cp[2] & 0x3f;
		cp += 3;
		dest[dst + 0] = b64chars[a];
		dest[dst + 1] = b64chars[b];
		dest[dst + 2] = b64chars[c];
		dest[dst + 3] = b64chars[d];
		dst += 4;
		linelen += 4;
		if ((maxline > 0) && (linelen >= maxline)) {
			dest[dst] = '\0';
			tcp_fprintf(N, sock, "%s\r\n", dest);
			dst = 0;
			linelen = 0;
		}
	}
	if (remlen == 1) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		dest[dst + 0] = b64chars[a];
		dest[dst + 1] = b64chars[b];
		dest[dst + 2] = '=';
		dest[dst + 3] = '=';
		dst += 4;
	} else if (remlen == 2) {
		a = (cp[0] >> 2);
		b = (cp[0] << 4) & 0x30;
		b |= (cp[1] >> 4);
		c = (cp[1] << 2) & 0x3c;
		dest[dst + 0] = b64chars[a];
		dest[dst + 1] = b64chars[b];
		dest[dst + 2] = b64chars[c];
		dest[dst + 3] = '=';
		dst += 4;
	}
	dest[dst] = '\0';
	tcp_fprintf(N, sock, "%s\r\n", dest);
	return 0;
#undef __FN__
}

static void smtp_lasterr(nsp_state *N, char *msg)
{
	striprn(msg);
	nsp_setstr(N, nsp_getobj(N, &N->context->l, "this"), "last_err", msg, -1);
	return;
}

NSP_CLASSMETHOD(libnsp_net_smtp_client_attach)
{
#define __FN__ __FILE__ ":libnsp_net_smtp_client_attach()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1"); /* attachment name */
	obj_t *cobj2 = nsp_getobj(N, &N->context->l, "2"); /* attachment */
	obj_t *tobj, *cobj;
	int size = 0;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if (cobj1->val->type != NT_STRING || cobj1->val->d.str == NULL) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING || cobj2->val->d.str == NULL) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	tobj = nsp_getobj(N, thisobj, "attachments");
	if (!nsp_istable(tobj)) {
		tobj = nsp_settable(N, thisobj, "attachments");
	}
	for (cobj = tobj->val->d.table.f; cobj; cobj = cobj->next) size++;
	cobj = nsp_settable(N, tobj, n_ntoa(N, N->numbuf, size, 10, 6));
	nsp_setstr(N, cobj, "name", cobj1->val->d.str, cobj1->val->size);
	nsp_setstr(N, cobj, "file", cobj2->val->d.str, cobj2->val->size);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_smtp_client_send)
{
#define __FN__ __FILE__ ":libnsp_net_smtp_client_send()"
	char tmpbuf[1024];
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	obj_t *cobj;
	obj_t *aobj;
	TCP_SOCKET sock;
	unsigned short use_tls = 0;
	unsigned short port;
	char *host;
	char *from;
	char *rcpt;
	char *subj;
	char *ctype;
	char *body;
	int blen = 0;
	int rc;
	int date = 0;
	struct timeval ttime;
	struct timezone tzone;
	char msgdate[100];
	short starttls = 0;

	if (!nsp_istable(thisobj)) n_error(N, NE_SYNTAX, __FN__, "expected a table for 'this'");
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "host")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for host");
	host = cobj->val->d.str;
	if (!nsp_isnum((cobj = nsp_getobj(N, thisobj, "port")))) n_error(N, NE_SYNTAX, __FN__, "expected a number for port");
	port = (unsigned short)cobj->val->d.num;
	if (nsp_isbool((cobj = nsp_getobj(N, thisobj, "use_tls")))) use_tls = cobj->val->d.num ? 1 : 0;
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "from")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for from");
	from = cobj->val->d.str;
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "rcpt")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for rcpt");
	rcpt = cobj->val->d.str;
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "subject")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for subject");
	subj = cobj->val->d.str;
	if (nsp_isnum((cobj = nsp_getobj(N, thisobj, "date")))) date = (int)cobj->val->d.num;
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "body")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for body");
	if (cobj->val->size < 1) n_error(N, NE_SYNTAX, __FN__, "body is empty");
	body = cobj->val->d.str;
	blen = cobj->val->size;
	if (!nsp_isstr((cobj = nsp_getobj(N, thisobj, "contenttype")))) n_error(N, NE_SYNTAX, __FN__, "expected a string for contenttype");
	ctype = cobj->val->d.str;
	aobj = nsp_getobj(N, thisobj, "attachments");

	nc_memset((char *)&sock, 0, sizeof(sock));
	if ((rc = tcp_connect(N, &sock, host, port, use_tls)) < 0) {
		nsp_setbool(N, &N->r, "", 0);
		smtp_lasterr(N, "tcp_connect error");
		return 0;
	}

	do {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	} while (rc > 0 && tmpbuf[3] != ' ' && tmpbuf[3] != '\0');
	if (nc_strncmp(tmpbuf, "220", 3) != 0) goto err;
#ifdef HAVE_TLS
helo:
#endif
	tcp_fprintf(N, &sock, "EHLO <%s>\r\n", sock.LocalAddr);
	do {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
		if (strstr(tmpbuf, "STARTTLS") != NULL) {
			starttls = 1;
		}
	} while (rc > 0 && tmpbuf[3] != ' ' && tmpbuf[3] != '\0');
	if (rc < 0 || nc_strncmp(tmpbuf, "250", 3) != 0) goto err;

	//// if EHLO fails, try HELO
	//if (strncasecmp(inbuffer, "250", 3) != 0) {
	//	tcp_fprintf(&smtp_sock, "HELO %s\r\n", nsp_getstr(proc->N, confobj, "host_name"));
	//	log_access(proc->N, MODSHORTNAME, "%s:%d >> HELO %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, nsp_getstr(proc->N, confobj, "host_name"));
	//	do {
	//		memset(inbuffer, 0, sizeof(inbuffer));
	//		if (tcp_fgets(inbuffer, sizeof(inbuffer) - 1, &smtp_sock) < 0) return -1;
	//		log_access(proc->N, MODSHORTNAME, "%s:%d << %s", smtp_sock.RemoteAddr, smtp_sock.RemotePort, inbuffer);
	//	} while ((inbuffer[3] != ' ') && (inbuffer[3] != '\0'));
	//}
	//if (strncasecmp(inbuffer, "250", 3) != 0) goto quit;

#ifdef HAVE_TLS
	if (starttls && !sock.use_tls) {
		tcp_fprintf(N, &sock, "STARTTLS\r\n");
		do {
			rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
		} while (rc > 0 && tmpbuf[3] != ' ' && tmpbuf[3] != '\0');
		if (nc_strncmp(tmpbuf, "220", 3) != 0) goto err;
		rc = _tls_connect(N, &sock);
		sock.use_tls = 1;
		if (rc != 0) goto err;
		goto helo;
	}
#endif

	tcp_fprintf(N, &sock, "MAIL FROM: <%s>\r\n", from);
	do {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	} while (rc > 0 && tmpbuf[3] != ' ' && tmpbuf[3] != '\0');
	if (rc < 0 || nc_strncmp(tmpbuf, "250", 3) != 0) goto err;

	tcp_fprintf(N, &sock, "RCPT TO: <%s>\r\n", rcpt);
	do {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	} while (rc > 0 && tmpbuf[3] != ' ' && tmpbuf[3] != '\0');
	if (rc < 0 || nc_strncmp(tmpbuf, "250", 3) != 0) goto err;

	tcp_fprintf(N, &sock, "DATA\r\n");
	do {
		rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	} while (rc > 0 && tmpbuf[3] != ' ' && tmpbuf[3] != '\0');
	if (rc < 0 || nc_strncmp(tmpbuf, "354", 3) != 0) goto err;
	tcp_fprintf(N, &sock, "From: %s\r\n", from);
	tcp_fprintf(N, &sock, "To: %s\r\n", rcpt);

	nc_gettimeofday(&ttime, &tzone);
	if (date) ttime.tv_sec = date;
/*
	strftime(msgdate, sizeof(msgdate), "%a, %d %b %Y %H:%M:%S", localtime((time_t *)&ttime.tv_sec));
	snprintf(msgdate+strlen(msgdate), sizeof(msgdate)-strlen(msgdate), " %+.4d", -tzone.tz_minuteswest/60*100);
*/
	strftime(msgdate, sizeof(msgdate), "%a, %d %b %Y %H:%M:%S +0000", gmtime((time_t *)&ttime.tv_sec));

	tcp_fprintf(N, &sock, "Date: %s\r\n", msgdate);

	tcp_fprintf(N, &sock, "Subject: %s\r\n", subj);
	if (nsp_istable(aobj)) {
		char *boundary = "LAME_MIME_BOUNDARY";

		tcp_fprintf(N, &sock, "Content-Type: multipart/mixed; boundary=\"%s\"\r\n", boundary);
		tcp_fprintf(N, &sock, "Mime-Version: 1.0\r\n");
		tcp_fprintf(N, &sock, "\r\n");
		tcp_fprintf(N, &sock, "This is MIME data.\r\n\r\n");
		tcp_fprintf(N, &sock, "--%s\r\n", boundary);
		tcp_fprintf(N, &sock, "Content-Type: %s; charset=utf-8\r\n", ctype);
		tcp_fprintf(N, &sock, "Content-Transfer-Encoding: 7bit\r\n\r\n");
		tcp_send(N, &sock, body, blen, 0);
		for (cobj = aobj->val->d.table.f; cobj; cobj = cobj->next) {
			tcp_fprintf(N, &sock, "\r\n--%s\r\n", boundary);
			tcp_fprintf(N, &sock, "Content-Type: application/octet-stream; name=\"%s\"\r\n", nsp_getstr(N, cobj, "name"));
			tcp_fprintf(N, &sock, "Content-Transfer-Encoding: base64\r\n");
			tcp_fprintf(N, &sock, "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", nsp_getstr(N, cobj, "name"));
			_base64_encode_send(N, &sock, nsp_getobj(N, cobj, "file"));
		}
		tcp_fprintf(N, &sock, "\r\n--%s--\r\n", boundary);
		tcp_fprintf(N, &sock, "\r\n");
	} else {
		tcp_fprintf(N, &sock, "Content-Type: %s\r\n", ctype);
		tcp_fprintf(N, &sock, "\r\n");
		tcp_send(N, &sock, body, (int)strlen(body), 0);
		tcp_fprintf(N, &sock, "\r\n");
	}
	tcp_fprintf(N, &sock, ".\r\n");
	rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	if (nc_strncmp(tmpbuf, "250", 3) != 0) goto err;

	nsp_setnum(N, &N->r, "", 1);
	N->r.val->type = NT_BOOLEAN;
	goto quit;
err:
	nsp_setbool(N, &N->r, "", 0);
	smtp_lasterr(N, tmpbuf);
quit:
	tcp_fprintf(N, &sock, "QUIT\r\n");
	rc = tcp_fgets(N, &sock, tmpbuf, sizeof(tmpbuf) - 1);
	striprn(tmpbuf);
	if (nc_strncmp(tmpbuf, "221", 3) != 0) {
		smtp_lasterr(N, tmpbuf);
	}
	tcp_close(N, &sock, 1);
	return 0;
#undef __FN__
}

NSP_CLASSMETHOD(libnsp_net_smtp_client_client)
{
#define __FN__ __FILE__ ":libnsp_net_smtp_client_client()"
	obj_t *thisobj = nsp_getobj(N, &N->context->l, "this");
	//obj_t *cobj;

	nsp_setstr(N, thisobj, "host", "localhost", 9);
	nsp_setnum(N, thisobj, "port", 25);
	nsp_setbool(N, thisobj, "use_tls", 0);
	nsp_setstr(N, thisobj, "from", "", 0);
	nsp_setstr(N, thisobj, "rcpt", "", 0);
	nsp_setnum(N, thisobj, "date", 0);
	nsp_setstr(N, thisobj, "subject", "(no subject)", 12);
	nsp_setstr(N, thisobj, "contenttype", "text/plain", 10);
	nsp_setstr(N, thisobj, "body", "", 0);
	//cobj = nsp_getobj(N, nsp_getobj(N, nsp_getobj(N, &N->g, "net"), "smtp"), "client");
	//if (nsp_istable(cobj)) nsp_zlink(N, &N->l, cobj);
	//else n_warn(N, __FN__, "net.smtp.client not found");
	return 0;
#undef __FN__
}
