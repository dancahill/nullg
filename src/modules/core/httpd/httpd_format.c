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
#include "httpd_main.h"

char *getbuffer(CONN *conn)
{
	conn->dat->lastbuf++;
	if (conn->dat->lastbuf > 3) conn->dat->lastbuf = 0;
	memset(conn->dat->smallbuf[conn->dat->lastbuf], 0, sizeof(conn->dat->smallbuf[conn->dat->lastbuf]));
	return conn->dat->smallbuf[conn->dat->lastbuf];
}

int hex2int(char *pChars)
{
	int Hi;
	int Lo;
	int Result;

	Hi = pChars[0];
	if ('0' <= Hi&&Hi <= '9') {
		Hi -= '0';
	}
	else if ('a' <= Hi&&Hi <= 'f') {
		Hi -= ('a' - 10);
	}
	else if ('A' <= Hi&&Hi <= 'F') {
		Hi -= ('A' - 10);
	}
	Lo = pChars[1];
	if ('0' <= Lo&&Lo <= '9') {
		Lo -= '0';
	}
	else if ('a' <= Lo&&Lo <= 'f') {
		Lo -= ('a' - 10);
	}
	else if ('A' <= Lo&&Lo <= 'F') {
		Lo -= ('A' - 10);
	}
	Result = Lo + (16 * Hi);
	return (Result);
}

void swapchar(char *string, char oldchar, char newchar)
{
	while (*string) {
		if (*string == oldchar) *string = newchar;
		string++;
	}
}

void decodeurl(char *src)
{
	char *dest = src;

	while (*src) {
		if (*src == '+') {
			*dest++ = ' ';
			src++;
		}
		else if (*src == '%') {
			src++;
			if (isxdigit(src[0]) && isxdigit(src[1])) {
				*dest++ = (char)hex2int(src);
				src += 2;
			}
		}
		else {
			*dest++ = *src++;
		}
	}
	*dest = '\0';
	return;
}

char *encodeurl(CONN *conn, char *src)
{
	char *buffer = getbuffer(conn);
	char *dest = buffer;
	/* char *unsafe="`'!@#$%^&*(){}<>~|\\\";? ,/"; */
	//char *unsafe="`'!@#$^&*(){}<>~|\\\";?,";
	char *hex = "0123456789ABCDEF";
	unsigned int dstlen = 0;

	while (*src) {
		if (dstlen + 1 > sizeof(conn->dat->smallbuf[0])) break;
		if (isalnum(*src) || *src == '~' || *src == '-' || *src == '.' || *src == '_') {
			*dest++ = *src++;
			dstlen++;
			continue;
		}
		if (dstlen + 4 > sizeof(conn->dat->smallbuf[0])) break;
		/*		if ((*src>32)&&(*src<128)&&(!strchr(unsafe, *src))) { */
		//		if ((*src>32)&&(!strchr(unsafe, *src))) {
		//			*dest++=*src++;
		//			dstlen++;
		//		} else {
		*dest++ = '%';
		*dest++ = hex[(short int)*src / 16];
		*dest++ = hex[(short int)*src++ & 15];
		dstlen += 3;
		//		}
	}
	*dest = '\0';
	return buffer;
}

/* HTML CODE CONVERSION TABLE */
typedef struct {
	char symbol;
	char *code;
} _htmltags;
static _htmltags htmltags[] = {
	{  '"', "&quot;" },
	{  '&', "&amp;" },
	{  '<', "&lt;" },
	{  '>', "&gt;" }
};

char *str2html(CONN *conn, char *instring)
{
	char *buffer = getbuffer(conn);
	char *inptr = instring;
	char *outptr = buffer;
	short int destleft = sizeof(conn->dat->smallbuf[0]) - 1;
	short int i;
	short int match;
	short int str_len;

	while ((*inptr) && (destleft > 0)) {
		match = 0;
		for (i = 0;i < 4;i++) {
			if (*inptr == htmltags[i].symbol) {
				match = 1;
				str_len = (short int)strlen(htmltags[i].code);
				if (destleft < str_len) goto done;
				strcpy(outptr, htmltags[i].code);
				destleft -= str_len;
				outptr += str_len;
				inptr++;
				break;
			}
		}
		if (!match) {
			if (destleft < 1) goto done;
			*outptr++ = *inptr++;
			destleft--;
		}
	}
done:
	*outptr = '\0';
	buffer[sizeof(conn->dat->smallbuf[0]) - 1] = '\0';
	return buffer;
}

