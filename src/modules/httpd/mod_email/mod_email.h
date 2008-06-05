/*
    NullLogic GroupServer - Copyright (C) 2000-2008 Dan Cahill

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
#include "nullgs/httpd_mod.h"
#ifdef WIN32
#include <share.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/file.h>
#endif

/* mod_email_accounts.c */
void wmaccount_edit(CONN *sid);
void wmaccount_list(CONN *sid);
void wmaccount_save(CONN *sid);
/* mod_email_addr.c */
void wmaddr_list(CONN *sid);
/* mod_email_codec.c */
char *DecodeRFC2047(CONN *sid, char *src);
int   DecodeHTML(CONN *sid, char *dest, unsigned int szdest, char *src, short int reply);
int   DecodeQP(char *dest, unsigned int szdest, char *src);
int   DecodeText(char *dest, unsigned int szdest, char *src);
int   DecodeBase64file(CONN *sid, char *src);
char *DecodeBase64string(CONN *sid, char *src);
int   EncodeBase64(CONN *sid, char *src, int srclen);
int   EncodeBase64file(FILE *fp, char *src, int srclen);
char *EncodeBase64string(CONN *sid, char *src);
/* mod_email_conf.c */
int conf_read(void);
/* mod_email_db.c */
int dbread_mailaccount(CONN *sid, short int perm, int index, obj_t **qobj);
int dbwrite_mailaccount(CONN *sid, int index, obj_t **qobj);
int dbread_mailcurrent(CONN *sid, int mailcurrent);
int dbread_mailheader(CONN *sid, short int perm, int index, obj_t **qobj);
int dbwrite_mailheader(CONN *sid, int index, obj_t **qobj);
/* mod_email_files.c */
void webmailsave(CONN *sid);
void webmailmove(CONN *sid);
void webmailpurge(CONN *sid);
/* mod_email_filters.c */
int wmfilter_apply(CONN *sid, obj_t **hobj, int accountid, int messageid);
void wmfilter_edit(CONN *sid);
void wmfilter_list(CONN *sid, int accountid);
void wmfilter_save(CONN *sid);
/* mod_email_folders.c */
void wmfolder_list(CONN *sid, int accountid);
void wmfolder_edit(CONN *sid);
void wmfolder_save(CONN *sid);
/* mod_email_html.c */
void htselect_mailaccount(CONN *sid, int selected);
void htselect_mailfolder(CONN *sid, int selected, short int allow_zero, short int show_root);
void htselect_mailfolderjump(CONN *sid, int selected);
/* void htselect_mailjump(CONN *sid, int selected); */
void htselect_mailjump(CONN *sid, int accountid, int folderid);
/* mod_email_lang.c */
int lang_read();
/* mod_email_main.c */
void wmloginform(CONN *sid);
/* mod_email_mime.c */
int   webmailheader(CONN *sid, obj_t **hobj, FILE **fp);
void  webmailfiledl(CONN *sid);
char *webmailfileul(CONN *sid, char *xfilename, char *xfilesize);
int   webmailmime(CONN *sid, FILE **fp, char *contenttype, char *encoding, char *boundary, int nummessage, short int reply, short int depth);
/* mod_email_search.c */
char *wmsearch_makestring(CONN *sid);
int   wmsearch_doquery(CONN *sid, obj_t **qobj, const char *order_by, int accountid, int folderid);
//int   wmsearch_doquery(CONN *sid, obj_t **qobj, const char *order_by, int folderid);
void  wmsearch_form(CONN *sid);
/* mod_email_folders.c */
int  wmfolder_makedefaults(CONN *sid, int accountid);
int  wmfolder_msgmove(CONN *sid, int accountid, int messageid, int srcfolderid, int dstfolderid);
int  wmfolder_testcreate(CONN *sid, int accountid, int folderid);
/* mod_email_server.c */
/*
int  wmprints(CONN *sid, const char *format, ...);
int  wmfgets(CONN *sid, char *buffer, int max, TCP_SOCKET *sock);
int  wmffgets(char *buffer, int max, FILE **fp);
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
void wmserver_purge(CONN *sid);
int  wmserver_send(CONN *sid, int mailid, int verbose);
int  wmsync(CONN *sid, int verbose);
*/
int  wmprints(CONN *sid, const char *format, ...);
int  wmfgets(CONN *sid, char *buffer, int max, TCP_SOCKET *sock);
int  wmffgets(char *buffer, int max, FILE **fp);
void wmserver_purge(CONN *sid);
int  wmsync(CONN *sid, int verbose);

typedef struct {
	char filter_program[256];
	int sock_maxidle;
} MOD_CONFIG;

#ifdef SRVMOD_MAIN
	MOD_CONFIG mod_config;
#else
	extern MOD_CONFIG mod_config;
#endif
