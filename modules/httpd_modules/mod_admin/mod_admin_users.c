/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "http_mod.h"
#include "mod_admin.h"

void adminuseredit(CONN *sid, REC_USER *user)
{
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
	REC_USER userrec;
	char maddr[250];
	char *ptemp;
	int userid;
	int tz1;
	int tz2;
	time_t t;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", userid);
				return;
			}
		}
		user=&userrec;
	}
	tz1=time_tzoffset(sid, time(NULL));
	tz2=time_tzoffset2(time(NULL), user->userid);
	if (user->userid<1) tz2=tz1;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	htscript_showpage(sid, 6);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (user->userid!=0) {
		prints(sid, "<B>User %d</B>\n", user->userid);
	} else {
		prints(sid, "<B>New User</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usersave NAME=useredit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", user->userid);
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SUMMARY</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>USER</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>NAME</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=4 HREF=javascript:showpage(4)>ADDRESS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page5tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=5 HREF=javascript:showpage(5)>PREFERENCES</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page6tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=6 HREF=javascript:showpage(6)>PERMISSIONS</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Name&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
	prints(sid, "%s%s", str2html(sid, user->givenname), strlen(user->givenname)?" ":"");
	prints(sid, "%s%s", str2html(sid, user->surname), strlen(user->surname)?" ":"");
	prints(sid, " (%s)", str2html(sid, user->username));
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;E-Mail Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
	if (strlen(user->email)==0) {
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (sid->dat->user_maildefault==0) {
		prints(sid, "<A HREF=\"mailto:%s\">%s</A>&nbsp;</TD></TR>\n", user->email, str2html(sid, user->email));
	} else {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "<A HREF=\"javascript:MsgTo('");
			prints(sid, "&quot;%s%s%s&quot;", str2html(sid, user->givenname), strlen(user->givenname)?" ":"", str2html(sid, user->surname));
			prints(sid, " <%s>')\">%s</A>&nbsp;</TD></TR>\n", user->email, str2html(sid, user->email));
		} else {
			prints(sid, "<A HREF=\"mailwrite?to=%s\">%s</A>&nbsp;</TD></TR>\n", user->email, str2html(sid, user->email));
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
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>Map Available</A>&nbsp;</TD></TR>\n", maddr);
	}
	if (tz1!=tz2) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Time&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><SPAN ID=contz></SPAN>&nbsp;</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Username&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username VALUE=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->username));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password VALUE=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->password));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Domain  &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=domainid style='width:255px'%s>\n", !(auth_priv(sid, "domainadmin")&A_ADMIN)||(user->userid>0)?" DISABLED":"");
	htselect_domain(sid, user->domainid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Group   &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=groupid style='width:255px'>\n");
	htselect_group(sid, A_ADMIN, user->groupid, user->domainid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Allow Login&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:255px'%s>\n", user->userid==1?" DISABLED":"");
	if ((user->userid==1)||(user->enabled)) {
		prints(sid, "<OPTION VALUE=0>No\n<OPTION VALUE=1 SELECTED>Yes\n");
	} else {
		prints(sid, "<OPTION VALUE=0 SELECTED>No\n<OPTION VALUE=1>Yes\n");
	}
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Surname&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->surname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Given Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname   value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->givenname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Job Title&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle     value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->jobtitle));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Division&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=division      value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->division));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Supervisor&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=supervisor  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->supervisor));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Date of Birth&nbsp;</B></TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=birthdate2>\n");
	htselect_month(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate1>\n");
	htselect_day(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate3>\n");
	htselect_year(sid, 1900, user->birthdate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Hire Date&nbsp;</B></TD><TD ALIGN=RIGHT>\n");
	prints(sid, "<SELECT NAME=hiredate2>\n");
	htselect_month(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate1>\n");
	htselect_day(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate3>\n");
	htselect_year(sid, 1900, user->hiredate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;S.I.N.&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sin         value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->sin));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Active&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=isactive    value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->isactive));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->homenumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->worknumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;FAX&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber          value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->faxnumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Cell Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=cellnumber  value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->cellnumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Pager&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pagernumber      value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->pagernumber));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;E-Mail&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email           value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->email));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->address));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;City&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=locality          value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->locality));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Province&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=region        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->region));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Country&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=country        value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->country));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=postalcode value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->postalcode));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page5 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if (module_exists("mod_calendar")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Start&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaystart style='width:255px'>\n");
		htselect_hour(sid, user->prefdaystart);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Calendar Length&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaylength style='width:255px'>\n");
		htselect_number(sid, user->prefdaylength, 0, 24, 1);
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Max Results/Page&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefmaxlist style='width:255px'>\n");
	htselect_number(sid, user->prefmaxlist, 5, 200, 5);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Time Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=preftimezone style='width:255px'>\n");
	htselect_timezone(sid, user->preftimezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefgeozone style='width:255px'>\n");
	htselect_zone(sid, user->prefgeozone, user->domainid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Language&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=preflanguage    value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->preflanguage));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Theme&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=preftheme   value=\"%s\" SIZE=45 style='width:255px'></TD></TR>\n", str2html(sid, user->preftheme));
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page6 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
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
	prints(sid, "</TR>\n");
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
	prints(sid, "</TR>\n");
	if (module_exists("mod_bookmarks")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Bookmarks&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_r VALUE='1' %s></TD>", (user->authbookmarks&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_m VALUE='1' %s></TD>", (user->authbookmarks&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_i VALUE='1' %s></TD>", (user->authbookmarks&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_d VALUE='1' %s></TD>", (user->authbookmarks&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authbookmarks_a VALUE='1' %s></TD>", (user->authbookmarks&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_calendar")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Calendar&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_r VALUE='1' %s></TD>", (user->authcalendar&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_m VALUE='1' %s></TD>", (user->authcalendar&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_i VALUE='1' %s></TD>", (user->authcalendar&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_d VALUE='1' %s></TD>", (user->authcalendar&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalendar_a VALUE='1' %s></TD>", (user->authcalendar&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_calls")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Calls&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_r VALUE='1' %s></TD>", (user->authcalls&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_m VALUE='1' %s></TD>", (user->authcalls&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_i VALUE='1' %s></TD>", (user->authcalls&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_d VALUE='1' %s></TD>", (user->authcalls&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcalls_a VALUE='1' %s></TD>", (user->authcalls&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_contacts")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Contacts&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_r VALUE='1' %s></TD>", (user->authcontacts&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_m VALUE='1' %s></TD>", (user->authcontacts&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_i VALUE='1' %s></TD>", (user->authcontacts&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_d VALUE='1' %s></TD>", (user->authcontacts&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authcontacts_a VALUE='1' %s></TD>", (user->authcontacts&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_mail")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;E-Mail&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authwebmail_r VALUE='1' %s></TD>", (user->authwebmail&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authwebmail_m VALUE='1' %s></TD>", (user->authwebmail&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authwebmail_i VALUE='1' %s></TD>", (user->authwebmail&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authwebmail_d VALUE='1' %s></TD>", (user->authwebmail&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_files")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Files&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_r VALUE='1' %s></TD>", (user->authfiles&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_m VALUE='1' %s></TD>", (user->authfiles&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_i VALUE='1' %s></TD>", (user->authfiles&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_d VALUE='1' %s></TD>", (user->authfiles&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authfiles_a VALUE='1' %s></TD>", (user->authfiles&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_forums")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Forums&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authforums_r VALUE='1' %s></TD>", (user->authforums&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authforums_i VALUE='1' %s></TD>", (user->authforums&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authforums_a VALUE='1' %s></TD>", (user->authforums&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_messages")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Messages&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authmessages_r VALUE='1' %s></TD>", (user->authmessages&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authmessages_i VALUE='1' %s></TD>", (user->authmessages&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authmessages_d VALUE='1' %s></TD>", (user->authmessages&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_orders")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Orders&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authorders_r VALUE='1' %s></TD>", (user->authorders&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authorders_m VALUE='1' %s></TD>", (user->authorders&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authorders_i VALUE='1' %s></TD>", (user->authorders&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authorders_d VALUE='1' %s></TD>", (user->authorders&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authorders_a VALUE='1' %s></TD>", (user->authorders&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_profile")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Profile&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprofile_r VALUE='1' %s></TD>", (user->authprofile&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprofile_m VALUE='1' %s></TD>", (user->authprofile&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_projects")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;Projects&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_r VALUE='1' %s></TD>", (user->authprojects&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_m VALUE='1' %s></TD>", (user->authprojects&A_MODIFY)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_i VALUE='1' %s></TD>", (user->authprojects&A_INSERT)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_d VALUE='1' %s></TD>", (user->authprojects&A_DELETE)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authprojects_a VALUE='1' %s></TD>", (user->authprojects&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	if (module_exists("mod_searches")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP WIDTH=100%%><B>&nbsp;SQL Queries&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authquery_r VALUE='1' %s></TD>", (user->authquery&A_READ)?"checked":"");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
		prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox CLASS='nomargin' NAME=authquery_a VALUE='1' %s></TD>", (user->authquery&A_ADMIN)?"checked":"");
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR><TD NOWRAP COLSPAN=2>");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
		prints(sid, "<TR><TH NOWRAP STYLE='border-style:solid'>Notes");
		prints(sid, " [<A HREF=%s/notes/editnew?table=users&index=%d>new</A>]", sid->dat->in_ScriptName, user->userid);
		prints(sid, "</FONT></TH></TR>\n");
		mod_notes_sublist(sid, "users", user->userid, 1);
		prints(sid, "</TABLE>\n");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(user->userid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if (user->userid!=0) {
		prints(sid, "[<A HREF=%s/admin/usertimeedit?userid=%d>Edit Availability</A>]\n", sid->dat->in_ScriptName, user->userid);
	}
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
//	prints(sid, "document.useredit.username.focus();\n");
	prints(sid, "showpage(1);\n");
	if (tz1!=tz2) {
		t=time(NULL);
		t+=time_tzoffset(sid, t);
		t+=(tz2-tz1);
		prints(sid, "var t=%d;\n", t);
		prints(sid, "var d=new Date();\n");
		prints(sid, "d.setTime(t*1000);\n");
		prints(sid, "t+=d.getTimezoneOffset()*60;\n");
		prints(sid, "function padout(number) { return (number<10)?'0'+number:number; }\n");
		prints(sid, "function setDuration() {\n");
		prints(sid, "	d.setTime(t*1000);\n");
		prints(sid, "	var h=d.getHours();\n");
		prints(sid, "	var ap='AM';\n");
		prints(sid, "	if (h>11) ap='PM';\n");
		prints(sid, "	if (h>12) h-=12;\n");
		prints(sid, "	if (h<1) h=12;\n");
		prints(sid, "	document.getElementById('contz').innerHTML=h+':'+padout(d.getMinutes())+':'+padout(d.getSeconds())+' '+ap;\n");
		prints(sid, "	t++;\n");
		prints(sid, "	setTimeout(\"setDuration()\", 1000);\n");
		prints(sid, "}\n");
		prints(sid, "setDuration();\n");
	}
	prints(sid, "// -->\n</SCRIPT>\n");
	return;
}

void adminuserlist(CONN *sid)
{
	int i, j;
	int lastdomain;
	int thisdomain;
	int sqr1;
	int sqr2;
	int sqr3;
	int sqr4;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		sqr1=sql_queryf("SELECT userid, groupid, domainid, username, surname, givenname, prefgeozone FROM gw_users ORDER BY domainid, username ASC");
	} else {
		sqr1=sql_queryf("SELECT userid, groupid, domainid, username, surname, givenname, prefgeozone FROM gw_users WHERE domainid = %d ORDER BY domainid, username ASC", sid->dat->user_did);
	}
	if (sqr1<0) return;
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		sqr2=sql_queryf("SELECT groupid, groupname FROM gw_groups ORDER BY groupid ASC");
	} else {
		sqr2=sql_queryf("SELECT groupid, groupname FROM gw_groups WHERE obj_did = %d ORDER BY groupid ASC", sid->dat->user_did);
	}
	if (sqr2<0) {
		sql_freeresult(sqr1);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		sqr3=sql_queryf("SELECT zoneid, zonename FROM gw_zones ORDER BY zoneid ASC");
	} else {
		sqr3=sql_queryf("SELECT zoneid, zonename FROM gw_zones WHERE obj_did = %d ORDER BY zoneid ASC", sid->dat->user_did);
	}
	if (sqr3<0) {
		sql_freeresult(sqr1);
		sql_freeresult(sqr2);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		sqr4=sql_queryf("SELECT domainid, domainname FROM gw_domains ORDER BY domainid ASC");
	} else {
		sqr4=sql_queryf("SELECT domainid, domainname FROM gw_domains WHERE domainid = %d", sid->dat->user_did);
	}
	if (sqr4<0) {
		sql_freeresult(sqr1);
		sql_freeresult(sqr2);
		sql_freeresult(sqr3);
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;User Name&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;Real Name&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;Group&nbsp;</TH>");
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100 STYLE='border-style:solid'>&nbsp;Zone&nbsp;</TH>");
	prints(sid, "</TR>\n");
	lastdomain=-1;
	if (sql_numtuples(sqr4)<2) {
		lastdomain=atoi(sql_getvalue(sqr1, 0, 2));
	}
	for (i=0;i<sql_numtuples(sqr1);i++) {
		thisdomain=atoi(sql_getvalue(sqr1, i, 2));
		if (lastdomain!=thisdomain) {
			lastdomain=thisdomain;
			prints(sid, "<TR CLASS=\"FIELDNAME\"><TD COLSPAN=4 NOWRAP style='border-style:solid'>");
			for (j=0;j<sql_numtuples(sqr4);j++) {
				if (atoi(sql_getvalue(sqr4, j, 0))==atoi(sql_getvalue(sqr1, i, 2))) {
					prints(sid, "<B>%s</B></TD>", str2html(sid, sql_getvalue(sqr4, j, 1)));
					break;
				}
			}
			if (j==sql_numtuples(sqr4)) {
				prints(sid, "&nbsp;</TD>");
			}
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 3)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s", str2html(sid, sql_getvalue(sqr1, i, 4)));
		if (strlen(sql_getvalue(sqr1, i, 4))&&strlen(sql_getvalue(sqr1, i, 5))) prints(sid, ", ");
		prints(sid, "%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 5)));
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvalue(sqr2, j, 0))==atoi(sql_getvalue(sqr1, i, 1))) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr2, j, 1)));
				break;
			}
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvalue(sqr3, j, 0))==atoi(sql_getvalue(sqr1, i, 6))) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", str2html(sid, sql_getvalue(sqr3, j, 1)));
				break;
			}
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<A HREF=%s/admin/usereditnew>New User</A>", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>");
	sql_freeresult(sqr1);
	sql_freeresult(sqr2);
	sql_freeresult(sqr3);
	sql_freeresult(sqr4);
	return;
}

void adminusersave(CONN *sid)
{
	REC_USER user;
	char query[4096];
	char curdate[40];
	char opassword[50];
	char *ptemp;
	time_t t;
	int userid;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	if (dbread_user(sid, 2, userid, &user)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", user.password);
	if ((ptemp=getpostenv(sid, "USERNAME"))!=NULL) snprintf(user.username, sizeof(user.username)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) snprintf(user.password, sizeof(user.password)-1, "%s", ptemp);
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
	if (module_exists("mod_files")) {
		user.authfiles=0;
		if ((ptemp=getpostenv(sid, "AUTHFILES_R"))!=NULL) user.authfiles+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHFILES_M"))!=NULL) user.authfiles+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHFILES_I"))!=NULL) user.authfiles+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHFILES_D"))!=NULL) user.authfiles+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHFILES_A"))!=NULL) user.authfiles+=A_ADMIN;
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
	if (module_exists("mod_orders")) {
		user.authorders=0;
		if ((ptemp=getpostenv(sid, "AUTHORDERS_R"))!=NULL) user.authorders+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHORDERS_M"))!=NULL) user.authorders+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHORDERS_I"))!=NULL) user.authorders+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHORDERS_D"))!=NULL) user.authorders+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHORDERS_A"))!=NULL) user.authorders+=A_ADMIN;
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
	if (module_exists("mod_mail")) {
		user.authwebmail=0;
		if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_R"))!=NULL) user.authwebmail+=A_READ;
		if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_M"))!=NULL) user.authwebmail+=A_MODIFY;
		if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_I"))!=NULL) user.authwebmail+=A_INSERT;
		if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_D"))!=NULL) user.authwebmail+=A_DELETE;
		if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_A"))!=NULL) user.authwebmail+=A_ADMIN;
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
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_users WHERE userid = %d", user.userid)<0) return;
		prints(sid, "<CENTER>User %d deleted successfully</CENTER><BR>\n", userid);
		db_log_activity(sid, "users", user.userid, "delete", "%s - %s deleted user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	} else if (user.userid==0) {
		if ((sqr=sql_queryf("SELECT username FROM gw_users where username = '%s' AND domainid = %d", user.username, user.domainid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>User %s already exists</B></CENTER>\n", user.username);
			sql_freeresult(sqr);
			adminuseredit(sid, &user);
			return;
		}
		sql_freeresult(sqr);
		if (strlen(user.username)<1) {
			prints(sid, "<CENTER><B>Username is too short</B></CENTER>\n");
			adminuseredit(sid, &user);
			return;
		}
		snprintf(user.password, sizeof(user.password)-1, "%s", auth_setpass(sid, user.password));
		if ((sqr=sql_query("SELECT max(userid) FROM gw_users"))<0) return;
		user.userid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (user.userid<1) user.userid=1;
		memset(query, 0, sizeof(query));
		strcpy(query, "INSERT INTO gw_users (userid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, loginip, logintime, logintoken, username, password, groupid, domainid, enabled, authdomainadmin, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authfiles, authforums, authmessages, authorders, authprofile, authprojects, authquery, authwebmail, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone, prefgeozone, preflanguage, preftheme, availability, surname, givenname, jobtitle, division, supervisor, address, locality, region, country, postalcode, homenumber, worknumber, faxnumber, cellnumber, pagernumber, email, birthdate, hiredate, sin, isactive) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", user.userid, curdate, curdate, user.obj_uid, user.obj_gid, user.obj_gperm, user.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'0.0.0.0', '1900-01-01 00:00:00', '', ");
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
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authfiles);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authforums);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authmessages);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authorders);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authprofile);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authprojects);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authquery);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.authwebmail);
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
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.email));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.birthdate);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.hiredate);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.sin));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.isactive));
		if (sql_update(query)<0) return;
		db_log_activity(sid, "users", user.userid, "insert", "%s - %s added user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
		prints(sid, "<CENTER>User added successfully</CENTER><BR>\n");
		memset(query, 0, sizeof(query));
		strcpy(query, "INSERT INTO gw_mailaccounts (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountname, realname, organization, address, replyto, hosttype, pophost, popport, smtphost, smtpport, popusername, poppassword, smtpauth, lastcount, notify, remove, showdebug, signature) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '%d', '0', '%d', '0', '0', ", curdate, curdate, user.userid, user.domainid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'Local Mail', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s %s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.givenname), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s@%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain_getname(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.domainid)));
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'POP3', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'localhost', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'110', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'localhost', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'25', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s@%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.username), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, domain_getname(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, user.domainid)));
//		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, EncodeBase64string(sid, mailacct.poppassword)));
		strncatf(query, sizeof(query)-strlen(query)-1, "'', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'n', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'0', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'0', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'2004-01-01', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'n', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'')");
		if (sql_update(query)<0) return;
