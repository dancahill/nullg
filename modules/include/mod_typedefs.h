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
/* typedefs */
typedef struct {
	int bookmarkid;
	time_t obj_ctime;
	time_t obj_mtime;
	int obj_uid;
	int obj_gid;
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
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
	int obj_did;
	int obj_gperm;
	int obj_operm;
	char zonename[51];
} REC_ZONE;

typedef	int  (*MOD_CONTACTS_READ)(CONN *, short int, int, REC_CONTACT *);
typedef	int  (*MOD_CONTACTS_WRITE)(CONN *, int, REC_CONTACT *);
