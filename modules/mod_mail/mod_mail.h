/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#include <share.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#define closesocket close
#endif

typedef struct {
	char From[128];
	char ReplyTo[128];
	char To[2048];
	char CC[2048];
	char BCC[2048];
	char Subject[128];
	char Date[128];
	char contenttype[128];
	char boundary[128];
	char encoding[128];
	char status;
} wmheader;

/* mod_mail_addr.c */
void wmaddr_list(CONN *sid);
/* mod_mail_codec.c */
char *DecodeRFC2047(CONN *sid, char *src);
int  DecodeHTML(CONN *sid, short int reply, char *src, char *ctype, short int crlf);
int  DecodeQP(CONN *sid, short int reply, char *src, char *ctype);
int  DecodeText(CONN *sid, short int reply, char *src);
int  EncodeBase64(CONN *sid, char *src, int srclen);
int  EncodeBase64file(FILE *fp, char *src, int srclen);
int  DecodeBase64(CONN *sid, char *src, char *ctype);
char *EncodeBase64string(CONN *sid, char *src);
char *DecodeBase64string(CONN *sid, char *src);
/* mod_mail_html.c */
void htselect_mailjump(CONN *sid, int selected);
void htselect_mailmbox(CONN *sid, char *selected);
/* mod_mail_main.c */
void wmloginform(CONN *sid);
int  webmailheader(CONN *sid, wmheader *header);
/* mod_mail_mime.c */
int webmailheader(CONN *sid, wmheader *header);
void webmailfiledl(CONN *sid);
char *webmailfileul(CONN *sid, char *xfilename, char *xfilesize);
int webmailmime(CONN *sid, FILE **fp, char *contenttype, char *encoding, char *boundary, short int nummessage, short int reply, short int depth);
/* mod_mail_server.c */
int  wmprints(CONN *sid, const char *format, ...);
int  wmfgets(CONN *sid, char *buffer, int max, int fd);
int  wmffgets(CONN *sid, char *buffer, int max, FILE **fp);
void wmclose(CONN *sid);
int  wmserver_smtpconnect(CONN *sid);
int  wmserver_smtpauth(CONN *sid);
void wmserver_smtpdisconnect(CONN *sid);
int  wmserver_connect(CONN *sid, int verbose);
void wmserver_disconnect(CONN *sid);
int  wmserver_count(CONN *sid);
int  wmserver_msgdele(CONN *sid, int message);
int  wmserver_msghead(CONN *sid, int message);
int  wmserver_msgretr(CONN *sid, int message);
int  wmserver_msgsize(CONN *sid, int message);
int  wmserver_uidl(CONN *sid, int message, char *uidl);
int  is_msg_end(CONN *sid, char *buffer);
int  wmserver_msgsync(CONN *sid, int remoteid, int localid, int verbose);
int  wmserver_mlistsync(CONN *sid, char ***uidl_list);
void wmserver_purge(CONN *sid, int remove);
int  wmserver_send(CONN *sid, int mailid, int verbose);
int  wmsync(CONN *sid, int verbose);