//		wmfolder_makedefaults(sid, mailacct.mailaccountid);
//		db_log_activity(sid, "mailaccounts", mailacct.mailaccountid, "insert", "%s - %s added mail account %d", sid->dat->in_RemoteAddr, sid->dat->user_username, mailacct.mailaccountid);
		prints(sid, "<CENTER>E-Mail account added successfully</CENTER><BR>\n");

		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_queryf("SELECT username FROM gw_users where username = '%s' and userid <> %d AND domainid = %d", user.username, user.userid, user.domainid))<0) return;
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>User %s already exists</B></CENTER>\n", user.username);
			sql_freeresult(sqr);
			adminuseredit(sid, &user);
			return;
		}
		sql_freeresult(sqr);
		if (strlen(user.username)<1) {
			prints(sid, "<CENTER><B>Username is too short</B></CENTER>\n");
			adminuseredit(sid, &user);
			return;
		}
		if (strcmp(opassword, user.password)!=0) {
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
		strncatf(query, sizeof(query)-strlen(query)-1, "authfiles = '%d', ", user.authfiles);
		strncatf(query, sizeof(query)-strlen(query)-1, "authforums = '%d', ", user.authforums);
		strncatf(query, sizeof(query)-strlen(query)-1, "authmessages = '%d', ", user.authmessages);
		strncatf(query, sizeof(query)-strlen(query)-1, "authorders = '%d', ", user.authorders);
		strncatf(query, sizeof(query)-strlen(query)-1, "authprofile = '%d', ", user.authprofile);
		strncatf(query, sizeof(query)-strlen(query)-1, "authprojects = '%d', ", user.authprojects);
		strncatf(query, sizeof(query)-strlen(query)-1, "authquery = '%d', ", user.authquery);
		strncatf(query, sizeof(query)-strlen(query)-1, "authwebmail = '%d', ", user.authwebmail);
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
		prints(sid, "<CENTER>User %d modified successfully</CENTER><BR>\n", userid);
		db_log_activity(sid, "users", user.userid, "modify", "%s - %s modified user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	return;
}

void adminusertimeedit(CONN *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char *ptemp;
	char gavailability[170];
	char uavailability[170];
	REC_USER user;
	int userid;
	int i;
	int j;
	int sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	if (dbread_user(sid, 2, userid, &user)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", userid);
		return;
	}
	if ((sqr=sql_queryf("SELECT availability FROM gw_users WHERE userid = %d", user.userid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for %s</CENTER>\n", user.userid);
		sql_freeresult(sqr);
		return;
	}
	memset(uavailability, 0, sizeof(uavailability));
	strncpy(uavailability, sql_getvalue(sqr, 0, 0), sizeof(uavailability)-1);
	sql_freeresult(sqr);
	if (strlen(uavailability)!=168) {
		for (i=0;i<168;i++) {
			uavailability[i]='0';
		}
	}
	if ((sqr=sql_queryf("SELECT availability FROM gw_groups WHERE groupid = %d", user.groupid))<0) return;
	if (sql_numtuples(sqr)!=1) {
		prints(sid, "<CENTER>No matching record found for group %d</CENTER>\n", user.groupid);
		sql_freeresult(sqr);
		return;
	}
	memset(gavailability, 0, sizeof(gavailability));
	strncpy(gavailability, sql_getvalue(sqr, 0, 0), sizeof(gavailability)-1);
	sql_freeresult(sqr);
	if (strlen(gavailability)!=168) {
		for (i=0;i<168;i++) {
			gavailability[i]='0';
		}
	}
	for (i=0;i<168;i++) {
		if (gavailability[i]=='0') {
			uavailability[i]='X';
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n");
	prints(sid, "<!--\n");
	prints(sid, "function toggle(b,n)\n");
	prints(sid, "{\n");
	prints(sid, "	if (b == \"t\" ) {\n");
	prints(sid, "		if (document.availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (document.availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				document.availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (document.availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			document.availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (document.availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				document.availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usertimesave NAME=availability>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", user.userid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR><TH COLSPAN=25 STYLE='border-style:solid'>Availability for <A HREF=%s/admin/useredit?userid=%d>%s</A></TH></TR>\n", sid->dat->in_ScriptName, userid, user.username);
	prints(sid, "<TR CLASS=\"FIELDNAME\">\n");
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2 STYLE='border-style:solid'>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12 STYLE='border-style:solid'><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR CLASS=\"FIELDNAME\">\n");
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER STYLE='border-style:solid'><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP CLASS=\"FIELDNAME\" STYLE='border-style:solid'><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", i, dow[i]);
		for (j=0;j<24;j++) {
			if (uavailability[i*24+j]=='X') {
				prints(sid, "<TD STYLE='border-style:solid'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
			} else {
				prints(sid, "<TD STYLE='border-style:solid'><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, uavailability[i*24+j]=='1'?"checked":"");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "<BR>[<A HREF=%s/admin/grouptimeedit?groupid=%d>Group Availability</A>]\n", sid->dat->in_ScriptName, user.groupid);
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void adminusertimesave(CONN *sid)
{
	char availability[170];
	char curdate[40];
	char qvar[10];
	time_t t;
	char *ptemp;
	char *pdest;
	int userid;
	int i;
	int j;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	memset(availability, 0, sizeof(availability));
	pdest=availability;
	for (i=0;i<7;i++) {
		for (j=0;j<24;j++) {
			memset(qvar, 0, sizeof(qvar));
			snprintf(qvar, sizeof(qvar)-1, "D%dT%d", i, j);
			if ((ptemp=getpostenv(sid, qvar))==NULL) {
				*pdest++='0';
				continue;
			}
			if (strcasecmp(ptemp, "false")==0) {
				*pdest++='1';
			} else {
				*pdest++='1';
			}
		}
	}
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (sql_updatef("UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, userid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	db_log_activity(sid, "users", userid, "modify", "%s - %s modified availability for user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, userid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/useredit?userid=%d\");\n", sid->dat->in_ScriptName, userid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/useredit?userid=%d\">\n", sid->dat->in_ScriptName, userid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}
