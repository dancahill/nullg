/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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
void wmaddr_list(CONNECTION *sid);
/* mod_mail_codec.c */
char *DecodeRFC2047(CONNECTION *sid, char *src);
int  DecodeHTML(CONNECTION *sid, short int reply, char *src, char *ctype, short int crlf);
int  DecodeQP(CONNECTION *sid, short int reply, char *src, char *ctype);
int  DecodeText(CONNECTION *sid, short int reply, char *src);
int  EncodeBase64(CONNECTION *sid, char *src, int srclen);
int  EncodeBase64file(FILE *fp, char *src, int srclen);
int  DecodeBase64(CONNECTION *sid, char *src, char *ctype);
char *EncodeBase64string(CONNECTION *sid, char *src);
char *DecodeBase64string(CONNECTION *sid, char *src);
/* mod_mail_html.c */
void htselect_mailjump(CONNECTION *sid, int selected);
void htselect_mailmbox(CONNECTION *sid, char *selected);
/* mod_mail_main.c */
void wmloginform(CONNECTION *sid);
int  webmailheader(CONNECTION *sid, wmheader *header);
/* mod_mail_mime.c */
int webmailheader(CONNECTION *sid, wmheader *header);
void webmailfiledl(CONNECTION *sid);
char *webmailfileul(CONNECTION *sid, char *xfilename, char *xfilesize);
int webmailmime(CONNECTION *sid, FILE *fp, char *contenttype, char *encoding, char *boundary, short int nummessage, short int reply, short int depth);
/* mod_mail_server.c */
int  wmprints(CONNECTION *sid, const char *format, ...);
int  wmfgets(CONNECTION *sid, char *buffer, int max, int fd);
int  wmffgets(CONNECTION *sid, char *buffer, int max, FILE **fp);
void wmclose(CONNECTION *sid);
int  wmserver_smtpconnect(CONNECTION *sid);
int  wmserver_smtpauth(CONNECTION *sid);
void wmserver_smtpdisconnect(CONNECTION *sid);
int  wmserver_connect(CONNECTION *sid, int verbose);
void wmserver_disconnect(CONNECTION *sid);
int  wmserver_count(CONNECTION *sid);
int  wmserver_msgdele(CONNECTION *sid, int message);
int  wmserver_msghead(CONNECTION *sid, int message);
int  wmserver_msgretr(CONNECTION *sid, int message);
int  wmserver_msgsize(CONNECTION *sid, int message);
int  wmserver_uidl(CONNECTION *sid, int message, char *uidl);
int  is_msg_end(CONNECTION *sid, char *buffer);
int  wmserver_msgsync(CONNECTION *sid, int remoteid, int localid, int verbose);
int  wmserver_mlistsync(CONNECTION *sid, char ***uidl_list);
void wmserver_purge(CONNECTION *sid, int remove);
int  wmserver_send(CONNECTION *sid, int mailid, int verbose);
int  wmsync(CONNECTION *sid, int verbose);