int printhex(CONN *conn, const char *format, ...)
{
	char *buffer = getbuffer(conn);
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(conn->dat->smallbuf[0]) - 1, format, ap);
	va_end(ap);
	buffer[sizeof(conn->dat->smallbuf[0]) - 1] = '\0';
	return prints(conn, "%s", encodeurl(conn, buffer));
}

int printht(CONN *conn, const char *format, ...)
{
	char *buffer = getbuffer(conn);
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(conn->dat->smallbuf[0]) - 1, format, ap);
	va_end(ap);
	buffer[sizeof(conn->dat->smallbuf[0]) - 1] = '\0';
	return prints(conn, "%s", str2html(conn, buffer));
}

void printline(CONN *conn, short int reply, char *msgtext)
{
	char *pmsgbody;
	char *ptemp;
	char line[120];
	unsigned int width = 78;

	pmsgbody = msgtext;
	if (reply) width = 76;
	while (strlen(pmsgbody) > width) {
		memset(line, 0, sizeof(line));
		snprintf(line, width + 1, "%s", pmsgbody);
		line[width] = '\0';
		if (reply) prints(conn, "> ");
		if (strchr(line, '\r') || strchr(line, '\n')) {
			if ((ptemp = strchr(line, '\r')) != NULL) *ptemp = '\0';
			if ((ptemp = strchr(line, '\n')) != NULL) *ptemp = '\0';
			printht(conn, "%s\r\n", line);
			pmsgbody += strlen(line);
			if (*pmsgbody == '\r') pmsgbody++;
			if (*pmsgbody == '\n') pmsgbody++;
		}
		else if ((ptemp = strrchr(line, ' ')) != NULL) {
			*ptemp = '\0';
			printht(conn, "%s\r\n", line);
			pmsgbody += strlen(line) + 1;
		}
		else {
			printht(conn, "%s", line);
			pmsgbody += strlen(line);
		}
	}
	if (reply) prints(conn, "> ");
	printht(conn, "%s\r\n", pmsgbody);
}

void printline2(CONN *conn, int dowrap, char *msgtext)
{
	char buffer[1024];
	char lastchar = '\0';
	char *ptemp1;
	char *ptemp2;
	int destleft;

	ptemp1 = msgtext;
	while (strlen(ptemp1) > 0) {
		memset(buffer, 0, sizeof(buffer));
		destleft = sizeof(buffer) - 6;
		ptemp2 = buffer;
		while ((*ptemp1) && (destleft > 64)) {
			if (*ptemp1 == '\n') {
				if (dowrap) {
					strcat(ptemp2, "\n<BR />");
					ptemp2 += 7;
					destleft -= 7;
				}
				else {
					strcat(ptemp2, "\n");
					ptemp2 += 1;
					destleft -= 1;
				}
			}
			else if (*ptemp1 == '<') {
				strcat(ptemp2, "&lt;");
				ptemp2 += 4;
				destleft -= 4;
			}
			else if (*ptemp1 == '>') {
				strcat(ptemp2, "&gt;");
				ptemp2 += 4;
				destleft -= 4;
			}
			else if (*ptemp1 == '&') {
				strcat(ptemp2, "&amp;");
				ptemp2 += 5;
				destleft -= 5;
			}
			else if (*ptemp1 == '"') {
				strcat(ptemp2, "&quot;");
				ptemp2 += 6;
				destleft -= 6;
			}
			else if (*ptemp1 == ' ') {
				if (dowrap) {
					if (!isalnum(ptemp1[1]) || !isalnum(lastchar)) {
						strcat(ptemp2, "&nbsp;");
						ptemp2 += 6;
						destleft -= 6;
					}
					else {
						*ptemp2 = *ptemp1;
						ptemp2++;
						destleft--;
					}
				}
				else {
					strcat(ptemp2, " ");
					ptemp2 += 1;
					destleft -= 1;
				}
			}
			else {
				*ptemp2 = *ptemp1;
				ptemp2++;
				destleft--;
			}
			lastchar = *ptemp1;
			ptemp1++;
		}
		prints(conn, "%s", buffer);
	}
	return;
}

