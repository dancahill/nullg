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
#include "mod_admin.h"
#ifndef WIN32
#include <dirent.h>
#endif

static int dirmake(const char *format, ...)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	struct stat sb;
	char dirname[512];
	va_list ap;

	memset(dirname, 0, sizeof(dirname));
	va_start(ap, format);
	vsnprintf(dirname, sizeof(dirname)-1, format, ap);
	va_end(ap);
	fixslashes(dirname);
	if (stat(dirname, &sb)==0) return 0;
#ifdef WIN32
	if (mkdir(dirname)!=0) {
#else
	if (mkdir(dirname, ~(int)nes_getnum(proc->N, confobj, "umask")&0777)!=0) {
#endif
		return -1;
	}
	return 0;
}

void htselect_lang(CONN *sid, char *selected)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR  *handle;
	char dirname[512];

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/share/locale", nes_getstr(proc->N, confobj, "var_path"));
	fixslashes(dirname);
	handle=opendir(dirname);
	while ((dentry=readdir(handle))!=NULL) {
//		stat(dentry->d_name, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if (strcmp("..", dentry->d_name)==0) continue;
		prints(sid, "<OPTION VALUE='%s'%s>%s\r\n", dentry->d_name, (strcmp(dentry->d_name, selected)==0)?" SELECTED":"", dentry->d_name);
	}
	closedir(handle);
	return;
}

void htselect_theme(CONN *sid, char *selected)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR  *handle;
	char dirname[512];

	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/share/htdocs/nullgs/themes", nes_getstr(proc->N, confobj, "var_path"));
	fixslashes(dirname);
	handle=opendir(dirname);
	while ((dentry=readdir(handle))!=NULL) {
//		stat(dentry->d_name, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if (strcmp("..", dentry->d_name)==0) continue;
		prints(sid, "<OPTION VALUE='%s'%s>%s\r\n", dentry->d_name, (strcmp(dentry->d_name, selected)==0)?" SELECTED":"", dentry->d_name);
	}
	closedir(handle);
	return;
}

