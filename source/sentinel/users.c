/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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
#include "main.h"

void useredit(int sid)
{
	char birthdate[16];
	char hiredate[16];
	int i;
	int userid;
	int sqr;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestURI, "/adminusereditnew")==0) {
		userid=0;
		if ((sqr=sqlQuery("SELECT * FROM gw_users WHERE userid = 1"))<0) return;
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
			conn[sid].dat->tuple[i].value[0]='\0';
		}
		sqlFreeconnect(sqr);
		strncpy(conn[sid].dat->tuple[0].value, "0", sizeof(conn[sid].dat->tuple[0].value)-1);
	} else {
		if (getgetenv(sid, "USERID")==NULL) return;
		userid=atoi(getgetenv(sid, "USERID"));
		if ((sqr=sqlQueryf("SELECT * FROM gw_users WHERE userid = %d", userid))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No matching record found for %d</CENTER>\n", userid);
			sqlFreeconnect(sqr);
			return;
		}
		for (i=0;i<sqlNumfields(sqr);i++) {
			strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
			strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		}
		sqlFreeconnect(sqr);
		snprintf(birthdate, sizeof(birthdate)-1, "%s", field(USERFIELDS, "birthdate"));
		snprintf(hiredate, sizeof(hiredate)-1, "%s", field(USERFIELDS, "hiredate"));
	}
	prints("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	prints("<!--\n");
	prints("function ConfirmDelete() {\n");
	prints("	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/adminusersave NAME=useredit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=userid VALUE='%s'>\n", field(USERFIELDS, "userid"));
	prints("<INPUT TYPE=hidden NAME=cdatetime VALUE='%s'>\n", field(USERFIELDS, "cdatetime"));
	prints("<INPUT TYPE=hidden NAME=mdatetime VALUE='%s'>\n", field(USERFIELDS, "mdatetime"));
	prints("<INPUT TYPE=hidden NAME=lastloginip VALUE='%s'>\n", field(USERFIELDS, "lastloginip"));
	prints("<INPUT TYPE=hidden NAME=lastlogintime VALUE='%s'>\n", field(USERFIELDS, "lastlogintime"));
	prints("<INPUT TYPE=hidden NAME=lastlogintoken VALUE='%s'>\n", field(USERFIELDS, "lastlogintoken"));
	prints("<INPUT TYPE=hidden NAME=groupid VALUE='%d'>\n", atoi(field(USERFIELDS, "groupid")));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>ACCOUNT INFORMATION - ", COLOR_TRIM);
	if (userid!=0) {
		prints("User %d</TH></TR>\n", userid);
	} else {
		prints("New User</TH></TR>\n");
	}
	prints("<TR BGCOLOR=%s><TD ALIGN=left VALIGN=top>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Username       </B></TD><TD><INPUT TYPE=TEXT NAME=username VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "username"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Password       </B></TD><TD><INPUT TYPE=PASSWORD NAME=password VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "password"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Allow Login    </B></TD><TD><SELECT NAME=enabled>\n", COLOR_EDITFORM, field(USERFIELDS, "emailaddress"));
	if (atoi(field(USERFIELDS, "enabled"))) {
		prints("<OPTION VALUE=0>No\n<OPTION VALUE=1 SELECTED>Yes\n");
	} else {
		prints("<OPTION VALUE=0 SELECTED>No\n<OPTION VALUE=1>Yes\n");
	}
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail Address </B></TD><TD><INPUT TYPE=TEXT NAME=emailaddress VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "emailaddress"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail Username</B></TD><TD><INPUT TYPE=TEXT NAME=emailusername VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "emailusername"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=emailpassword VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "emailpassword"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>POP3 Server    </B></TD><TD><INPUT TYPE=TEXT NAME=pop3server VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "pop3server"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SMTP Server    </B></TD><TD><INPUT TYPE=TEXT NAME=smtpserver VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "smtpserver"));
	prints("</TABLE>\n</TD><TD ALIGN=center VALIGN=top><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Administration </B></TD><TD><SELECT NAME=authadmin>\n", COLOR_EDITFORM);
	numberselect(sid, atoi(field(USERFIELDS, "authadmin")), 0, 4);
	prints("</SELECT></TD>\n");
	prints("<TD NOWRAP><B>Links      </B></TD><TD><SELECT NAME=authlinks>\n");
	numberselect(sid, atoi(field(USERFIELDS, "authlinks")), 0, 4);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Calendar       </B></TD><TD><SELECT NAME=authcalendar>\n", COLOR_EDITFORM);
	numberselect(sid, atoi(field(USERFIELDS, "authcalendar")), 0, 4);
	prints("</SELECT></TD>\n");
	prints("<TD NOWRAP><B>Messages   </B></TD><TD><SELECT NAME=authmessages>\n");
	numberselect(sid, atoi(field(USERFIELDS, "authmessages")), 0, 4);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Contacts       </B></TD><TD><SELECT NAME=authcontacts>\n", COLOR_EDITFORM);
	numberselect(sid, atoi(field(USERFIELDS, "authcontacts")), 0, 4);
	prints("</SELECT></TD>\n");
	prints("<TD NOWRAP><B>Orders     </B></TD><TD><SELECT NAME=authorders>\n");
	numberselect(sid, atoi(field(USERFIELDS, "authorders")), 0, 4);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Files          </B></TD><TD><SELECT NAME=authfiles>\n", COLOR_EDITFORM);
	numberselect(sid, atoi(field(USERFIELDS, "authfiles")), 0, 4);
	prints("</SELECT></TD>\n");
	prints("<TD NOWRAP><B>Queries    </B></TD><TD><SELECT NAME=authquery>\n");
	numberselect(sid, atoi(field(USERFIELDS, "authquery")), 0, 4);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Forums         </B></TD><TD><SELECT NAME=authforums>\n", COLOR_EDITFORM);
	numberselect(sid, atoi(field(USERFIELDS, "authforums")), 0, 4);
	prints("</SELECT></TD>\n");
	prints("<TD NOWRAP><B>Web E-Mail </B></TD><TD><SELECT NAME=authwebmail>\n");
	numberselect(sid, atoi(field(USERFIELDS, "authwebmail")), 0, 4);
	prints("</SELECT></TD></TR>\n");
	prints("</TABLE></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2>PERSONAL INFORMATION</TH></TR>\n", COLOR_TRIM);
	prints("<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Surname       </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=lastname    value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "lastname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Given Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=firstname   value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "firstname"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Title         </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle    value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "jobtitle"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Division      </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=division    value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "division"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Supervisor    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=supervisor  value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "supervisor"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Address       </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "address"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>City, Province</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=city        value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "city"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Country       </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=country     value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "country"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Postal Code   </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=postalcode  value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "postalcode"));
	prints("</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Home Phone    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber  value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "homenumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Work Phone    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber  value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "worknumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Cell Phone    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=cellnumber  value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "cellnumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>FAX           </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber   value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "faxnumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Pager         </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pagernumber value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "pagernumber"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Date of Birth </B></TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints("<SELECT NAME=birthdate2>\n");
	monthselect(sid, birthdate);
	prints("</SELECT>");
	prints("<SELECT NAME=birthdate1>\n");
	dayselect(sid, birthdate);
	prints("</SELECT>");
	prints("<SELECT NAME=birthdate3>\n");
	yearselect(sid, 1900, birthdate);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Hire Date     </B></TD><TD ALIGN=RIGHT>\n", COLOR_EDITFORM);
	prints("<SELECT NAME=hiredate2>\n");
	monthselect(sid, hiredate);
	prints("</SELECT>");
	prints("<SELECT NAME=hiredate1>\n");
	dayselect(sid, hiredate);
	prints("</SELECT>");
	prints("<SELECT NAME=hiredate3>\n");
	yearselect(sid, 1900, hiredate);
	prints("</SELECT></TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>S.I.N.        </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sin         value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "sin"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Active        </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=isactive    value='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "isactive"));
	prints("</TABLE>\n</TD></TR>\n");
	prints("<TR BGCOLOR=%s><TD COLSPAN=2><B>Notes</B></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=virtual NAME=comment ROWS=5 COLS=60>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "comment"));
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	if ((auth(sid, "admin")>3)&&(userid>1)) {
		prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.useredit.username.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void userlist(int sid)
{
	int i;
	int sqr;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQuery("SELECT userid, username, lastname, firstname FROM gw_users ORDER BY userid ASC"))<0) return;
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", COLOR_TRIM);
	prints("<TH NOWRAP>User ID</TH><TH NOWRAP>User Name</TH><TH NOWRAP>Real Name</TH></TR>\n");
	for (i=0;i<sqlNumtuples(sqr);i++) {
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		prints("<TD ALIGN=RIGHT NOWRAP><A HREF=%s/adminuseredit?userid=%s>%s</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 0));
		prints("<TD NOWRAP><A HREF=%s/adminuseredit?userid=%s>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
		prints("%s&nbsp;</A></TD>", sqlGetvalue(sqr, i, 1));
		prints("<TD NOWRAP>%s", sqlGetvalue(sqr, i, 2));
		if (strlen(sqlGetvalue(sqr, i, 2))&&strlen(sqlGetvalue(sqr, i, 3))) prints(", ");
		prints("%s&nbsp;</TD>", sqlGetvalue(sqr, i, 3));
		prints("</TR>\n");
	}
	prints("</TABLE>\n");
	sqlFreeconnect(sqr);
	prints("<A HREF=%s/adminusereditnew>New User</A>\n", conn[sid].dat->in_ScriptName);
	prints("</CENTER>\n");
	return;
}

void usersave(int sid)
{
	time_t t;
	char query[4096];
	int userid;
	int i;
	int sqr;

	if (auth(sid, "admin")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQuery("SELECT * FROM gw_users WHERE userid = 1"))<0) return;
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (strcmp(conn[sid].dat->tuple[i].name, "birthdate")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s-", getpostenv(sid, "BIRTHDATE3"));
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "BIRTHDATE2"));
			strcat(conn[sid].dat->tuple[i].value, "-");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "BIRTHDATE1"));
		} else if (strcmp(conn[sid].dat->tuple[i].name, "hiredate")==0) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "%s-", getpostenv(sid, "HIREDATE3"));
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "HIREDATE2"));
			strcat(conn[sid].dat->tuple[i].value, "-");
			strcat(conn[sid].dat->tuple[i].value, getpostenv(sid, "HIREDATE1"));
		} else if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL) {
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
		} else if (strncmp(conn[sid].dat->tuple[i].name, "auth", 4)==0) {
			strncpy(conn[sid].dat->tuple[i].value, "0", sizeof(conn[sid].dat->tuple[i].value)-1);
		}
	}
	userid=atoi(conn[sid].dat->tuple[0].value);
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		if (((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(userid==0))||(strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0)) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "cdatetime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "lastlogintime")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "1900-01-01 00:00:00");
		}
		if ((strcmp(conn[sid].dat->tuple[i].name, "enabled")==0)&&(strlen(conn[sid].dat->tuple[i].value)==0)) {
			snprintf(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value)-1, "0");
		}
	}
	sqlFreeconnect(sqr);
	if (userid==0) {
		if ((sqr=sqlQueryf("SELECT username FROM gw_users where username like '%s'", field(USERFIELDS, "username")))<0) return;
		if (sqlNumtuples(sqr)>0) {
			prints("<CENTER>User %s already exists</CENTER><BR>\n", field(USERFIELDS, "username"));
			sqlFreeconnect(sqr);
			return;
		}
		sqlFreeconnect(sqr);
		if (strlen(field(USERFIELDS, "username"))<1) {
			prints("<CENTER>Username is too short</CENTER><BR>\n");
			return;
		}
		if ((sqr=sqlQuery("SELECT max(userid) FROM gw_users"))<0) return;
		userid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (userid<1) userid=1;
		sqlFreeconnect(sqr);
		snprintf(conn[sid].dat->tuple[0].value, sizeof(conn[sid].dat->tuple[0].value)-1, "%d", userid);
		strncpy(conn[sid].dat->tuple[7].value, setpass(conn[sid].dat->tuple[7].value), sizeof(conn[sid].dat->tuple[7].value)-1);
		strncpy(query, "INSERT INTO gw_users (", sizeof(query)-1);
		for (i=0;i<USERFIELDS;i++) {
			strcat(query, conn[sid].dat->tuple[i].name);
			if (i<USERFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ") values (");
		for (i=0;i<USERFIELDS;i++) {
			strcat(query, "'");
			strcat(query, conn[sid].dat->tuple[i].value);
			strcat(query, "'");
			if (i<USERFIELDS-1) strcat(query, ", ");
		}
		strcat(query, ")");
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>User added successfully</CENTER><BR>\n");
		logaccess(1, "%s - %s added user %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, userid);
	} else if (strcmp(getpostenv(sid, "SUBMIT"), "Delete")==0) {
		if ((auth(sid, "admin")<4)||(userid<2)) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_users WHERE userid = %d", userid)<0) return;
		prints("<CENTER>User %d deleted successfully</CENTER><BR>\n", userid);
		logaccess(1, "%s - %s deleted user %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, userid);
	} else {
		if (auth(sid, "admin")<4) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if ((sqr=sqlQueryf("SELECT username, password FROM gw_users WHERE userid = %d", userid))<0) return;
		if (sqlNumtuples(sqr)<1) return;
		if (strcmp(sqlGetvalue(sqr, 0, 1), conn[sid].dat->tuple[7].value)!=0)
			strncpy(conn[sid].dat->tuple[7].value, setpass(conn[sid].dat->tuple[7].value), sizeof(conn[sid].dat->tuple[7].value)-1);
		sqlFreeconnect(sqr);
		strncpy(query, "UPDATE gw_users SET ", sizeof(query)-1);
		for (i=2;i<USERFIELDS;i++) {
			strcatf(query, "%s = '%s'", conn[sid].dat->tuple[i].name, conn[sid].dat->tuple[i].value);
			if (i<USERFIELDS-1) strcat(query, ", ");
		}
		strcatf(query, " WHERE userid = %s", conn[sid].dat->tuple[0].value);
		if (sqlUpdate(query)<0) return;
		prints("<CENTER>User %d modified successfully</CENTER><BR>\n", userid);
		logaccess(1, "%s - %s modified user %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, userid);
	}
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/adminuserlist\">\n", conn[sid].dat->in_ScriptName);
	return;
}

void profileedit(int sid)
{
	int i;
	int userid;
	int sqr;

	if (auth(sid, "webmail")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if ((sqr=sqlQueryf("SELECT * FROM gw_users WHERE username = '%s'", conn[sid].dat->in_username))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching record found for %s</CENTER>\n", conn[sid].dat->in_username);
		sqlFreeconnect(sqr);
		return;
	}
	for (i=0;i<sqlNumfields(sqr);i++) {
		strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
	}
	userid=atoi(field(USERFIELDS, "userid"));
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	prints("<FORM METHOD=POST ACTION=%s/profilesave NAME=profileedit>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=hidden NAME=userid VALUE='%s'>\n", field(USERFIELDS, "userid"));
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=2 NOWRAP>User Profile for %s</TH></TR>\n", COLOR_TRIM, conn[sid].dat->in_username);
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>Password       </B></TD><TD><INPUT TYPE=PASSWORD NAME=password VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "password"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail Address </B></TD><TD><INPUT TYPE=TEXT NAME=emailaddress VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "emailaddress"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail Username</B></TD><TD><INPUT TYPE=TEXT NAME=emailusername VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "emailusername"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>E-mail Password</B></TD><TD><INPUT TYPE=PASSWORD NAME=emailpassword VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "emailpassword"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>POP3 Server    </B></TD><TD><INPUT TYPE=TEXT NAME=pop3server VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "pop3server"));
	prints("<TR BGCOLOR=%s><TD NOWRAP><B>SMTP Server    </B></TD><TD><INPUT TYPE=TEXT NAME=smtpserver VALUE='%s' SIZE=22></TD></TR>\n", COLOR_EDITFORM, field(USERFIELDS, "smtpserver"));
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints("<INPUT TYPE=RESET NAME=reset VALUE='Reset'>\n");
	prints("</FORM>\n");
	prints("</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.profileedit.password.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void profileview(int sid)
{
	int i;
	int sqr;

	if ((sqr=sqlQueryf("SELECT * FROM gw_users WHERE username = '%s'", conn[sid].dat->in_username))<0) return;
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>No matching record found for %s</CENTER>\n", conn[sid].dat->in_username);
		sqlFreeconnect(sqr);
		return;
	}
	for (i=0;i<sqlNumfields(sqr);i++) {
		strncpy(conn[sid].dat->tuple[i].value, sqlGetvalue(sqr, 0, i), sizeof(conn[sid].dat->tuple[i].value)-1);
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
	}
	sqlFreeconnect(sqr);
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=6 NOWRAP>ACCOUNT INFORMATION", COLOR_TRIM);
	prints(" [<A HREF=%s/profileedit>edit</A>]", conn[sid].dat->in_ScriptName);
	prints("</TH></TR>\n");
	prints("<TR><TD BGCOLOR=%s><B>Username       </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "username"));
	prints("<TD BGCOLOR=%s><B>Administration</B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authadmin"));
	prints("<TD BGCOLOR=%s><B>Links         </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authlinks"));
	prints("<TR><TD BGCOLOR=%s><B>Password       </B></TD><TD BGCOLOR=%s>[HIDDEN]&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT);
	prints("<TD BGCOLOR=%s><B>Calendar      </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authcalendar"));
	prints("<TD BGCOLOR=%s><B>Messages      </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authmessages"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>E-mail Address </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "emailaddress"));
	prints("<TD BGCOLOR=%s><B>Contacts      </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authcontacts"));
	prints("<TD BGCOLOR=%s><B>Orders        </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authorders"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>E-mail Username</B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "emailusername"));
	prints("<TD BGCOLOR=%s><B>Files         </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authfiles"));
	prints("<TD BGCOLOR=%s><B>Queries       </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authquery"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>E-mail Password</B></TD><TD BGCOLOR=%s>[HIDDEN]&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT);
	prints("<TD BGCOLOR=%s><B>Forums        </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authforums"));
	prints("<TD BGCOLOR=%s NOWRAP><B>Web E-Mail    </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "authwebmail"));
	prints("<TR><TD BGCOLOR=%s><B>POP3 Server    </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "pop3server"));
	prints("<TD BGCOLOR=%s>&nbsp;        </TD><TD BGCOLOR=%s>&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT);
	prints("<TD BGCOLOR=%s>&nbsp;        </TD><TD BGCOLOR=%s>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT);
	prints("<TR><TD BGCOLOR=%s><B>SMTP Server    </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "smtpserver"));
	prints("<TD BGCOLOR=%s>&nbsp;        </TD><TD BGCOLOR=%s>&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT);
	prints("<TD BGCOLOR=%s>&nbsp;        </TD><TD BGCOLOR=%s>&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT);
	prints("<TR BGCOLOR=%s><TH COLSPAN=6>PERSONAL INFORMATION</TH></TR>\n", COLOR_TRIM);
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Surname       </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "lastname"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Home Phone    </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "homenumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Given Name    </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "firstname"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Work Phone    </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "worknumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Title         </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "jobtitle"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Cell Phone    </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "cellnumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Division      </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "division"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>FAX           </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "faxnumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Supervisor    </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "supervisor"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Pager         </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "pagernumber"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Address       </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "address"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Date of Birth </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, sqldate2text(field(USERFIELDS, "birthdate")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>City, Province</B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "city"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Hire Date     </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, sqldate2text(field(USERFIELDS, "hiredate")));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Country       </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "country"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>S.I.N.        </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "sin"));
	prints("<TR><TD BGCOLOR=%s NOWRAP><B>Postal Code   </B></TD><TD BGCOLOR=%s>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "postalcode"));
	prints("    <TD BGCOLOR=%s COLSPAN=2><B>Active        </B></TD><TD BGCOLOR=%s COLSPAN=2 NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FTEXT, field(USERFIELDS, "isactive"));
	prints("<TR BGCOLOR=%s><TD COLSPAN=6><B>Notes</B></TD></TR>\n", COLOR_FNAME);
	prints("<TR BGCOLOR=%s><TD COLSPAN=6>%s&nbsp;</TD></TR>\n", COLOR_FTEXT, field(USERFIELDS, "comment"));
	prints("</TABLE>\n<BR>\n</CENTER>\n");
	return;
}

void profilesave(int sid)
{
	time_t t;
	int userid;
	int i;
	int sqr;

	if (auth(sid, "webmail")<2) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0)
		return;
	if ((sqr=sqlQueryf("SELECT * FROM gw_users WHERE username = '%s'", conn[sid].dat->in_username))<0) return;
	t=time((time_t*)0);
	for (i=0;i<sqlNumfields(sqr);i++) {
		memset(conn[sid].dat->tuple[i].value, 0, sizeof(conn[sid].dat->tuple[i].value));
		strncpy(conn[sid].dat->tuple[i].name, sqlGetfieldname(sqr, i), sizeof(conn[sid].dat->tuple[i].name)-1);
		if (getpostenv(sid, conn[sid].dat->tuple[i].name)!=NULL) {
			strncpy(conn[sid].dat->tuple[i].value, str2sql(getpostenv(sid, conn[sid].dat->tuple[i].name)), sizeof(conn[sid].dat->tuple[i].value)-1);
		}
		if (strcmp(conn[sid].dat->tuple[i].name, "mdatetime")==0) {
			strftime(conn[sid].dat->tuple[i].value, sizeof(conn[sid].dat->tuple[i].value), "%Y-%m-%d %H:%M:%S", localtime(&t));
		}
	}
	sqlFreeconnect(sqr);
	userid=atoi(conn[sid].dat->tuple[0].value);
	if (userid!=0) {
		if ((sqr=sqlQueryf("SELECT username, password FROM gw_users WHERE username = '%s'", conn[sid].dat->in_username))<0) return;
		if (sqlNumtuples(sqr)<1) return;
		if (strcmp(sqlGetvalue(sqr, 0, 1), conn[sid].dat->tuple[7].value)!=0) {
			strncpy(conn[sid].dat->tuple[7].value, setpass(conn[sid].dat->tuple[7].value), sizeof(conn[sid].dat->tuple[7].value)-1);
		}
		sqlFreeconnect(sqr);
		if (sqlUpdatef("UPDATE gw_users SET mdatetime = '%s', password = '%s', emailaddress = '%s', emailusername = '%s', emailpassword = '%s', pop3server = '%s', smtpserver = '%s' WHERE userid = %d", field(USERFIELDS, "mdatetime"), field(USERFIELDS, "password"), field(USERFIELDS, "emailaddress"), field(USERFIELDS, "emailusername"), field(USERFIELDS, "emailpassword"), field(USERFIELDS, "pop3server"), field(USERFIELDS, "smtpserver"), userid)<0) return;
		prints("<CENTER>Profile modified successfully</CENTER><BR>\n");
		logaccess(1, "%s - %s modified profile", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username);
	}
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=/profileview\">\n");
	return;
}

void profilemain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware User Profile");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch02-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/profileedit>EDIT PROFILE</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	flushbuffer(sid);
	if (strncmp(conn[sid].dat->in_RequestURI, "/profileedit", 12)==0)
		profileedit(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/profileview", 12)==0)
		profileview(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/profilesave", 12)==0)
		profilesave(sid);
	printfooter(sid);
	return;
}
