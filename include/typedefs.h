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
/* typedefs */
#ifdef WIN32
struct direct {
	ino_t d_ino;
	int d_reclen;
	int d_namlen;
	char d_name[MAXNAMLEN+1];
};
struct _dircontents {
	char *_d_entry;
	struct _dircontents *_d_next;
};
typedef struct _dirdesc {
	int dd_id;
	long dd_loc;
	struct _dircontents *dd_contents;
	struct _dircontents *dd_cp;
} DIR;
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
/* pthread_ typedefs */
typedef HANDLE pthread_t;
typedef struct thread_attr {
	DWORD dwStackSize;
	DWORD dwCreatingFlag;
	int priority;
} pthread_attr_t;
typedef struct {
	int dummy;
} pthread_condattr_t;
typedef unsigned int uint;
typedef struct {
	uint waiting;
	HANDLE semaphore;
} pthread_cond_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif

typedef struct {
	char version[11];
	char tax1name[21];
	char tax2name[21];
	float tax1percent;
	float tax2percent;
} DBINFO;
typedef struct {
	DBINFO info;
	int RunAsCGI;
	char config_filename[255];
	char colour_topmenu[10];
	char colour_editform[10];
	char colour_fieldname[10];
	char colour_fieldnametext[10];
	char colour_fieldval[10];
	char colour_fieldvaltext[10];
	char colour_th[10];
	char colour_thtext[10];
	char colour_thlink[10];
	char colour_links[10];
	char server_dir_base[255];
	char server_dir_bin[255];
	char server_dir_cgi[255];
	char server_dir_etc[255];
	char server_dir_lib[255];
	char server_dir_var[255];
	char server_dir_var_backup[255];
	char server_dir_var_db[255];
	char server_dir_var_files[255];
	char server_dir_var_htdocs[255];
	char server_dir_var_log[255];
	char server_dir_var_mail[255];
	char server_hostname[64];
	short int server_port;
	short int server_loglevel;
	short int server_maxconn;
	short int server_maxidle;
	char sql_type[32];
	char sql_username[32];
	char sql_password[32];
	char sql_dbname[32];
	char sql_odbc_dsn[200];
	char sql_hostname[64];
	short int sql_port;
	short int sql_maxconn;
} CONFIG;
typedef struct {
	time_t starttime;
	int pages;
	int conns;
	int queries;
	int updates;
} STATS;

typedef struct {
	// incoming data
	char in_Connection[16];
	int  in_ContentLength;
	char in_ContentType[128];
	char in_Cookie[1024];
	char in_Host[64];
	char in_IfModifiedSince[64];
	char in_PathInfo[128];
	char in_Protocol[16];
	char in_QueryString[1024];
	char in_Referer[128];
	char in_RemoteAddr[16];
	int  in_RemotePort;
	char in_RequestMethod[8];
	char in_RequestURI[1024];
	char in_ScriptName[128];
	char in_UserAgent[128];
	// outgoing data
	short int out_status;
	char out_CacheControl[40];
	char out_Connection[20];
	char out_ContentDisposition[20];
	int  out_ContentLength;
	char out_Date[40];
	char out_Expires[40];
	char out_LastModified[64];
	char out_Location[128];
	char out_Pragma[40];
	char out_Protocol[20];
	char out_Server[128];
	char out_SetCookieUser[128];
	char out_SetCookiePass[128];
	char out_ContentType[128];
	unsigned char out_ReplyData[MAX_REPLYSIZE];
	short int out_headdone;
	short int out_bodydone;
	short int out_flushed;
	// user data
	char      user_username[64];
	char      user_token[64];
	short int user_uid;
	short int user_gid;
	short int user_authadmin;
	short int user_authbookmarks;
	short int user_authcalendar;
	short int user_authcalls;
	short int user_authcontacts;
	short int user_authfiles;
	short int user_authforums;
	short int user_authmessages;
	short int user_authorders;
	short int user_authprofile;
	short int user_authqueries;
	short int user_authwebmail;
	short int user_daystart;
	short int user_daylength;
	short int user_mailcurrent;
	short int user_maildefault;
	short int user_menustyle;
	short int user_maxlist;
	short int user_timezone;
	int       user_wmimapmread;
	int       user_wmimapmsize;
	short int user_wmpopport;
	short int user_wmsmtpport;
	short int user_wmsocket;
	short int user_wmconnected;
	short int user_wmremove;
	short int user_wmimapidx;
	short int user_wmcodecstate;
	short int user_wmnumfiles;
	char      user_wmfiles[20][100];
	char      user_wmusername[64];
	char      user_wmpassword[64];
	char      user_wmrealname[64];
	char      user_wmorganization[64];
	char      user_wmreplyto[64];
	char      user_wmservertype[64];
	char      user_wmpopserver[64];
	char      user_wmsmtpserver[64];
	char      user_wmsignature[1025];
	// sql data
	short int lastbuf;
	char      smallbuf[4][2048];
	char      largebuf[12288];
} CONNDATA;
typedef struct {
	pthread_t handle;
	unsigned int id;
	short int state;
	short int socket;
	struct sockaddr_in ClientAddr;
	time_t ctime; // Creation time
	time_t atime; // Last Access time
	char *PostData;
	CONNDATA *dat;
} CONNECTION;
typedef struct {
	unsigned int ctime;
	unsigned int NumFields;
	unsigned int NumTuples;
	char fields[1024];
	char **cursor;
} SQLRES;
typedef struct {
	int in;
	int out;
} pipe_fd;
typedef struct {
	int userid;
	int time;
} u_avail;
typedef struct {
	short int minutes;
	short int dst;
	char *name;
} tzentry;

