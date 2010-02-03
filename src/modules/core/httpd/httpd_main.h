/*
    NullLogic GroupServer - Copyright (C) 2000-2010 Dan Cahill

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
#include "nullsd/core_mod.h"
#include "nullsd/httpd_typedefs.h"

#define MODSHORTNAME "httpd"

extern HTTP_PROC htproc;

/* auth.c functions */
void auth_savesession(CONN *conn);
char *auth_setpass(CONN *conn, char *rpassword);
int auth_getcookie(CONN *conn);
int auth_setcookie(CONN *conn);
void auth_logout(CONN *conn);
int auth_priv(CONN *conn, char *service);
/* conf.c functions */
int conf_read(void);
/* format.c functions */
char *getbuffer(CONN *conn);
int hex2int(char *pChars);
void swapchar(char *string, char oldchar, char newchar);
void decodeurl(char *src);
char *encodeurl(CONN *conn, char *src);
char *str2html(CONN *conn, char *instring);
int printhex(CONN *conn, const char *format, ...);
int printht(CONN *conn, const char *format, ...);
void printline(CONN *conn, short int reply, char *msgtext);
void printline2(CONN *conn, int dowrap, char *msgtext);
void htselect_timezone(CONN *conn, short int selected);
char *time_sql2datetext(CONN *conn, char *sqldate);
char *time_sql2timetext(CONN *conn, char *sqldate);
char *time_unix2text(CONN *conn, time_t unixdate);
char *time_unix2datetext(CONN *conn, time_t unixdate);
char *time_unix2timetext(CONN *conn, time_t unixdate);
char *time_unix2lotimetext(CONN *conn, time_t unixdate);
char *time_unix2sqldate(CONN *conn, time_t unixdate);
char *time_unix2sqltime(CONN *conn, time_t unixdate);
int time_tzoffset(CONN *conn, time_t unixdate);
int time_tzoffset2(CONN *conn, time_t unixdate, int userid);
int time_tzoffsetcon(CONN *conn, time_t unixdate, int contactid);
time_t time_wmgetdate(char *src);
/* html.c functions */
void htpage_header(CONN *conn, char *title);
void htpage_footer(CONN *conn);
int  htpage_dirlist(CONN *conn);
void htpage_login(CONN *conn);
void htpage_logout(CONN *conn);
void htpage_frameset(CONN *conn);
void htpage_topmenu(CONN *conn, char *modname);
void htpage_motd(CONN *conn);
/*
void htselect_day(CONN *conn, char *selected);
void htselect_hour(CONN *conn, int selected);
void htselect_month(CONN *conn, char *selected);
void htselect_qhours(CONN *conn, int selected);
void htselect_qminutes(CONN *conn, int selected);
void htselect_minutes(CONN *conn, int selected);
void htselect_time(CONN *conn, time_t unixtime);
void htselect_year(CONN *conn, int startyear, char *selected);
void htselect_contact(CONN *conn, int selected);
void htselect_domain(CONN *conn, int selected);
void htselect_eventstatus(CONN *conn, int selected);
void htselect_group(CONN *conn, int perm, int selected, int domainid);
void htselect_number(CONN *conn, int selected, int start, int end, int increment);
void htselect_priority(CONN *conn, int selected);
void htselect_reminder(CONN *conn, int selected);
void htselect_user(CONN *conn, int selected, int domainid);
void htselect_zone(CONN *conn, int selected, int domainid);
char *htview_callaction(CONN *conn, int selected);
char *htview_contact(CONN *conn, int selected);
char *htview_domain(CONN *conn, int selected);
char *htview_eventclosingstatus(CONN *conn, int selected);
char *htview_eventstatus(int selected);
char *htview_eventtype(CONN *conn, int selected);
char *htview_holiday(char *date);
char *htview_reminder(CONN *conn, int selected);
char *htview_user(CONN *conn, int selected);
*/
void htscript_showpage(CONN *conn, short int pages);
/* http.c functions */
char *getgetenv(CONN *conn, char *query);
char *getpostenv(CONN *conn, char *query);
char *getxmlenv(CONN *conn, char *query);
char *getxmlparam(CONN *conn, int param, char *reqtype);
char *getxmlstruct(CONN *conn, char *reqmember, char *reqtype);
char *get_mime_type(CONN *conn, char *name);
int read_header(CONN *conn);
void send_error(CONN *conn, int status, char* title, char* text);
void send_header(CONN *conn, int cacheable, int status, char *extra_header, char *mime_type, int length, time_t mod);
void http_dorequest(CONN *conn);
/* io.c functions */
void flushheader(CONN *conn);
void flushbuffer(CONN *conn);
int prints(CONN *conn, const char *format, ...);
int raw_prints(CONN *conn, const char *format, ...);
int filesend(CONN *conn, char *file);
int closeconnect(CONN *conn, int exitflag);
/* httpd_lang.c */
char *lang_gets(CONN *conn, char *sect, char *label);
/* httpd_ldir.c */
void   ldir_freeresult(obj_t **qobj);
int    ldir_deleteentry(char *oc, int id, int did);
int    ldir_saveentry(CONN *conn, int id, char *oc, obj_t **qobj);
obj_t *ldir_getlist(nsp_state *N, char *oc, int pid, int did);
void   ldir_sortlist(nsp_state *N, obj_t *qobj, char *key, char *subtab, int order);
obj_t *ldir_getentry(nsp_state *N, char *oc, char *name, int id, int did);
int    ldir_numentries(obj_t **qobj);
char  *ldir_getval(obj_t **qobj, int pid, char *name);
/* log.c functions */
int db_log_activity(CONN *conn, char *category, int indexid, char *action, const char *format, ...);
void log_htaccess(CONN *conn);
/* modctl.c functions */
int module_exists(char *mod_name);
void *module_call(char *fn_name);
int module_menucall(CONN *conn);
int module_load(char *modname);
int modules_init();
/* httpd_nsp.c */
int htnsp_flush(nsp_state *N);
int htnsp_runinit(CONN *conn);
int htnsp_doscript(CONN *conn);
int htnsp_doscript_htdocs(CONN *conn, char *dir, char *file);
int htnsp_dotemplate(CONN *conn, char *dir, char *file);
/* httpd_sql.c functions */
int sql_permprune(CONN *conn, obj_t **qobj, char *permtype);