void adminuseredit(CONN *sid, obj_t **qobj)
{
/*
	SQLRES sqr1;
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_USER userrec;
	char maddr[250];
	char *ptemp;
	int userid;
	int tz1;
	int tz2;
	time_t t;
	int i, j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (user==NULL) {
		if (strcmp(sid->dat->in_RequestURI, "/admin/usereditnew")==0) {
			userid=0;
			dbread_user(sid, 2, 0, &userrec);
		} else {
			if ((ptemp=getgetenv(sid, "USERID"))==NULL) return;
			userid=atoi(ptemp);
			if (dbread_user(sid, 2, userid, &userrec)!=0) {
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", userid);
				return;
			}
		}
		user=&userrec;
	}
	tz1=time_tzoffset(sid, time(NULL));
	tz2=time_tzoffset2(time(NULL), user->userid);
	if (user->userid<1) tz2=tz1;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	htscript_showpage(sid, 6);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (user->userid!=0) {
		prints(sid, "<B>User %d</B>\r\n", user->userid);
	} else {
		prints(sid, "<B>New User</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usersave NAME=useredit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\r\n", user->userid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid' WIDTH=100%%>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ALIGN=CENTER ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SUMMARY</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>USER</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>NAME</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=4 HREF=javascript:showpage(4)>ADDRESS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER ID=page5tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=5 HREF=javascript:showpage(5)>PREFERENCES</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER ID=page6tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=6 HREF=javascript:showpage(6)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	if (sql_queryf(&sqr1, "SELECT gw_groups.groupid, gw_groups.groupname FROM gw_groups, gw_groups_members WHERE gw_groups_members.groupid = gw_groups.groupid AND gw_groups_members.userid = %d AND gw_groups.obj_did = %d AND gw_groups_members.obj_did = %d ORDER BY gw_groups.groupname ASC", user->userid, user->domainid, user->domainid)<0) return;
	if (user->userid!=0) {
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Name&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
		prints(sid, "%s%s", str2html(sid, user->givenname), strlen(user->givenname)?" ":"");
		prints(sid, "%s%s", str2html(sid, user->surname), strlen(user->surname)?" ":"");
		prints(sid, " (%s)", str2html(sid, user->username));
		prints(sid, "</TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;E-Mail Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
		if (strlen(user->email)==0) {
			prints(sid, "&nbsp;</TD></TR>\r\n");
		} else if (sid->dat->maildefault==0) {
			prints(sid, "<A HREF=\"mailto:%s\">%s</A>&nbsp;</TD></TR>\r\n", user->email, str2html(sid, user->email));
		} else {
			if (sid->dat->menustyle>0) {
				prints(sid, "<A HREF=\"javascript:MsgTo('");
				prints(sid, "&quot;%s%s%s&quot;", str2html(sid, user->givenname), strlen(user->givenname)?" ":"", str2html(sid, user->surname));
				prints(sid, " <%s>')\">%s</A>&nbsp;</TD></TR>\r\n", user->email, str2html(sid, user->email));
			} else {
				prints(sid, "<A HREF=\"/mail/write?to=%s\">%s</A>&nbsp;</TD></TR>\r\n", user->email, str2html(sid, user->email));
			}
		}
		memset(maddr, 0, sizeof(maddr));
		if (strlen(user->address)&&strlen(user->locality)&&strlen(user->region)&&strlen(user->country)) {
			snprintf(maddr, sizeof(maddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", user->country, user->address, user->locality, user->region);
		}
		ptemp=maddr;
		while (*ptemp!=0) {
			if (*ptemp==' ') *ptemp='+';
			ptemp++;
		}
		if (strlen(maddr)) {
			prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>Map Available</A>&nbsp;</TD></TR>\r\n", maddr);
		}
		if (tz1!=tz2) {
			prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Time&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><SPAN ID=contz></SPAN>&nbsp;</TD></TR>\r\n");
		}
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2 NOWRAP><HR></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2 NOWRAP><B>&nbsp;Groups   &nbsp;</B>(<A HREF=%s/admin/groupmembereditnew?userid=%d>add</A>)</TD></TR>\r\n", sid->dat->in_ScriptName, user->userid);
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2 WIDTH=100%% NOWRAP style='cursor:hand' onClick=\"window.location.href='%s/admin/groupedit?groupid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			prints(sid, "&nbsp;&nbsp;<A HREF=%s/admin/groupedit?groupid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr1, i, 1)));
		}
		prints(sid, "</TABLE>\r\n");
	}
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Username&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username VALUE=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->username));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password VALUE=\"\" SIZE=45 style='width:255px'></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Default Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=groupid style='width:255px'>\r\n");
	prints(sid, "<OPTION VALUE='0'>\r\n");
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		j=atoi(sql_getvalue(&sqr1, i, 0));
		prints(sid, "<OPTION VALUE='%d'%s>%s\r\n", j, j==user->groupid?" SELECTED":"", str2html(sid, sql_getvalue(&sqr1, i, 1)));
	}
	sql_freeresult(&sqr1);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Domain  &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=domainid style='width:255px'%s>\r\n", !(auth_priv(sid, "domainadmin")&A_ADMIN)||(user->userid>0)?" DISABLED":"");
	htselect_domain(sid, user->domainid);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Allow Login&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:255px'%s>\r\n", user->userid==1?" DISABLED":"");
	if ((user->userid==1)||(user->enabled)) {
		prints(sid, "<OPTION VALUE=0>No\r\n<OPTION VALUE=1 SELECTED>Yes\r\n");
	} else {
		prints(sid, "<OPTION VALUE=0 SELECTED>No\r\n<OPTION VALUE=1>Yes\r\n");
	}
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Surname&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->surname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Given Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname   value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->givenname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Job Title&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle     value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->jobtitle));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Division&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=division      value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->division));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Supervisor&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=supervisor  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->supervisor));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Date of Birth&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=birthdate2>\r\n");
	htselect_month(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate1>\r\n");
	htselect_day(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate3>\r\n");
	htselect_year(sid, 1900, user->birthdate);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Hire Date&nbsp;</B></TD><TD ALIGN=RIGHT>\r\n");
	prints(sid, "<SELECT NAME=hiredate2>\r\n");
	htselect_month(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate1>\r\n");
	htselect_day(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate3>\r\n");
	htselect_year(sid, 1900, user->hiredate);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;S.I.N.&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sin         value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->sin));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Active&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=isactive    value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->isactive));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->homenumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->worknumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;FAX&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber          value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->faxnumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Cell Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=cellnumber  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->cellnumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Pager&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pagernumber      value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->pagernumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;E-Mail&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email           value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->email));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->address));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;City&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=locality          value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->locality));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Province&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=region        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->region));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Country&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=country        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->country));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=postalcode value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\r\n", str2html(sid, user->postalcode));
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page5 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if (module_exists("mod_calendar")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Start&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaystart style='width:255px'>\r\n");
		htselect_hour(sid, user->prefdaystart);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Length&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaylength style='width:255px'>\r\n");
		htselect_number(sid, user->prefdaylength, 0, 24, 1);
		prints(sid, "</SELECT></TD></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Max Results/Page&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefmaxlist style='width:255px'>\r\n");
	htselect_number(sid, user->prefmaxlist, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Time Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=preftimezone style='width:255px'>\r\n");
	htselect_timezone(sid, user->preftimezone);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefgeozone style='width:255px'>\r\n");
	if (user->userid==0) {
		htselect_zone(sid, user->prefgeozone, 0);
	} else {
		htselect_zone(sid, user->prefgeozone, user->domainid);
	}
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Language&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=preflanguage style='width:255px'>\r\n");
	htselect_lang(sid, user->preflanguage);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Theme&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=preftheme style='width:255px'>\r\n");
	htselect_theme(sid, user->preftheme);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page6 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%>&nbsp;</TD><TD ALIGN=CENTER><B>Read&nbsp;</B></TD><TD ALIGN=CENTER><B>Modify&nbsp;</B></TD><TD ALIGN=CENTER><B>Insert&nbsp;</B></TD><TD ALIGN=CENTER><B>Delete&nbsp;</B></TD><TD ALIGN=CENTER><B>Admin</B></TD></TR>");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Domain Administration&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	if (user->userid==1) {
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authdomainadmin_a VALUE='1' CHECKED DISABLED></TD>");
	} else {
		if (auth_priv(sid, "domainadmin")&A_ADMIN) {
			prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authdomainadmin_a VALUE='1' %s></TD>", (user->authdomainadmin&A_ADMIN)?"checked":"");
		} else {
			prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authdomainadmin_a VALUE='1' %s DISABLED></TD>", (user->authdomainadmin&A_ADMIN)?"checked":"");
		}
	}
	prints(sid, "</TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Administration&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	if (user->userid==1) {
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authadmin_a VALUE='1' CHECKED DISABLED></TD>");
	} else {
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authadmin_a VALUE='1' %s></TD>", (user->authadmin&A_ADMIN)?"checked":"");
	}
	prints(sid, "</TR>\r\n");
	if (module_exists("mod_bookmarks")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Bookmarks&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_r VALUE='1' %s></TD>", (user->authbookmarks&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_m VALUE='1' %s></TD>", (user->authbookmarks&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_i VALUE='1' %s></TD>", (user->authbookmarks&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_d VALUE='1' %s></TD>", (user->authbookmarks&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_a VALUE='1' %s></TD>", (user->authbookmarks&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_calendar")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Calendar&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_r VALUE='1' %s></TD>", (user->authcalendar&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_m VALUE='1' %s></TD>", (user->authcalendar&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_i VALUE='1' %s></TD>", (user->authcalendar&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_d VALUE='1' %s></TD>", (user->authcalendar&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_a VALUE='1' %s></TD>", (user->authcalendar&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_calls")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Calls&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_r VALUE='1' %s></TD>", (user->authcalls&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_m VALUE='1' %s></TD>", (user->authcalls&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_i VALUE='1' %s></TD>", (user->authcalls&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_d VALUE='1' %s></TD>", (user->authcalls&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_a VALUE='1' %s></TD>", (user->authcalls&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_contacts")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Contacts&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_r VALUE='1' %s></TD>", (user->authcontacts&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_m VALUE='1' %s></TD>", (user->authcontacts&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_i VALUE='1' %s></TD>", (user->authcontacts&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_d VALUE='1' %s></TD>", (user->authcontacts&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_a VALUE='1' %s></TD>", (user->authcontacts&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_email")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;E-Mail&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authemail_r VALUE='1' %s></TD>", (user->authemail&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authemail_m VALUE='1' %s></TD>", (user->authemail&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authemail_i VALUE='1' %s></TD>", (user->authemail&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authemail_d VALUE='1' %s></TD>", (user->authemail&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_files")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Files&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_r VALUE='1' %s></TD>", (user->authfiles&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_m VALUE='1' %s></TD>", (user->authfiles&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_i VALUE='1' %s></TD>", (user->authfiles&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_d VALUE='1' %s></TD>", (user->authfiles&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_a VALUE='1' %s></TD>", (user->authfiles&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_finance")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Finances&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfinance_r VALUE='1' %s></TD>", (user->authfinance&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfinance_m VALUE='1' %s></TD>", (user->authfinance&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfinance_i VALUE='1' %s></TD>", (user->authfinance&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfinance_d VALUE='1' %s></TD>", (user->authfinance&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfinance_a VALUE='1' %s></TD>", (user->authfinance&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_forums")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Forums&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authforums_r VALUE='1' %s></TD>", (user->authforums&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authforums_i VALUE='1' %s></TD>", (user->authforums&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authforums_a VALUE='1' %s></TD>", (user->authforums&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_messages")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Messages&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authmessages_r VALUE='1' %s></TD>", (user->authmessages&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authmessages_i VALUE='1' %s></TD>", (user->authmessages&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authmessages_d VALUE='1' %s></TD>", (user->authmessages&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_profile")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Profile&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprofile_r VALUE='1' %s></TD>", (user->authprofile&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprofile_m VALUE='1' %s></TD>", (user->authprofile&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_projects")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Projects&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_r VALUE='1' %s></TD>", (user->authprojects&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_m VALUE='1' %s></TD>", (user->authprojects&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_i VALUE='1' %s></TD>", (user->authprojects&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_d VALUE='1' %s></TD>", (user->authprojects&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_a VALUE='1' %s></TD>", (user->authprojects&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	if (module_exists("mod_searches")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;SQL Queries&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authquery_r VALUE='1' %s></TD>", (user->authquery&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authquery_a VALUE='1' %s></TD>", (user->authquery&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TD NOWRAP COLSPAN=2>");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=users&index=%d>new</A>]", sid->dat->in_ScriptName, user->userid);
		prints(sid, "</FONT></TH></TR>\r\n");
		mod_notes_sublist(sid, "users", user->userid, 1);
		prints(sid, "</TABLE>\r\n");
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(user->userid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	if (user->userid!=0) {
		prints(sid, "[<A HREF=%s/admin/usertimeedit?userid=%d>Edit Availability</A>]\r\n", sid->dat->in_ScriptName, user->userid);
	}
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
//	prints(sid, "document.useredit.username.focus();\r\n");
	prints(sid, "showpage(1);\r\n");
	if (tz1!=tz2) {
		t=time(NULL);
		t+=time_tzoffset(sid, t);
		t+=(tz2-tz1);
		prints(sid, "var t=%d;\r\n", t);
		prints(sid, "var d=new Date();\r\n");
		prints(sid, "d.setTime(t*1000);\r\n");
		prints(sid, "t+=d.getTimezoneOffset()*60;\r\n");
		prints(sid, "function padout(number) { return (number<10)?'0'+number:number; }\r\n");
		prints(sid, "function setDuration() {\r\n");
		prints(sid, "	d.setTime(t*1000);\r\n");
		prints(sid, "	var h=d.getHours();\r\n");
		prints(sid, "	var ap='AM';\r\n");
		prints(sid, "	if (h>11) ap='PM';\r\n");
		prints(sid, "	if (h>12) h-=12;\r\n");
		prints(sid, "	if (h<1) h=12;\r\n");
		prints(sid, "	document.getElementById('contz').innerHTML=h+':'+padout(d.getMinutes())+':'+padout(d.getSeconds())+' '+ap;\r\n");
		prints(sid, "	t++;\r\n");
		prints(sid, "	setTimeout(\"setDuration()\", 1000);\r\n");
		prints(sid, "}\r\n");
		prints(sid, "setDuration();\r\n");
	}
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void adminuserlist(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	int i, j;
	int rc;
	int match;
	int lastdomain;
	int thisdomain;
	obj_t *qobj1=NULL;
	obj_t *qobj2=NULL;
	obj_t *qobj3=NULL;
	obj_t *qobj4=NULL;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		rc=sql_queryf(proc->N, &qobj1, "SELECT userid, groupid, domainid, username, surname, givenname, prefgeozone FROM gw_users ORDER BY domainid, username ASC");
	} else {
		rc=sql_queryf(proc->N, &qobj1, "SELECT userid, groupid, domainid, username, surname, givenname, prefgeozone FROM gw_users WHERE domainid = %d ORDER BY domainid, username ASC", sid->dat->did);
	}
	if (rc<0) return;
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		rc=sql_queryf(proc->N, &qobj2, "SELECT gw_groups_members.userid, gw_groups_members.groupid, gw_groups.groupname FROM gw_groups_members, gw_groups WHERE gw_groups_members.groupid = gw_groups.groupid ORDER BY gw_groups.groupname ASC");
	} else {
		rc=sql_queryf(proc->N, &qobj2, "SELECT gw_groups_members.userid, gw_groups_members.groupid, gw_groups.groupname FROM gw_groups_members, gw_groups WHERE gw_groups_members.groupid = gw_groups.groupid AND gw_groups.obj_did = %d AND gw_groups_members.obj_did = %d ORDER BY gw_groups.groupname ASC", sid->dat->did, sid->dat->did);
//		rc=sql_queryf(proc->N, &qobj2, "SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d ORDER BY groupid ASC", sid->dat->did);
	}
	if (rc<0) {
		sql_freeresult(proc->N, &qobj1);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		rc=sql_queryf(proc->N, &qobj3, "SELECT zoneid, zonename FROM gw_zones ORDER BY zoneid ASC");
	} else {
		rc=sql_queryf(proc->N, &qobj3, "SELECT zoneid, zonename FROM gw_zones WHERE obj_did = %d ORDER BY zoneid ASC", sid->dat->did);
	}
	if (rc<0) {
		sql_freeresult(proc->N, &qobj1);
		sql_freeresult(proc->N, &qobj2);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		rc=sql_queryf(proc->N, &qobj4, "SELECT domainid, domainname FROM gw_domains ORDER BY domainname ASC");
	} else {
		rc=sql_queryf(proc->N, &qobj4, "SELECT domainid, domainname FROM gw_domains WHERE domainid = %d", sid->dat->did);
	}
	if (rc<0) {
		sql_freeresult(proc->N, &qobj1);
		sql_freeresult(proc->N, &qobj2);
		sql_freeresult(proc->N, &qobj3);
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;User Name&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;Real Name&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;Zone&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;Groups&nbsp;</TH>");
	if (auth_priv(sid, "domainadmin")&A_ADMIN) prints(sid, "<TH STYLE='border-style:solid'>&nbsp;</TH>");
	prints(sid, "</TR>\r\n");
	lastdomain=-1;
	if (sql_numtuples(proc->N, &qobj4)<2) {
		lastdomain=atoi(sql_getvalue(proc->N, &qobj1, 0, 2));
	}
	for (i=0;i<sql_numtuples(proc->N, &qobj1);i++) {
		thisdomain=atoi(sql_getvalue(proc->N, &qobj1, i, 2));
		if (lastdomain!=thisdomain) {
			lastdomain=thisdomain;
			prints(sid, "<TR CLASS=\"FIELDNAME\"><TD COLSPAN=5 NOWRAP style='border-style:solid'>");
			for (j=0;j<sql_numtuples(proc->N, &qobj4);j++) {
				if (atoi(sql_getvalue(proc->N, &qobj4, j, 0))==atoi(sql_getvalue(proc->N, &qobj1, i, 2))) {
					prints(sid, "<B>%s</B></TD>", str2html(sid, sql_getvalue(proc->N, &qobj4, j, 1)));
					break;
				}
			}
			if (j==sql_numtuples(proc->N, &qobj4)) {
				prints(sid, "&nbsp;</TD>");
			}
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP VALIGN=top style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", ScriptName, atoi(sql_getvalue(proc->N, &qobj1, i, 0)));
		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", ScriptName, atoi(sql_getvalue(proc->N, &qobj1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(proc->N, &qobj1, i, 3)));
		prints(sid, "<TD NOWRAP VALIGN=top STYLE='border-style:solid'>%s", str2html(sid, sql_getvalue(proc->N, &qobj1, i, 4)));
		if (strlen(sql_getvalue(proc->N, &qobj1, i, 4))&&strlen(sql_getvalue(proc->N, &qobj1, i, 5))) prints(sid, ", ");
		prints(sid, "%s&nbsp;</TD>", str2html(sid, sql_getvalue(proc->N, &qobj1, i, 5)));
		match=0;
		for (j=0;j<sql_numtuples(proc->N, &qobj3);j++) {
			if (atoi(sql_getvalue(proc->N, &qobj3, j, 0))==atoi(sql_getvalue(proc->N, &qobj1, i, 6))) {
				prints(sid, "<TD NOWRAP VALIGN=top STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(proc->N, &qobj3, j, 1)));
				match++;
				break;
			}
		}
		if (!match) prints(sid, "<TD NOWRAP VALIGN=top STYLE='border-style:solid'>&nbsp;</TD>");
		match=0;
		prints(sid, "<TD VALIGN=top STYLE='border-style:solid'>");
		for (j=0;j<sql_numtuples(proc->N, &qobj2);j++) {
			if (atoi(sql_getvalue(proc->N, &qobj2, j, 0))==atoi(sql_getvalue(proc->N, &qobj1, i, 0))) {
				if (match) prints(sid, ", ");
				prints(sid, "<A HREF=%s/admin/groupedit?groupid=%d>%s</A>", ScriptName, atoi(sql_getvalue(proc->N, &qobj2, j, 1)), str2html(sid, sql_getvalue(proc->N, &qobj2, j, 2)));
				match++;
			}
		}
		if (!match) prints(sid, "&nbsp;");
		prints(sid, "</TD>");
		if (auth_priv(sid, "domainadmin")&A_ADMIN) {
			prints(sid, "<TD STYLE='border-style:solid'><A HREF=%s/admin/su?uid=%d&did=%d>su</A></TD>", ScriptName, atoi(sql_getvalue(proc->N, &qobj1, i, 0)), thisdomain);
		}
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<A HREF=%s/admin/usereditnew>New User</A>", ScriptName);
	prints(sid, "</CENTER>");
	sql_freeresult(proc->N, &qobj1);
	sql_freeresult(proc->N, &qobj2);
	sql_freeresult(proc->N, &qobj3);
	sql_freeresult(proc->N, &qobj4);
	return;
}

void adminusersave(CONN *sid)
{
/*
	REC_USER user;
	char query[4096];
	char curdate[40];
	short int newpass=0;
	char *ptemp;
	time_t t;
	int userid;
	SQLRES sqr;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	if (dbread_user(sid, 2, userid, &user)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getpostenv(sid, "USERNAME"))!=NULL) snprintf(user.username, sizeof(user.username)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) {
		if (strlen(ptemp)>0) {
			snprintf(user.password, sizeof(user.password)-1, "%s", ptemp);
			newpass=1;
		}
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "DOMAINID"))!=NULL) user.domainid=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "GROUPID"))!=NULL) user.groupid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ENABLED"))!=NULL) user.enabled=atoi(ptemp);
	if (module_exists("mod_admin")) {
		user.authdomainadmin=0;
		if ((ptemp=getpostenv(sid, "AUTHDOMAINADMIN_R"))!=NULL) user.authdomainadmin+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHDOMAINADMIN_M"))!=NULL) user.authdomainadmin+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHDOMAINADMIN_I"))!=NULL) user.authdomainadmin+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHDOMAINADMIN_D"))!=NULL) user.authdomainadmin+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHDOMAINADMIN_A"))!=NULL) user.authdomainadmin+=A_ADMIN;
	}
	if (module_exists("mod_admin")) {
		user.authadmin=0;
		if ((ptemp=getpostenv(sid, "AUTHADMIN_R"))!=NULL) user.authadmin+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHADMIN_M"))!=NULL) user.authadmin+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHADMIN_I"))!=NULL) user.authadmin+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHADMIN_D"))!=NULL) user.authadmin+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHADMIN_A"))!=NULL) user.authadmin+=A_ADMIN;
	}
	if (module_exists("mod_bookmarks")) {
		user.authbookmarks=0;
		if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_R"))!=NULL) user.authbookmarks+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_M"))!=NULL) user.authbookmarks+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_I"))!=NULL) user.authbookmarks+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_D"))!=NULL) user.authbookmarks+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_A"))!=NULL) user.authbookmarks+=A_ADMIN;
	}
	if (module_exists("mod_calendar")) {
		user.authcalendar=0;
		if ((ptemp=getpostenv(sid, "AUTHCALENDAR_R"))!=NULL) user.authcalendar+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHCALENDAR_M"))!=NULL) user.authcalendar+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHCALENDAR_I"))!=NULL) user.authcalendar+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHCALENDAR_D"))!=NULL) user.authcalendar+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHCALENDAR_A"))!=NULL) user.authcalendar+=A_ADMIN;
	}
	if (module_exists("mod_calls")) {
		user.authcalls=0;
		if ((ptemp=getpostenv(sid, "AUTHCALLS_R"))!=NULL) user.authcalls+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHCALLS_M"))!=NULL) user.authcalls+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHCALLS_I"))!=NULL) user.authcalls+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHCALLS_D"))!=NULL) user.authcalls+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHCALLS_A"))!=NULL) user.authcalls+=A_ADMIN;
	}
	if (module_exists("mod_contacts")) {
		user.authcontacts=0;
		if ((ptemp=getpostenv(sid, "AUTHCONTACTS_R"))!=NULL) user.authcontacts+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHCONTACTS_M"))!=NULL) user.authcontacts+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHCONTACTS_I"))!=NULL) user.authcontacts+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHCONTACTS_D"))!=NULL) user.authcontacts+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHCONTACTS_A"))!=NULL) user.authcontacts+=A_ADMIN;
	}
	if (module_exists("mod_email")) {
		user.authemail=0;
		if ((ptemp=getpostenv(sid, "AUTHEMAIL_R"))!=NULL) user.authemail+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHEMAIL_M"))!=NULL) user.authemail+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHEMAIL_I"))!=NULL) user.authemail+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHEMAIL_D"))!=NULL) user.authemail+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHEMAIL_A"))!=NULL) user.authemail+=A_ADMIN;
	}
	if (module_exists("mod_files")) {
		user.authfiles=0;
		if ((ptemp=getpostenv(sid, "AUTHFILES_R"))!=NULL) user.authfiles+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHFILES_M"))!=NULL) user.authfiles+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHFILES_I"))!=NULL) user.authfiles+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHFILES_D"))!=NULL) user.authfiles+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHFILES_A"))!=NULL) user.authfiles+=A_ADMIN;
	}
	if (module_exists("mod_finance")) {
		user.authfinance=0;
		if ((ptemp=getpostenv(sid, "AUTHFINANCE_R"))!=NULL) user.authfinance+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHFINANCE_M"))!=NULL) user.authfinance+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHFINANCE_I"))!=NULL) user.authfinance+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHFINANCE_D"))!=NULL) user.authfinance+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHFINANCE_A"))!=NULL) user.authfinance+=A_ADMIN;
	}
	if (module_exists("mod_forums")) {
		user.authforums=0;
		if ((ptemp=getpostenv(sid, "AUTHFORUMS_R"))!=NULL) user.authforums+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHFORUMS_M"))!=NULL) user.authforums+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHFORUMS_I"))!=NULL) user.authforums+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHFORUMS_D"))!=NULL) user.authforums+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHFORUMS_A"))!=NULL) user.authforums+=A_ADMIN;
	}
	if (module_exists("mod_messages")) {
		user.authmessages=0;
		if ((ptemp=getpostenv(sid, "AUTHMESSAGES_R"))!=NULL) user.authmessages+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHMESSAGES_M"))!=NULL) user.authmessages+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHMESSAGES_I"))!=NULL) user.authmessages+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHMESSAGES_D"))!=NULL) user.authmessages+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHMESSAGES_A"))!=NULL) user.authmessages+=A_ADMIN;
	}
	if (module_exists("mod_profile")) {
		user.authprofile=0;
		if ((ptemp=getpostenv(sid, "AUTHPROFILE_R"))!=NULL) user.authprofile+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHPROFILE_M"))!=NULL) user.authprofile+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHPROFILE_I"))!=NULL) user.authprofile+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHPROFILE_D"))!=NULL) user.authprofile+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHPROFILE_A"))!=NULL) user.authprofile+=A_ADMIN;
	}
	if (module_exists("mod_projects")) {
		user.authprojects=0;
		if ((ptemp=getpostenv(sid, "AUTHPROJECTS_R"))!=NULL) user.authprojects+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHPROJECTS_M"))!=NULL) user.authprojects+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHPROJECTS_I"))!=NULL) user.authprojects+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHPROJECTS_D"))!=NULL) user.authprojects+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHPROJECTS_A"))!=NULL) user.authprojects+=A_ADMIN;
	}
	if (module_exists("mod_searches")) {
		user.authquery=0;
		if ((ptemp=getpostenv(sid, "AUTHQUERY_R"))!=NULL) user.authquery+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHQUERY_M"))!=NULL) user.authquery+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHQUERY_I"))!=NULL) user.authquery+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHQUERY_D"))!=NULL) user.authquery+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHQUERY_A"))!=NULL) user.authquery+=A_ADMIN;
	}
	if ((ptemp=getpostenv(sid, "PREFDAYSTART"))!=NULL) user.prefdaystart=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYLENGTH"))!=NULL) user.prefdaylength=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAXLIST"))!=NULL) user.prefmaxlist=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFTIMEZONE"))!=NULL) user.preftimezone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFGEOZONE"))!=NULL) user.prefgeozone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFLANGUAGE"))!=NULL) snprintf(user.preflanguage, sizeof(user.preflanguage)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PREFTHEME"))!=NULL) snprintf(user.preftheme, sizeof(user.preftheme)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SURNAME"))!=NULL) snprintf(user.surname, sizeof(user.surname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "GIVENNAME"))!=NULL) snprintf(user.givenname, sizeof(user.givenname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "JOBTITLE"))!=NULL) snprintf(user.jobtitle, sizeof(user.jobtitle)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "DIVISION"))!=NULL) snprintf(user.division, sizeof(user.division)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SUPERVISOR"))!=NULL) snprintf(user.supervisor, sizeof(user.supervisor)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ADDRESS"))!=NULL) snprintf(user.address, sizeof(user.address)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "LOCALITY"))!=NULL) snprintf(user.locality, sizeof(user.locality)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "REGION"))!=NULL) snprintf(user.region, sizeof(user.region)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "COUNTRY"))!=NULL) snprintf(user.country, sizeof(user.country)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "POSTALCODE"))!=NULL) snprintf(user.postalcode, sizeof(user.postalcode)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMENUMBER"))!=NULL) snprintf(user.homenumber, sizeof(user.homenumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKNUMBER"))!=NULL) snprintf(user.worknumber, sizeof(user.worknumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "FAXNUMBER"))!=NULL) snprintf(user.faxnumber, sizeof(user.faxnumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "CELLNUMBER"))!=NULL) snprintf(user.cellnumber, sizeof(user.cellnumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PAGERNUMBER"))!=NULL) snprintf(user.pagernumber, sizeof(user.pagernumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "EMAIL"))!=NULL) snprintf(user.email, sizeof(user.email)-1, "%s", ptemp);
	memset(user.birthdate, 0, sizeof(user.birthdate));
	if ((ptemp=getpostenv(sid, "BIRTHDATE3"))!=NULL) snprintf(user.birthdate, sizeof(user.birthdate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "BIRTHDATE2"))!=NULL) strncatf(user.birthdate, sizeof(user.birthdate)-strlen(user.birthdate)-1, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "BIRTHDATE1"))!=NULL) strncatf(user.birthdate, sizeof(user.birthdate)-strlen(user.birthdate)-1, "%02d", atoi(ptemp));
	if (strlen(user.birthdate)!=10) memset(user.birthdate, 0, sizeof(user.birthdate));
	memset(user.hiredate, 0, sizeof(user.hiredate));
	if ((ptemp=getpostenv(sid, "HIREDATE3"))!=NULL) snprintf(user.hiredate, sizeof(user.hiredate)-1, "%d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "HIREDATE2"))!=NULL) strncatf(user.hiredate, sizeof(user.hiredate)-strlen(user.hiredate)-1, "%02d-", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "HIREDATE1"))!=NULL) strncatf(user.hiredate, sizeof(user.hiredate)-strlen(user.hiredate)-1, "%02d", atoi(ptemp));
	if (strlen(user.hiredate)!=10) memset(user.hiredate, 0, sizeof(user.hiredate));
	if ((ptemp=getpostenv(sid, "SIN"))!=NULL) snprintf(user.sin, sizeof(user.sin)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ISACTIVE"))!=NULL) snprintf(user.isactive, sizeof(user.isactive)-1, "%s", ptemp);
	if (user.userid==1) {
		user.enabled=1;
		user.authdomainadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
		user.authadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	}
	if (strlen(user.birthdate)==0) snprintf(user.birthdate, sizeof(user.birthdate)-1, "1900-01-01");
	if (strlen(user.hiredate)==0) snprintf(user.hiredate, sizeof(user.hiredate)-1, "1900-01-01");
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)||(userid<2)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
		if (sql_updatef("DELETE FROM gw_users WHERE userid = %d", user.userid)<0) return;
		prints(sid, "<CENTER>User %d deleted successfully</CENTER><BR />\r\n", userid);
		db_log_activity(sid, "users", user.userid, "delete", "%s - %s deleted user %d", sid->dat->in_RemoteAddr, sid->dat->username, user.userid);
	} else if (user.userid==0) {
		if (sql_queryf(&sqr, "SELECT username FROM gw_users where username = '%s' AND domainid = %d", user.username, user.domainid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>User %s already exists</B></CENTER>\r\n", user.username);
			sql_freeresult(&sqr);
			adminuseredit(sid, &user);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(user.username)<1) {
			prints(sid, "<CENTER><B>Username is too short</B></CENTER>\r\n");
			adminuseredit(sid, &user);
			return;
		}
		snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
		if (sql_query(&sqr, "SELECT max(userid) FROM gw_users")<0) return;
		user.userid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (user.userid<1) user.userid=1;
		memset(query, 0, sizeof(query));
		strcpy(query, "INSERT INTO gw_users (userid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, username, password, groupid, domainid, enabled, authdomainadmin, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authemail, authfiles, authfinance, authforums, authmessages, authprofile, authprojects, authquery, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone, prefgeozone, preflanguage, preftheme, availability, surname, givenname, jobtitle, division, supervisor, address, locality, region, country, postalcode, homenumber, worknumber, faxnumber, cellnumber, pagernumber, email, birthdate, hiredate, sin, isactive) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", user.userid, curdate, curdate, user.obj_uid, user.obj_gid, user.domainid, user.obj_gperm, user.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.password);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.groupid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.domainid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.enabled);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authdomainadmin);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authadmin);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authbookmarks);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authcalendar);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authcalls);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authcontacts);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authemail);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authfiles);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authfinance);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authforums);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authmessages);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authprofile);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authprojects);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authquery);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefdaystart);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefdaylength);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefmailcurrent);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefmaildefault);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefmaxlist);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefmenustyle);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.preftimezone);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.prefgeozone);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.preflanguage));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.preftheme));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.availability);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.givenname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.jobtitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.division));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.supervisor));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.address));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.locality));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.region));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.country));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.postalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.homenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.worknumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.faxnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.cellnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.pagernumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s@%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain_getname(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.domainid)));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.birthdate);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.hiredate);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.sin));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.isactive));
		if (sql_update(query)<0) return;
		db_log_activity(sid, "users", user.userid, "insert", "%s - %s added user %d", sid->dat->in_RemoteAddr, sid->dat->username, user.userid);
		prints(sid, "<CENTER>User added successfully</CENTER><BR />\r\n");
		memset(query, 0, sizeof(query));
		strcpy(query, "INSERT INTO gw_email_accounts (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountname, realname, organization, address, replyto, hosttype, pophost, popport, smtphost, smtpport, popusername, poppassword, smtpauth, lastcount, notify, remove, showdebug, signature) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '%d', '0', '%d', '0', '0', 'Local Mail', ", curdate, curdate, user.userid, user.domainid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s %s', '', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.givenname), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s@%s', '', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain_getname(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.domainid)));
		strncatf(query, sizeof(query)-strlen(query)-1, "'POP3', 'localhost', '110', 'localhost', '25', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s@%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain_getname(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.domainid)));
//		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, EncodeBase64string(sid, mailacct.poppassword)));
		strncatf(query, sizeof(query)-strlen(query)-1, "'', 'n', '0', '0', '2', 'n', '')");
		if (sql_update(query)<0) return;
		if (sql_queryf(&sqr, "SELECT mailaccountid FROM gw_email_accounts WHERE obj_uid = %d AND obj_did = %d", user.userid, user.domainid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			j=atoi(sql_getvalue(&sqr, 0, 0));
			if (sql_updatef("UPDATE gw_users SET prefmailcurrent = %d WHERE userid = %d AND domainid = %d", j, user.userid, user.domainid)<0) return;
		}
		sql_freeresult(&sqr);
//		wmfolder_makedefaults(sid, mailacct.mailaccountid);
//		db_log_activity(sid, "mailaccounts", mailacct.mailaccountid, "insert", "%s - %s added mail account %d", sid->dat->in_RemoteAddr, sid->dat->username, mailacct.mailaccountid);
		prints(sid, "<CENTER>E-Mail account added successfully</CENTER><BR />\r\n");
		if (dirmake("%s/%04d/files/users/%s", config->dir_var_domains, user.domainid, user.username)!=0) return;
		if (dirmake("%s/%04d/mailspool/%s", config->dir_var_domains, user.domainid, user.username)!=0) return;
		if (sql_updatef("INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('%s', '%s', '%d', '0', '%d', '1', '1', '/files/users/', '%s', 'dir', '%s', '%s', '0', '')", curdate, curdate, user.userid, user.domainid, user.username, curdate, curdate)<0) return;
		if (sql_queryf(&sqr, "SELECT folderid FROM gw_bookmarks_folders WHERE parentid = 0 AND obj_did = %d AND foldername = 'users'", user.domainid)<0) return;
		if (sql_numtuples(&sqr)==1) {
			j=atoi(sql_getvalue(&sqr, 0, 0));
			if (sql_updatef("INSERT INTO gw_bookmarks_folders (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('%s', '%s', '%d', '0', '%d', '1', '1', '%d', '%s')", curdate, curdate, user.userid, user.domainid, j, user.username)<0) return;
		}
		sql_freeresult(&sqr);
		prints(sid, "<CENTER>Home folders added successfully</CENTER><BR />\r\n");
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
		if (sql_queryf(&sqr, "SELECT username FROM gw_users where username = '%s' and userid <> %d AND domainid = %d", user.username, user.userid, user.domainid)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>User %s already exists</B></CENTER>\r\n", user.username);
			sql_freeresult(&sqr);
			adminuseredit(sid, &user);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(user.username)<1) {
			prints(sid, "<CENTER><B>Username is too short</B></CENTER>\r\n");
			adminuseredit(sid, &user);
			return;
		}
		if (newpass) {
			snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_users SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, user.obj_uid, user.obj_gid, user.obj_gperm, user.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "username = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username));
		strncatf(query, sizeof(query)-strlen(query)-1, "password = '%s', ", user.password);
		strncatf(query, sizeof(query)-strlen(query)-1, "groupid = '%d', ", user.groupid);
		strncatf(query, sizeof(query)-strlen(query)-1, "domainid = '%d', ", user.domainid);
		strncatf(query, sizeof(query)-strlen(query)-1, "enabled = '%d', ", user.enabled);
		strncatf(query, sizeof(query)-strlen(query)-1, "authdomainadmin = '%d', ", user.authdomainadmin);
		strncatf(query, sizeof(query)-strlen(query)-1, "authadmin = '%d', ", user.authadmin);
		strncatf(query, sizeof(query)-strlen(query)-1, "authbookmarks = '%d', ", user.authbookmarks);
		strncatf(query, sizeof(query)-strlen(query)-1, "authcalendar = '%d', ", user.authcalendar);
		strncatf(query, sizeof(query)-strlen(query)-1, "authcalls = '%d', ", user.authcalls);
		strncatf(query, sizeof(query)-strlen(query)-1, "authcontacts = '%d', ", user.authcontacts);
		strncatf(query, sizeof(query)-strlen(query)-1, "authemail = '%d', ", user.authemail);
		strncatf(query, sizeof(query)-strlen(query)-1, "authfiles = '%d', ", user.authfiles);
		strncatf(query, sizeof(query)-strlen(query)-1, "authfinance = '%d', ", user.authfinance);
		strncatf(query, sizeof(query)-strlen(query)-1, "authforums = '%d', ", user.authforums);
		strncatf(query, sizeof(query)-strlen(query)-1, "authmessages = '%d', ", user.authmessages);
		strncatf(query, sizeof(query)-strlen(query)-1, "authprofile = '%d', ", user.authprofile);
		strncatf(query, sizeof(query)-strlen(query)-1, "authprojects = '%d', ", user.authprojects);
		strncatf(query, sizeof(query)-strlen(query)-1, "authquery = '%d', ", user.authquery);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefdaystart = '%d', ", user.prefdaystart);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefdaylength = '%d', ", user.prefdaylength);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefmailcurrent = '%d', ", user.prefmailcurrent);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefmaildefault = '%d', ", user.prefmaildefault);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefmaxlist = '%d', ", user.prefmaxlist);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefmenustyle = '%d', ", user.prefmenustyle);
		strncatf(query, sizeof(query)-strlen(query)-1, "preftimezone = '%d', ", user.preftimezone);
		strncatf(query, sizeof(query)-strlen(query)-1, "prefgeozone = '%d', ", user.prefgeozone);
		strncatf(query, sizeof(query)-strlen(query)-1, "preflanguage = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.preflanguage));
		strncatf(query, sizeof(query)-strlen(query)-1, "preftheme = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.preftheme));
		strncatf(query, sizeof(query)-strlen(query)-1, "availability = '%s', ", user.availability);
		strncatf(query, sizeof(query)-strlen(query)-1, "surname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "givenname = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.givenname));
		strncatf(query, sizeof(query)-strlen(query)-1, "jobtitle = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.jobtitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "division = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.division));
		strncatf(query, sizeof(query)-strlen(query)-1, "supervisor = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.supervisor));
		strncatf(query, sizeof(query)-strlen(query)-1, "address = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.address));
		strncatf(query, sizeof(query)-strlen(query)-1, "locality = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.locality));
		strncatf(query, sizeof(query)-strlen(query)-1, "region = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.region));
		strncatf(query, sizeof(query)-strlen(query)-1, "country = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.country));
		strncatf(query, sizeof(query)-strlen(query)-1, "postalcode = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.postalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, "homenumber = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.homenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "worknumber = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.worknumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "faxnumber = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.faxnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "cellnumber = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.cellnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "pagernumber = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.pagernumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "email = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.email));
		strncatf(query, sizeof(query)-strlen(query)-1, "birthdate = '%s', ", user.birthdate);
		strncatf(query, sizeof(query)-strlen(query)-1, "hiredate = '%s', ", user.hiredate);
		strncatf(query, sizeof(query)-strlen(query)-1, "sin = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.sin));
		strncatf(query, sizeof(query)-strlen(query)-1, "isactive = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.isactive));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE userid = %d", user.userid);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>User %d modified successfully</CENTER><BR />\r\n", userid);
		db_log_activity(sid, "users", user.userid, "modify", "%s - %s modified user %d", sid->dat->in_RemoteAddr, sid->dat->username, user.userid);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/admin/userlist\");\r\n// -->\r\n</SCRIPT>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<NOSCRIPT>\r\n<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\r\n</NOSCRIPT>\r\n", sid->dat->in_ScriptName);
*/
	return;
}
