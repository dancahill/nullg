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
#include "mod_substub.h"
#include "mod_admin.h"

void adminuseredit(CONNECTION *sid, REC_USER *user)
{
	MOD_NOTES_SUBLIST mod_notes_sublist;
	REC_USER userrec;
	int userid;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (user==NULL) {
		if (strcmp(sid->dat->in_RequestURI, "/admin/usereditnew")==0) {
			userid=0;
			db_read(sid, 2, DB_USERS, 0, &userrec);
		} else {
			if (getgetenv(sid, "USERID")==NULL) return;
			userid=atoi(getgetenv(sid, "USERID"));
			if (db_read(sid, 2, DB_USERS, userid, &userrec)!=0) {
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", userid);
				return;
			}
		}
		user=&userrec;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usersave NAME=useredit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", user->userid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>ACCOUNT INFORMATION - ", config->colour_th, config->colour_thtext);
	if (user->userid!=0) {
		prints(sid, "User %d</FONT></TH></TR>\n", user->userid);
	} else {
		prints(sid, "New User</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=left VALIGN=top>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Username&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->username));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Password&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password VALUE=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->password));
	prints(sid, "</TABLE>\n</TD><TD ALIGN=center VALIGN=top><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Group&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=groupid style='width:182px'>\n", config->colour_editform);
	htselect_group(sid, user->groupid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Allow Login&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:182px'%s>\n", config->colour_editform, user->userid==1?" DISABLED":"");
	if ((user->userid==1)||(user->enabled)) {
		prints(sid, "<OPTION VALUE=0>No\n<OPTION VALUE=1 SELECTED>Yes\n");
	} else {
		prints(sid, "<OPTION VALUE=0 SELECTED>No\n<OPTION VALUE=1>Yes\n");
	}
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>PERMISSIONS</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=left VALIGN=top>\n<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%>&nbsp;</TD><TD ALIGN=CENTER><B>Read&nbsp;</B></TD><TD ALIGN=CENTER><B>Modify&nbsp;</B></TD><TD ALIGN=CENTER><B>Insert&nbsp;</B></TD><TD ALIGN=CENTER><B>Delete&nbsp;</B></TD><TD ALIGN=CENTER><B>Admin</B></TD></TR>", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Administration&nbsp;</B></TD>", config->colour_editform);
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
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Bookmarks&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_r VALUE='1' %s></TD>", (user->authbookmarks&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_m VALUE='1' %s></TD>", (user->authbookmarks&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_i VALUE='1' %s></TD>", (user->authbookmarks&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_d VALUE='1' %s></TD>", (user->authbookmarks&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authbookmarks_a VALUE='1' %s></TD>", (user->authbookmarks&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Calendar&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_r VALUE='1' %s></TD>", (user->authcalendar&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_m VALUE='1' %s></TD>", (user->authcalendar&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_i VALUE='1' %s></TD>", (user->authcalendar&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_d VALUE='1' %s></TD>", (user->authcalendar&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalendar_a VALUE='1' %s></TD>", (user->authcalendar&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Calls&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_r VALUE='1' %s></TD>", (user->authcalls&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_m VALUE='1' %s></TD>", (user->authcalls&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_i VALUE='1' %s></TD>", (user->authcalls&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_d VALUE='1' %s></TD>", (user->authcalls&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcalls_a VALUE='1' %s></TD>", (user->authcalls&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Contacts&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_r VALUE='1' %s></TD>", (user->authcontacts&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_m VALUE='1' %s></TD>", (user->authcontacts&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_i VALUE='1' %s></TD>", (user->authcontacts&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_d VALUE='1' %s></TD>", (user->authcontacts&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authcontacts_a VALUE='1' %s></TD>", (user->authcontacts&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;E-Mail&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_r VALUE='1' %s></TD>", (user->authwebmail&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_m VALUE='1' %s></TD>", (user->authwebmail&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_i VALUE='1' %s></TD>", (user->authwebmail&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authwebmail_d VALUE='1' %s></TD>", (user->authwebmail&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "</TR>\n");
	prints(sid, "</TABLE>\n</TD><TD ALIGN=center VALIGN=top><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%>&nbsp;</TD><TD ALIGN=CENTER><B>Read&nbsp;</B></TD><TD ALIGN=CENTER><B>Modify&nbsp;</B></TD><TD ALIGN=CENTER><B>Insert&nbsp;</B></TD><TD ALIGN=CENTER><B>Delete&nbsp;</B></TD><TD ALIGN=CENTER><B>Admin</B></TD></TR>", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Files&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_r VALUE='1' %s></TD>", (user->authfiles&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_m VALUE='1' %s></TD>", (user->authfiles&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_i VALUE='1' %s></TD>", (user->authfiles&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_d VALUE='1' %s></TD>", (user->authfiles&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authfiles_a VALUE='1' %s></TD>", (user->authfiles&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Forums&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authforums_r VALUE='1' %s></TD>", (user->authforums&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authforums_i VALUE='1' %s></TD>", (user->authforums&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authforums_a VALUE='1' %s></TD>", (user->authforums&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Messages&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authmessages_r VALUE='1' %s></TD>", (user->authmessages&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authmessages_i VALUE='1' %s></TD>", (user->authmessages&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authmessages_d VALUE='1' %s></TD>", (user->authmessages&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Orders&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_r VALUE='1' %s></TD>", (user->authorders&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_m VALUE='1' %s></TD>", (user->authorders&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_i VALUE='1' %s></TD>", (user->authorders&A_INSERT)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_d VALUE='1' %s></TD>", (user->authorders&A_DELETE)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authorders_a VALUE='1' %s></TD>", (user->authorders&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;Profile&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authprofile_r VALUE='1' %s></TD>", (user->authprofile&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authprofile_m VALUE='1' %s></TD>", (user->authprofile&A_MODIFY)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP WIDTH=100%%><B>&nbsp;SQL Queries&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authquery_r VALUE='1' %s></TD>", (user->authquery&A_READ)?"checked":"");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER>&nbsp;</TD>");
	prints(sid, "<TD ALIGN=CENTER><INPUT TYPE=checkbox NAME=authquery_a VALUE='1' %s></TD>", (user->authquery&A_ADMIN)?"checked":"");
	prints(sid, "</TR>\n");
	prints(sid, "</TABLE></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>PREFERENCE INFORMATION</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Calendar Start&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaystart style='width:182px'>\n", config->colour_editform);
	htselect_hour(sid, user->prefdaystart);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Calendar Length&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefdaylength style='width:182px'>\n", config->colour_editform);
	htselect_number(sid, user->prefdaylength, 0, 24);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Max Results/Page&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefmaxlist style='width:182px'>\n", config->colour_editform);
	htselect_number(sid, user->prefmaxlist, 5, 50);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Time Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=preftimezone style='width:182px'>\n", config->colour_editform);
	htselect_timezone(sid, user->preftimezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=prefgeozone style='width:182px'>\n", config->colour_editform);
	htselect_zone(sid, user->prefgeozone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>PERSONAL INFORMATION</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", config->colour_editform);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Surname&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->surname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Given Name&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname   value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->givenname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Job Title&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->jobtitle));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Division&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=division      value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->division));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Supervisor&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=supervisor  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->supervisor));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Address&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=address        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->address));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;City&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=locality          value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->locality));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Province&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=region        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->region));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Country&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=country        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->country));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=postalcode value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->postalcode));
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->homenumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->worknumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;FAX&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber          value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->faxnumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Cell Phone&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=cellnumber  value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->cellnumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Pager&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=pagernumber      value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->pagernumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;E-Mail&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email           value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->email));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Date of Birth&nbsp;</B></TD><TD ALIGN=RIGHT>\n", config->colour_editform);
	prints(sid, "<SELECT NAME=birthdate2>\n");
	htselect_month(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate1>\n");
	htselect_day(sid, user->birthdate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=birthdate3>\n");
	htselect_year(sid, 1900, user->birthdate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Hire Date&nbsp;</B></TD><TD ALIGN=RIGHT>\n", config->colour_editform);
	prints(sid, "<SELECT NAME=hiredate2>\n");
	htselect_month(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate1>\n");
	htselect_day(sid, user->hiredate);
	prints(sid, "</SELECT>");
	prints(sid, "<SELECT NAME=hiredate3>\n");
	htselect_year(sid, 1900, user->hiredate);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;S.I.N.&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=sin         value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->sin));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Active&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=isactive    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", config->colour_editform, str2html(sid, user->isactive));
	prints(sid, "</TABLE>\n</TD></TR>\n");
	if ((mod_notes_sublist=module_call(sid, "mod_notes_sublist"))!=NULL) {
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>Notes", config->colour_th, config->colour_thtext);
		prints(sid, " [<A HREF=%s/notes/editnew?table=users&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, user->userid, config->colour_thlink);
		prints(sid, "</FONT></TH></TR>\n");
		mod_notes_sublist(sid, "users", user->userid, 2);
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_ADMIN)&A_ADMIN)&&(user->userid>1)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>\n", config->colour_editform);
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if (user->userid!=0) {
		prints(sid, "[<A HREF=%s/admin/usertimeedit?userid=%d>Edit Availability</A>]\n", sid->dat->in_ScriptName, user->userid);
	}
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.useredit.username.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void adminuserlist(CONNECTION *sid)
{
	int i, j;
	int sqr1;
	int sqr2;
	int sqr3;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr1=sql_query(sid, "SELECT userid, username, surname, givenname, groupid, prefgeozone FROM gw_users ORDER BY userid ASC"))<0) return;
	if ((sqr2=sql_query(sid, "SELECT groupid, groupname FROM gw_groups ORDER BY groupid ASC"))<0) {
		sql_freeresult(sqr1);
		return;
	}
	if ((sqr3=sql_query(sid, "SELECT zoneid, zonename FROM gw_zones ORDER BY zoneid ASC"))<0) {
		sql_freeresult(sqr1);
		sql_freeresult(sqr2);
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>", config->colour_th);
	prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;User Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;Real Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;Group&nbsp;</FONT></TH>", config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
	if (sql_numtuples(sqr3)>0) {
		prints(sid, "<TH ALIGN=LEFT NOWRAP WIDTH=100><FONT COLOR=%s>&nbsp;Zone&nbsp;</FONT></TH>", config->colour_thtext);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<sql_numtuples(sqr1);i++) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/admin/useredit?userid=%d'\">", config->colour_fieldval, sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/admin/useredit?userid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 1)));
		prints(sid, "<TD NOWRAP>%s", str2html(sid, sql_getvalue(sqr1, i, 2)));
		if (strlen(sql_getvalue(sqr1, i, 2))&&strlen(sql_getvalue(sqr1, i, 3))) prints(sid, ", ");
		prints(sid, "%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 3)));
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvalue(sqr2, j, 0))==atoi(sql_getvalue(sqr1, i, 4))) {
				prints(sid, "<TD NOWRAP>%s</TD>", str2html(sid, sql_getvalue(sqr2, j, 1)));
				break;
			}
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<TD NOWRAP>&nbsp;</TD>");
		}
		if (sql_numtuples(sqr3)>0) {
			for (j=0;j<sql_numtuples(sqr3);j++) {
				if (atoi(sql_getvalue(sqr3, j, 0))==atoi(sql_getvalue(sqr1, i, 5))) {
					prints(sid, "<TD NOWRAP>%s</TD>", str2html(sid, sql_getvalue(sqr3, j, 1)));
					break;
				}
			}
			if (j==sql_numtuples(sqr3)) {
				prints(sid, "<TD NOWRAP>&nbsp;</TD>");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	sql_freeresult(sqr1);
	sql_freeresult(sqr2);
	sql_freeresult(sqr3);
	prints(sid, "<A HREF=%s/admin/usereditnew>New User</A>\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\n");
	return;
}

void adminusersave(CONNECTION *sid)
{
	REC_USER user;
	char query[4096];
	char curdate[40];
	char opassword[50];
	char *ptemp;
	time_t t;
	int userid;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "USERID"))==NULL) return;
	userid=atoi(ptemp);
	if (db_read(sid, 2, DB_USERS, userid, &user)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", user.password);
	if ((ptemp=getpostenv(sid, "USERNAME"))!=NULL) snprintf(user.username, sizeof(user.username)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) snprintf(user.password, sizeof(user.password)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "GROUPID"))!=NULL) user.groupid=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "ENABLED"))!=NULL) user.enabled=atoi(ptemp);
	user.authadmin=0;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_R"))!=NULL) user.authadmin+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_M"))!=NULL) user.authadmin+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_I"))!=NULL) user.authadmin+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_D"))!=NULL) user.authadmin+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHADMIN_A"))!=NULL) user.authadmin+=A_ADMIN;
	user.authbookmarks=0;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_R"))!=NULL) user.authbookmarks+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_M"))!=NULL) user.authbookmarks+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_I"))!=NULL) user.authbookmarks+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_D"))!=NULL) user.authbookmarks+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHBOOKMARKS_A"))!=NULL) user.authbookmarks+=A_ADMIN;
	user.authcalendar=0;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_R"))!=NULL) user.authcalendar+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_M"))!=NULL) user.authcalendar+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_I"))!=NULL) user.authcalendar+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_D"))!=NULL) user.authcalendar+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHCALENDAR_A"))!=NULL) user.authcalendar+=A_ADMIN;
	user.authcalls=0;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_R"))!=NULL) user.authcalls+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_M"))!=NULL) user.authcalls+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_I"))!=NULL) user.authcalls+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_D"))!=NULL) user.authcalls+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHCALLS_A"))!=NULL) user.authcalls+=A_ADMIN;
	user.authcontacts=0;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_R"))!=NULL) user.authcontacts+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_M"))!=NULL) user.authcontacts+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_I"))!=NULL) user.authcontacts+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_D"))!=NULL) user.authcontacts+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHCONTACTS_A"))!=NULL) user.authcontacts+=A_ADMIN;
	user.authfiles=0;
	if ((ptemp=getpostenv(sid, "AUTHFILES_R"))!=NULL) user.authfiles+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHFILES_M"))!=NULL) user.authfiles+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHFILES_I"))!=NULL) user.authfiles+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHFILES_D"))!=NULL) user.authfiles+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHFILES_A"))!=NULL) user.authfiles+=A_ADMIN;
	user.authforums=0;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_R"))!=NULL) user.authforums+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_M"))!=NULL) user.authforums+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_I"))!=NULL) user.authforums+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_D"))!=NULL) user.authforums+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHFORUMS_A"))!=NULL) user.authforums+=A_ADMIN;
	user.authmessages=0;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_R"))!=NULL) user.authmessages+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_M"))!=NULL) user.authmessages+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_I"))!=NULL) user.authmessages+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_D"))!=NULL) user.authmessages+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHMESSAGES_A"))!=NULL) user.authmessages+=A_ADMIN;
	user.authorders=0;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_R"))!=NULL) user.authorders+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_M"))!=NULL) user.authorders+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_I"))!=NULL) user.authorders+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_D"))!=NULL) user.authorders+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHORDERS_A"))!=NULL) user.authorders+=A_ADMIN;
	user.authprofile=0;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_R"))!=NULL) user.authprofile+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_M"))!=NULL) user.authprofile+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_I"))!=NULL) user.authprofile+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_D"))!=NULL) user.authprofile+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHPROFILE_A"))!=NULL) user.authprofile+=A_ADMIN;
	user.authquery=0;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_R"))!=NULL) user.authquery+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_M"))!=NULL) user.authquery+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_I"))!=NULL) user.authquery+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_D"))!=NULL) user.authquery+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHQUERY_A"))!=NULL) user.authquery+=A_ADMIN;
	user.authwebmail=0;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_R"))!=NULL) user.authwebmail+=A_READ;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_M"))!=NULL) user.authwebmail+=A_MODIFY;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_I"))!=NULL) user.authwebmail+=A_INSERT;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_D"))!=NULL) user.authwebmail+=A_DELETE;
	if ((ptemp=getpostenv(sid, "AUTHWEBMAIL_A"))!=NULL) user.authwebmail+=A_ADMIN;
	if ((ptemp=getpostenv(sid, "PREFDAYSTART"))!=NULL) user.prefdaystart=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFDAYLENGTH"))!=NULL) user.prefdaylength=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFMAXLIST"))!=NULL) user.prefmaxlist=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFTIMEZONE"))!=NULL) user.preftimezone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "PREFGEOZONE"))!=NULL) user.prefgeozone=atoi(ptemp);
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
		user.authadmin=A_READ+A_MODIFY+A_INSERT+A_DELETE+A_ADMIN;
	}
	if (strlen(user.birthdate)==0) snprintf(user.birthdate, sizeof(user.birthdate)-1, "1900-01-01");
	if (strlen(user.hiredate)==0) snprintf(user.hiredate, sizeof(user.hiredate)-1, "1900-01-01");
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)||(userid<2)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_users WHERE userid = %d", user.userid)<0) return;
		prints(sid, "<CENTER>User %d deleted successfully</CENTER><BR>\n", userid);
		db_log_activity(sid, 1, "users", user.userid, "delete", "%s - %s deleted user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	} else if (user.userid==0) {
		if ((sqr=sql_queryf(sid, "SELECT username FROM gw_users where username = '%s'", user.username))<0) return;
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
		if ((sqr=sql_query(sid, "SELECT max(userid) FROM gw_users"))<0) return;
		user.userid=atoi(sql_getvalue(sqr, 0, 0))+1;
		sql_freeresult(sqr);
		if (user.userid<1) user.userid=1;
		strcpy(query, "INSERT INTO gw_users (userid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, loginip, logintime, logintoken, username, password, groupid, enabled, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authfiles, authforums, authmessages, authorders, authprofile, authquery, authwebmail, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone, prefgeozone, availability, surname, givenname, jobtitle, division, supervisor, address, locality, region, country, postalcode, homenumber, worknumber, faxnumber, cellnumber, pagernumber, email, birthdate, hiredate, sin, isactive) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", user.userid, curdate, curdate, user.obj_uid, user.obj_gid, user.obj_gperm, user.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'0.0.0.0', '1900-01-01 00:00:00', '', ");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.username));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.password);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.groupid);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', ", user.enabled);
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
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.availability);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.givenname));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.jobtitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.division));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.supervisor));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.address));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.locality));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.region));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.country));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.postalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.homenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.worknumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.faxnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.cellnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.pagernumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.email));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.birthdate);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", user.hiredate);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, user.sin));
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, user.isactive));
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>User added successfully</CENTER><BR>\n");
		db_log_activity(sid, 1, "users", user.userid, "insert", "%s - %s added user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	} else {
		if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((sqr=sql_queryf(sid, "SELECT username FROM gw_users where username = '%s' and userid <> %d", user.username, user.userid))<0) return;
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
		strncatf(query, sizeof(query)-strlen(query)-1, "username = '%s', ", str2sql(sid, user.username));
		strncatf(query, sizeof(query)-strlen(query)-1, "password = '%s', ", user.password);
		strncatf(query, sizeof(query)-strlen(query)-1, "groupid = '%d', ", user.groupid);
		strncatf(query, sizeof(query)-strlen(query)-1, "enabled = '%d', ", user.enabled);
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
		strncatf(query, sizeof(query)-strlen(query)-1, "availability = '%s', ", user.availability);
		strncatf(query, sizeof(query)-strlen(query)-1, "surname = '%s', ", str2sql(sid, user.surname));
		strncatf(query, sizeof(query)-strlen(query)-1, "givenname = '%s', ", str2sql(sid, user.givenname));
		strncatf(query, sizeof(query)-strlen(query)-1, "jobtitle = '%s', ", str2sql(sid, user.jobtitle));
		strncatf(query, sizeof(query)-strlen(query)-1, "division = '%s', ", str2sql(sid, user.division));
		strncatf(query, sizeof(query)-strlen(query)-1, "supervisor = '%s', ", str2sql(sid, user.supervisor));
		strncatf(query, sizeof(query)-strlen(query)-1, "address = '%s', ", str2sql(sid, user.address));
		strncatf(query, sizeof(query)-strlen(query)-1, "locality = '%s', ", str2sql(sid, user.locality));
		strncatf(query, sizeof(query)-strlen(query)-1, "region = '%s', ", str2sql(sid, user.region));
		strncatf(query, sizeof(query)-strlen(query)-1, "country = '%s', ", str2sql(sid, user.country));
		strncatf(query, sizeof(query)-strlen(query)-1, "postalcode = '%s', ", str2sql(sid, user.postalcode));
		strncatf(query, sizeof(query)-strlen(query)-1, "homenumber = '%s', ", str2sql(sid, user.homenumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "worknumber = '%s', ", str2sql(sid, user.worknumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "faxnumber = '%s', ", str2sql(sid, user.faxnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "cellnumber = '%s', ", str2sql(sid, user.cellnumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "pagernumber = '%s', ", str2sql(sid, user.pagernumber));
		strncatf(query, sizeof(query)-strlen(query)-1, "email = '%s', ", str2sql(sid, user.email));
		strncatf(query, sizeof(query)-strlen(query)-1, "birthdate = '%s', ", user.birthdate);
		strncatf(query, sizeof(query)-strlen(query)-1, "hiredate = '%s', ", user.hiredate);
		strncatf(query, sizeof(query)-strlen(query)-1, "sin = '%s', ", str2sql(sid, user.sin));
		strncatf(query, sizeof(query)-strlen(query)-1, "isactive = '%s'", str2sql(sid, user.isactive));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE userid = %d", user.userid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>User %d modified successfully</CENTER><BR>\n", userid);
		db_log_activity(sid, 1, "users", user.userid, "modify", "%s - %s modified user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, user.userid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/userlist\">\n", sid->dat->in_ScriptName);
	return;
}

void adminusertimeedit(CONNECTION *sid)
{
	char *dow[7]={ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char gavailability[170];
	char uavailability[170];
	REC_USER user;
	int userid;
	int i;
	int j;
	int sqr;

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "USERID")==NULL) return;
	userid=atoi(getgetenv(sid, "USERID"));
	if (db_read(sid, 2, DB_USERS, userid, &user)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", userid);
		return;
	}
	if ((sqr=sql_queryf(sid, "SELECT availability FROM gw_users WHERE userid = %d", user.userid))<0) return;
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
	if ((sqr=sql_queryf(sid, "SELECT availability FROM gw_groups WHERE groupid = %d", user.groupid))<0) return;
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
	prints(sid, "		if (availability[\"t\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"t\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"t\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<7;x++) {\n");
	prints(sid, "			if (availability[\"d\" + x + \"t\" + n]) {\n");
	prints(sid, "				availability[\"d\" + x + \"t\" + n].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	} else {\n");
	prints(sid, "		if (availability[\"d\" + n].value == 'true') {\n");
	prints(sid, "			availability[\"d\" + n].value = 'false'\n");
	prints(sid, "			var bool = true\n");
	prints(sid, "		} else {\n");
	prints(sid, "			availability[\"d\" + n].value = 'true'\n");
	prints(sid, "			var bool = false\n");
	prints(sid, "		}\n");
	prints(sid, "		for (x=0;x<24;x++) {\n");
	prints(sid, "			if (availability[\"d\" + n + \"t\" + x]) {\n");
	prints(sid, "				availability[\"d\" + n + \"t\" + x].checked = bool\n");
	prints(sid, "			}\n");
	prints(sid, "		}\n");
	prints(sid, "	}\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n");
	prints(sid, "</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/usertimesave NAME=availability>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=userid VALUE='%d'>\n", user.userid);
	for (i=0;i<7;i++) {
		prints(sid, "<input type='hidden' name='d%d' value='true'>\n", i);
	}
	for (i=0;i<24;i++) {
		prints(sid, "<input type='hidden' name='t%d' value='true'>\n", i);
	}
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=25><FONT COLOR=%s>Availability for <A HREF=%s/admin/useredit?userid=%d STYLE='color: %s'>%s</A></FONT></TH></TR>\n", config->colour_th, config->colour_thtext, sid->dat->in_ScriptName, userid, config->colour_thlink, user.username);
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldname);
	prints(sid, "<TD ALIGN=CENTER ROWSPAN=2>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>A.M.</B></TD>\n");
	prints(sid, "<TD ALIGN=CENTER COLSPAN=12><B>P.M.</B></TD>\n");
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldname);
	for (i=0, j=0;i<24;i++, j++) {
		if (j<1) j=12;
		if (j>12) j-=12;
		prints(sid, "<TD ALIGN=CENTER><A HREF=\"#\" onclick=\"toggle('t','%d')\"><B>%02d</B></A></TD>\n", i, j);
	}
	prints(sid, "</TR>\n");
	for (i=0;i<7;i++) {
		prints(sid, "<TR BGCOLOR=%s>\n", config->colour_fieldval);
		prints(sid, "<TD ALIGN=LEFT NOWRAP BGCOLOR=%s><B>&nbsp;<A HREF=\"#\" onclick=\"toggle('d','%d')\">%s</A>&nbsp;</B></TD>\n", config->colour_fieldname, i, dow[i]);
		for (j=0;j<24;j++) {
			if (uavailability[i*24+j]=='X') {
				prints(sid, "<TD>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>\n");
			} else {
				prints(sid, "<TD><INPUT TYPE=checkbox NAME=d%dt%d VALUE='d%dt%d' %s></TD>\n", i, j, i, j, uavailability[i*24+j]=='1'?"checked":"");
			}
		}
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	return;
}

void adminusertimesave(CONNECTION *sid)
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

	if (!(auth_priv(sid, AUTH_ADMIN)&A_ADMIN)) {
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
	if (sql_updatef(sid, "UPDATE gw_users SET obj_mtime = '%s', availability = '%s' WHERE userid = %d", curdate, availability, userid)<0) return;
	prints(sid, "<CENTER>Availability modified successfully</CENTER><BR>\n");
	db_log_activity(sid, 1, "users", userid, "modify", "%s - %s modified availability for user %d", sid->dat->in_RemoteAddr, sid->dat->user_username, userid);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "location.replace(\"%s/admin/useredit?userid=%d\");\n", sid->dat->in_ScriptName, userid);
	prints(sid, "// -->\n</SCRIPT>\n<NOSCRIPT>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/admin/useredit?userid=%d\">\n", sid->dat->in_ScriptName, userid);
	prints(sid, "</NOSCRIPT>\n");
	return;
}