void htselect_timezone(CONN *conn, short int selected)
{
	obj_t *cobj, *tobj;
	int i = 0;//, n;

	if ((conn == NULL) || (conn->N == NULL)) return;
	tobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	cobj = nsp_getobj(conn->N, tobj, "TIMEZONES");
	if (cobj->val->type != NT_TABLE) {
		cobj = nsp_settable(conn->N, tobj, "TIMEZONES");
		cobj->val->attr |= NST_HIDDEN;
		htnsp_dotemplate(conn, "", "tzones.ns");
	}
	for (tobj = cobj->val->d.table.f;tobj;tobj = tobj->next) {
		if (tobj->val->type != NT_TABLE) continue;
		//n=(int)nsp_getnum(conn->N, tobj, "o");
		prints(conn, "<OPTION VALUE='%d'%s>%s\r\n", i, (i == selected) ? " SELECTED" : "", nsp_getstr(conn->N, tobj, "n"));
		i++;
	}
	return;
}

char *time_sql2text(CONN *conn, char *sqldate)
{
	char *buffer = getbuffer(conn);
	time_t unixdate = time_sql2unix(sqldate);

	strftime(buffer, 30, "%b %d, %Y %I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_sql2datetext(CONN *conn, char *sqldate)
{
	char *buffer = getbuffer(conn);
	time_t unixdate = time_sql2unix(sqldate);

	strftime(buffer, 30, "%b %d, %Y", gmtime(&unixdate));
	return buffer;
}

char *time_sql2timetext(CONN *conn, char *sqldate)
{
	char *buffer = getbuffer(conn);
	time_t unixdate = time_sql2unix(sqldate);

	strftime(buffer, 30, "%I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_sql2lotimetext(CONN *conn, char *sqldate)
{
	char *buffer = getbuffer(conn);
	char *ptemp = buffer;
	time_t unixdate = time_sql2unix(sqldate);

	strftime(buffer, 30, "%I:%M%p", gmtime(&unixdate));
	while (*ptemp) *ptemp++ = tolower(*ptemp);
	return buffer;
}

char *time_unix2sqldate(CONN *conn, time_t unixdate)
{
	char *buffer = getbuffer(conn);

	strftime(buffer, 30, "%Y-%m-%d", gmtime(&unixdate));
	return buffer;
}

char *time_unix2sqltime(CONN *conn, time_t unixdate)
{
	char *buffer = getbuffer(conn);

	strftime(buffer, 30, "%H:%M:%S", gmtime(&unixdate));
	return buffer;
}

char *time_unix2text(CONN *conn, time_t unixdate)
{
	char *buffer = getbuffer(conn);

	strftime(buffer, 50, "%b %d, %Y %I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_unix2datetext(CONN *conn, time_t unixdate)
{
	char *buffer = getbuffer(conn);

	strftime(buffer, 30, "%b %d, %Y", gmtime(&unixdate));
	return buffer;
}

char *time_unix2timetext(CONN *conn, time_t unixdate)
{
	char *buffer = getbuffer(conn);

	strftime(buffer, 30, "%I:%M %p", gmtime(&unixdate));
	return buffer;
}

char *time_unix2lotimetext(CONN *conn, time_t unixdate)
{
	char *buffer = getbuffer(conn);
	char *ptemp = buffer;

	strftime(buffer, 30, "%I:%M%p", gmtime(&unixdate));
	while (*ptemp) { *ptemp = tolower(*ptemp); ptemp++; }
	return buffer;
}

int time_tzoffset(CONN *conn, time_t unixdate)
{
	obj_t *cobj, *tobj;
	struct tm *today;
	time_t tzoffset;

	if ((conn == NULL) || (conn->N == NULL)) return 0;
	tobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	cobj = nsp_getobj(conn->N, tobj, "TIMEZONES");
	if (cobj->val->type != NT_TABLE) {
		cobj = nsp_settable(conn->N, tobj, "TIMEZONES");
		cobj->val->attr |= NST_HIDDEN;
		htnsp_dotemplate(conn, "", "tzones.ns");
	}
	tobj = nsp_getiobj(conn->N, cobj, conn->dat->timezone);
	if (tobj->val->type != NT_NUMBER) return 0;
	tzoffset = (int)nsp_getnum(conn->N, tobj, "o") * 60;
	today = localtime(&unixdate);
	if (((int)nsp_getnum(conn->N, tobj, "d")) && (today->tm_isdst)) {
		tzoffset += 3600;
	}
	return tzoffset;
}

int time_tzoffset2(CONN *conn, time_t unixdate, int userid)
{
	obj_t *cobj, *tobj;
	obj_t *qobj = NULL;
	int tz = -1;
	time_t tzoffset;
	struct tm *today;

	if (sql_queryf(proc->N, &qobj, "SELECT preftimezone FROM gw_users where userid = %d", userid) < 0) return 0;
	if (sql_numtuples(proc->N, &qobj) == 1) tz = atoi(sql_getvalue(proc->N, &qobj, 0, 0));
	sql_freeresult(proc->N, &qobj);
	tobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	cobj = nsp_getobj(conn->N, tobj, "TIMEZONES");
	if (cobj->val->type != NT_TABLE) {
		cobj = nsp_settable(conn->N, tobj, "TIMEZONES");
		cobj->val->attr |= NST_HIDDEN;
		htnsp_dotemplate(conn, "", "tzones.ns");
	}
	tobj = nsp_getiobj(conn->N, cobj, tz);
	if (tobj->val->type != NT_NUMBER) return 0;
	tzoffset = (int)nsp_getnum(conn->N, tobj, "o") * 60;
	today = localtime(&unixdate);
	if (((int)nsp_getnum(conn->N, tobj, "d")) && (today->tm_isdst)) {
		tzoffset += 3600;
	}
	return tzoffset;
}

int time_tzoffsetcon(CONN *conn, time_t unixdate, int contactid)
{
	obj_t *cobj, *tobj;
	obj_t *qobj = NULL;
	int tz = -1;
	time_t tzoffset;
	struct tm *today;

	if (sql_queryf(proc->N, &qobj, "SELECT timezone FROM gw_contacts where contactid = %d", contactid) < 0) return 0;
	if (sql_numtuples(proc->N, &qobj) == 1) tz = atoi(sql_getvalue(proc->N, &qobj, 0, 0));
	sql_freeresult(proc->N, &qobj);
	tobj = nsp_settable(conn->N, &conn->N->g, "_SERVER");
	cobj = nsp_getobj(conn->N, tobj, "TIMEZONES");
	if (cobj->val->type != NT_TABLE) {
		cobj = nsp_settable(conn->N, tobj, "TIMEZONES");
		cobj->val->attr |= NST_HIDDEN;
		htnsp_dotemplate(conn, "", "tzones.ns");
	}
	tobj = nsp_getiobj(conn->N, cobj, tz);
	if (tobj->val->type != NT_NUMBER) return 0;
	tzoffset = (int)nsp_getnum(conn->N, tobj, "o") * 60;
	today = localtime(&unixdate);
	if (((int)nsp_getnum(conn->N, tobj, "d")) && (today->tm_isdst)) {
		tzoffset += 3600;
	}
	return tzoffset;
}

time_t time_wmgetdate(char *src)
{
	char *months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
	int dim[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	time_t unixdate = -1;
	int year, month, day, hours, minutes, seconds;
	int i, tzoffset;
	char *p = src;

	while ((*p) && (!isdigit(*p))) p++;
	if (!isdigit(*p)) return 0;
	if (*p == '0') p++;
	day = atoi(p);
	while ((*p) && (!isalpha(*p))) p++;
	month = -1;
	for (i = 0;i < 12;i++) {
		if (strncasecmp(p, months[i], 3) == 0) month = i;
	}
	if (month == -1) return 0;
	while ((*p) && (!isdigit(*p))) p++;
	year = atoi(p);
	for (i = 1970;i < year;i++) {
		unixdate += 365;
		if ((i / 4.0f) == (int)(i / 4)) {
			if ((i / 400.0f) == (int)(i / 400)) {
				unixdate++;
			}
			else if ((i / 100.0f) != (int)(i / 100)) {
				unixdate++;
			}
		}
	}
	for (i = 0;i < month;i++) {
		unixdate += dim[i];
		if (i != 1) continue;
		if ((year / 4.0f) == (int)(year / 4)) {
			if ((year / 400.0f) == (int)(year / 400)) {
				unixdate++;
			}
			else if ((year / 100.0f) != (int)(year / 100)) {
				unixdate++;
			}
		}
	}
	unixdate += day;
	if (unixdate < 1) return 0;
	unixdate *= 86400;
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p))) p++;
	hours = atoi(p);
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p))) p++;
	minutes = atoi(p);
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p))) p++;
	seconds = atoi(p);
	while (isdigit(*p)) p++;
	while ((*p) && (!isdigit(*p)) && (*p != '-')) p++;
	tzoffset = atoi(p);
	unixdate += hours * 3600;
	unixdate += minutes * 60;
	unixdate += seconds;
	unixdate -= (int)(tzoffset / 100) * 3600;
	return unixdate;
}