typedef struct {
	int recordid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
} REC_HEADER;
typedef struct {
	int bookmarkid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int folderid;
	char bookmarkname[51];
	char bookmarkurl[256];
} REC_BOOKMARK;
typedef struct {
	int folderid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int parentid;
	char foldername[51];
} REC_BOOKMARKFOLDER;
typedef struct {
	int callid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int assignedby;
	int assignedto;
	char callname[51];
	time_t callstart;
	time_t callfinish;
	int contactid;
	int action;
	int status;
	char details[1025];
} REC_CALL;
typedef struct {
	int contactid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char loginip[21];
	time_t logintime;
	char logintoken[51];
	char username[51];
	char password[51];
	int enabled;
	short int geozone;
	short int timezone;
	char surname[51];
	char givenname[51];
	char salutation[51];
	char contacttype[51];
	char referredby[51];
	char altcontact[51];
	char prefbilling[51];
	char email[51];
	char homenumber[26];
	char worknumber[26];
	char faxnumber[26];
	char mobilenumber[26];
	char jobtitle[51];
	char organization[51];
	char homeaddress[51];
	char homelocality[51];
	char homeregion[51];
	char homecountry[51];
	char homepostalcode[11];
	char workaddress[51];
	char worklocality[51];
	char workregion[51];
	char workcountry[51];
	char workpostalcode[11];
} REC_CONTACT;
typedef struct {
	int eventid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int assignedby;
	int assignedto;
	char eventname[51];
	int eventtype;
	int contactid;
	int priority;
	int reminder;
	time_t eventstart;
	time_t eventfinish;
	int busy;
	int status;
	int closingstatus;
	char details[1025];
} REC_EVENT;
typedef struct {
	int fileid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char filename[256];
	char filepath[256];
	char filetype[11];
	time_t uldate;
	time_t lastdldate;
	int numdownloads;
	char description[1025];
} REC_FILE;
typedef struct {
	int forumgroupid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char title[51];
	char description[1025];
} REC_FORUMGROUP;
typedef struct {
	int groupid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char groupname[51];
	char availability[169];
	char motd[1025];
	char members[1025];
} REC_GROUP;
typedef struct {
	int mailaccountid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char accountname[51];
	char realname[51];
	char organization[51];
	char address[51];
	char hosttype[12];
	char pophost[51];
	int popport;
	char smtphost[51];
	int smtpport;
	char popusername[51];
	char poppassword[51];
	int lastcount;
	int notify;
	int remove;
	time_t lastcheck;
	char signature[1025];
} REC_MAILACCT;
typedef struct {
	int noteid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char tablename[51];
	int tableindex;
	char notetitle[51];
	char notetext[8193];
} REC_NOTE;
typedef struct {
	int orderid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int contactid;
	int userid;
	time_t orderdate;
	char ordertype[51];
	char paymentmethod[51];
	float paymentdue;
	float paymentreceived;
	char details[1025];
} REC_ORDER;
typedef struct {
	int orderitemid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int orderid;
	int productid;
	float quantity;
	float discount;
	float unitprice;
	float internalcost;
	float tax1;
	float tax2;
} REC_ORDERITEM;
typedef struct {
	int productid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char productname[51];
	char category[51];
	float discount;
	float unitprice;
	float internalcost;
	float tax1;
	float tax2;
	char details[1025];
} REC_PRODUCT;
typedef struct {
	int queryid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char queryname[51];
	char query[1025];
} REC_QUERY;
typedef struct {
	int taskid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	int assignedby;
	int assignedto;
	char taskname[51];
	time_t duedate;
	int priority;
	int reminder;
	int status;
	char details[1025];
} REC_TASK;
typedef struct {
	int userid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char loginip[21];
	time_t logintime;
	char logintoken[51];
	char username[51];
	char password[51];
	int groupid;
	int enabled;
	int authadmin;
	int authbookmarks;
	int authcalendar;
	int authcalls;
	int authcontacts;
	int authfiles;
	int authforums;
	int authmessages;
	int authorders;
	int authprofile;
	int authquery;
	int authwebmail;
	int prefdaystart;
	int prefdaylength;
	int prefmailcurrent;
	int prefmaildefault;
	int prefmaxlist;
	int prefmenustyle;
	short int preftimezone;
	short int prefgeozone;
	char availability[169];
	char surname[51];
	char givenname[51];
	char jobtitle[51];
	char division[51];
	char supervisor[51];
	char address[51];
	char locality[51];
	char region[51];
	char country[51];
	char postalcode[11];
	char homenumber[26];
	char worknumber[26];
	char faxnumber[26];
	char cellnumber[26];
	char pagernumber[26];
	char email[51];
	char birthdate[12];
	char hiredate[12];
	char sin[16];
	char isactive[11];
} REC_USER;
typedef struct {
	int zoneid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_gperm;
	int obj_operm;
	char zonename[51];
} REC_ZONE;

typedef struct {
	char *fn_name;
	void *fn_ptr;
} FUNCTION;

typedef struct {
	char mod_name[40];
	char mod_menuname[40];
	char mod_menuuri[40];
	char fn_name[40];
	char fn_uri[40];
	void *fn_ptr;
} MODULE_MENU;

typedef struct {
	char mod_name[40];
	char fn_name[40];
	void *fn_ptr;
} MODULE_FUNC;

typedef	int  (*MOD_INIT)(CONFIG *, FUNCTION *, MODULE_MENU *, MODULE_FUNC *);
typedef	void (*MOD_MAIN)(CONNECTION *);
typedef	int  (*MOD_MAIL_SYNC)(CONNECTION *, int);
typedef	void (*MOD_NOTES_SUBLIST)(CONNECTION *, char *, int, int);
typedef	void (*MOD_TASKS_LIST)(CONNECTION *, int, int);
