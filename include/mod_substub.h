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
#include "mod_typedefs.h"

/* auth.c functions */
extern MAIN_AUTH_PRIV				auth_priv;
extern MAIN_AUTH_SETCOOKIE			auth_setcookie;
extern MAIN_AUTH_SETPASS			auth_setpass;
/* config.c functions */
extern MAIN_CONFIG_READ				config_read;
extern MAIN_CONFIG_WRITE			config_write;
/* connio.c functions */
extern MAIN_RAW_PRINTS				raw_prints;
extern MAIN_PRINTS				prints;
extern MAIN_PRINTHEX				printhex;
extern MAIN_PRINTHT				printht;
extern MAIN_PRINTLINE				printline;
extern MAIN_PRINTLINE2				printline2;
/* format.c */
extern MAIN_TIME_SQL2DATETEXT			time_sql2datetext;
extern MAIN_TIME_SQL2TIMETEXT			time_sql2timetext;
extern MAIN_TIME_SQL2UNIX			time_sql2unix;
extern MAIN_TIME_TZOFFSET			time_tzoffset;
extern MAIN_TIME_TZOFFSET2			time_tzoffset2;
extern MAIN_TIME_TZOFFSETCON			time_tzoffsetcon;
extern MAIN_TIME_UNIX2SQL			time_unix2sql;
extern MAIN_TIME_UNIX2TEXT			time_unix2text;
extern MAIN_TIME_UNIX2LOTIMETEXT		time_unix2lotimetext;
extern MAIN_TIME_UNIX2TIMETEXT			time_unix2timetext;
extern MAIN_TIME_UNIX2DATETEXT			time_unix2datetext;
extern MAIN_TIME_WMGETDATE			time_wmgetdate;
/* html.c */
extern MAIN_HTPAGE_HEADER			htpage_header;
extern MAIN_HTPAGE_FOOTER			htpage_footer;
extern MAIN_HTPAGE_TOPMENU			htpage_topmenu;
extern MAIN_HTSELECT_CONTACT			htselect_contact;
extern MAIN_HTSELECT_DAY			htselect_day;
extern MAIN_HTSELECT_GROUP			htselect_group;
extern MAIN_HTSELECT_EVENTSTATUS		htselect_eventstatus;
extern MAIN_HTSELECT_HOUR			htselect_hour;
extern MAIN_HTSELECT_NUMBER			htselect_number;
extern MAIN_HTSELECT_MINUTES			htselect_minutes;
extern MAIN_HTSELECT_MONTH			htselect_month;
extern MAIN_HTSELECT_PRIORITY			htselect_priority;
extern MAIN_HTSELECT_QHOURS			htselect_qhours;
extern MAIN_HTSELECT_QMINUTES			htselect_qminutes;
extern MAIN_HTSELECT_REMINDER			htselect_reminder;
extern MAIN_HTSELECT_TIME			htselect_time;
extern MAIN_HTSELECT_TIMEZONE			htselect_timezone;
extern MAIN_HTSELECT_USER			htselect_user;
extern MAIN_HTSELECT_YEAR			htselect_year;
extern MAIN_HTSELECT_ZONE			htselect_zone;
extern MAIN_HTVIEW_CALLACTION			htview_callaction;
extern MAIN_HTVIEW_CONTACT			htview_contact;
extern MAIN_HTVIEW_EVENTCLOSINGSTATUS		htview_eventclosingstatus;
extern MAIN_HTVIEW_EVENTSTATUS			htview_eventstatus;
extern MAIN_HTVIEW_EVENTTYPE			htview_eventtype;
extern MAIN_HTVIEW_HOLIDAY			htview_holiday;
extern MAIN_HTVIEW_REMINDER			htview_reminder;
extern MAIN_HTVIEW_USER				htview_user;
extern MAIN_HTSCRIPT_SHOWPAGE			htscript_showpage;
/* http.c functions */
extern MAIN_GET_MIME_TYPE			get_mime_type;
extern MAIN_GETGETENV				getgetenv;
extern MAIN_GETMIMEENV				getmimeenv;
extern MAIN_GETPOSTENV				getpostenv;
extern MAIN_GETXMLENV				getxmlenv;
extern MAIN_GETXMLPARAM				getxmlparam;
extern MAIN_GETXMLSTRUCT			getxmlstruct;
/* log.c functions */
extern MAIN_DB_LOG_ACTIVITY			db_log_activity;
extern MAIN_LOGACCESS				logaccess;
extern MAIN_LOGERROR				logerror;
/* md5.c functions */
extern MAIN_MD5_INIT				md5_init;
extern MAIN_MD5_UPDATE				md5_update;
extern MAIN_MD5_FINAL				md5_final;
extern MAIN_MD5_CRYPT				md5_crypt;
/* sql.c functions */
extern MAIN_SQL_FREERESULT			sql_freeresult;
extern MAIN_SQL_GETNAME				sql_getname;
extern MAIN_SQL_GETVALUE			sql_getvalue;
extern MAIN_SQL_GETVALUEBYNAME			sql_getvaluebyname;
extern MAIN_SQL_NUMFIELDS			sql_numfields;
extern MAIN_SQL_NUMTUPLES			sql_numtuples;
extern MAIN_SQL_QUERY				sql_query;
extern MAIN_SQL_QUERYF				sql_queryf;
extern MAIN_SQL_UPDATE				sql_update;
extern MAIN_SQL_UPDATEF				sql_updatef;

extern MAIN_DECODEURL				decodeurl;
extern MAIN_ENCODEURL				encodeurl;
extern MAIN_DECODE_B64S				decode_b64s;
extern MAIN_GETBUFFER				getbuffer;
extern MAIN_STR2HTML				str2html;
extern MAIN_STRNCATF				strncatf;
extern MAIN_SEND_ERROR				send_error;
extern MAIN_SEND_HEADER				send_header;
extern MAIN_FIXSLASHES				fixslashes;
extern MAIN_STRIPRN				striprn;
extern MAIN_FLUSHBUFFER				flushbuffer;
extern MAIN_STR2SQL				str2sql;
extern MAIN_STR2SQLBUF				str2sqlbuf;
extern MAIN_FILESEND				filesend;
extern MAIN_TCP_SEND				tcp_send;
extern MAIN_HEX2INT				hex2int;
extern MAIN_CLOSECONNECT			closeconnect;
extern MAIN_P_STRCASESTR			p_strcasestr;
extern MAIN_SRV_RESTART				srv_restart;
extern MAIN_MODULE_EXISTS			module_exists;
extern MAIN_MODULE_CALL				module_call;
#ifdef WIN32
extern MAIN_GETTIMEOFDAY			gettimeofday;
extern MAIN_OPENDIR				opendir;
extern MAIN_READDIR				readdir;
extern MAIN_CLOSEDIR				closedir;
#endif

#ifdef WIN32
#define msleep(x) Sleep(x)
#else
#define msleep(x) usleep(x*1000)
#endif

extern CONFIG   *config;
extern FUNCTION *functions;
extern _PROC    *proc;
