/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
#include "core/mod.h"
#include "httpd/typedefs.h"
#include "httpd_lang.h"

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

/* auth.c functions */
char *auth_setpass(CONN *sid, char *rpassword);
int auth_getcookie(CONN *sid);
int auth_setcookie(CONN *sid);
void auth_logout(CONN *sid);
int auth_priv(CONN *sid, char *service);
/* conf.c functions */
int conf_read(void);
int conf_read_modules(void);
/* format.c functions */
char *getbuffer(CONN *sid);
int hex2int(char *pChars);
void swapchar(char *string, char oldchar, char newchar);
void decodeurl(unsigned char *src);
char *encodeurl(CONN *sid, unsigned char *src);
char *str2html(CONN *sid, char *instring);
int printhex(CONN *sid, const char *format, ...);
int printht(CONN *sid, const char *format, ...);
void printline(CONN *sid, short int reply, char *msgtext);
void printline2(CONN *sid, int dowrap, char *msgtext);
void htselect_timezone(CONN *sid, short int selected);
char *time_sql2datetext(CONN *sid, char *sqldate);
char *time_sql2timetext(CONN *sid, char *sqldate);
char *time_unix2text(CONN *sid, time_t unixdate);
char *time_unix2datetext(CONN *sid, time_t unixdate);
char *time_unix2timetext(CONN *sid, time_t unixdate);
char *time_unix2lotimetext(CONN *sid, time_t unixdate);
char *time_unix2sqldate(CONN *sid, time_t unixdate);
char *time_unix2sqltime(CONN *sid, time_t unixdate);
int time_tzoffset(CONN *sid, time_t unixdate);
int time_tzoffset2(time_t unixdate, int userid);
int time_tzoffsetcon(time_t unixdate, int contactid);
time_t time_wmgetdate(char *src);
/* html.c functions */
void htpage_header(CONN *sid, char *title);
void htpage_footer(CONN *sid);
int  htpage_dirlist(CONN *sid);
void htpage_login(CONN *sid);
void htpage_logout(CONN *sid);
void htpage_frameset(CONN *sid);
void htpage_topmenu(CONN *sid, int menu);
void htpage_motd(CONN *sid);
void htselect_day(CONN *sid, char *selected);
void htselect_hour(CONN *sid, int selected);
void htselect_month(CONN *sid, char *selected);
void htselect_qhours(CONN *sid, int selected);
void htselect_qminutes(CONN *sid, int selected);
void htselect_minutes(CONN *sid, int selected);
void htselect_time(CONN *sid, time_t unixtime);
void htselect_year(CONN *sid, int startyear, char *selected);
void htselect_contact(CONN *sid, int selected);
void htselect_domain(CONN *sid, int selected);
void htselect_eventstatus(CONN *sid, int selected);
void htselect_group(CONN *sid, int perm, int selected, int domainid);
void htselect_number(CONN *sid, int selected, int start, int end, int increment);
void htselect_priority(CONN *sid, int selected);
void htselect_reminder(CONN *sid, int selected);
void htselect_user(CONN *sid, int selected);
void htselect_zone(CONN *sid, int selected, int domainid);
char *htview_callaction(CONN *sid, int selected);
char *htview_contact(CONN *sid, int selected);
char *htview_domain(CONN *sid, int selected);
char *htview_eventclosingstatus(CONN *sid, int selected);
char *htview_eventstatus(int selected);
char *htview_eventtype(CONN *sid, int selected);
char *htview_holiday(char *date);
char *htview_reminder(CONN *sid, int selected);
char *htview_user(CONN *sid, int selected);
void htscript_showpage(CONN *sid, short int pages);
/* http.c functions */
char *get_mime_type(char *name);
void ReadPOSTData(CONN *sid);
char *getgetenv(CONN *sid, char *query);
char *getmimeenv(CONN *sid, char *query, unsigned int *buffersize);
char *getpostenv(CONN *sid, char *query);
char *getxmlenv(CONN *sid, char *query);
char *getxmlparam(CONN *sid, int param, char *reqtype);
char *getxmlstruct(CONN *sid, char *reqmember, char *reqtype);
void read_cgienv(CONN *sid);
int read_header(CONN *sid);
void send_error(CONN *sid, int status, char* title, char* text);
void send_header(CONN *sid, int cacheable, int status, char *extra_header, char *mime_type, int length, time_t mod);
void send_fileheader(CONN *sid, int cacheable, int status, char *extra_header, char *mime_type, int length, time_t mod);
void http_dorequest(CONN *sid);
/* io.c functions */
void flushheader(CONN *sid);
void flushbuffer(CONN *sid);
int prints(CONN *sid, const char *format, ...);
int raw_prints(CONN *sid, const char *format, ...);
int filesend(CONN *sid, char *file);
int closeconnect(CONN *sid, int exitflag);
/* lang.c functions */
int lang_read();
/* log.c functions */
int db_log_activity(CONN *sid, char *category, int indexid, char *action, const char *format, ...);
void log_htaccess(CONN *sid);
/* modctl.c functions */
int module_exists(char *mod_name);
void *module_call(char *fn_name);
int module_menucall(CONN *sid);
int module_load(char *modname);
int modules_init();

#ifdef SRVMOD_MAIN
/*	CONN *conn; */
	HTTP_PROC http_proc;
	pthread_mutex_t ListenerMutex;
	HTTP_CONFIG *mod_config;
#else
/*	extern CONN *conn; */
	extern HTTP_PROC http_proc;
	extern pthread_mutex_t ListenerMutex;
	extern HTTP_CONFIG *mod_config;
#endif
