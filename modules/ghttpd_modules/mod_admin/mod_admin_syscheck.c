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
#include "ghttpd/mod.h"
#include "mod_admin.h"

void admin_syscheck(CONN *sid)
{
/*	int errors=0;
	int warnings=0;
	int i, j;
	int recordid;
	SQLRES sqr1=-1;
	SQLRES sqr2=-1;
	SQLRES sqr3=-1;
	SQLRES sqr4=-1;
	SQLRES sqr5=-1;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if ((sqr1=sql_query("SELECT * FROM gw_users"))<0) {
		prints(sid, "<B>ERROR: could not query gw_users!</B><BR>");
		errors++;
		goto cleanup;
	}
	if ((sqr2=sql_query("SELECT * FROM gw_groups"))<0) {
		prints(sid, "<B>ERROR: could not query gw_groups!</B><BR>");
		errors++;
		goto cleanup;
	}
	if ((sqr3=sql_query("SELECT * FROM gw_domains"))<0) {
		prints(sid, "<B>ERROR: could not query gw_domains!</B><BR>");
		errors++;
		goto cleanup;
	}
	if ((sqr4=sql_query("SELECT * FROM gw_contacts"))<0) {
		prints(sid, "<B>ERROR: could not query gw_contacts!</B><BR>");
		errors++;
		goto cleanup;
	}
	if ((sqr5=sql_query("SELECT * FROM gw_zones"))<0) {
		prints(sid, "<B>ERROR: could not query gw_zones!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr1);i++) {
		recordid=atoi(sql_getvaluebyname(sqr1, i, "userid"));
		if (strlen(sql_getvaluebyname(sqr1, i, "username"))==0) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> has a blank username!</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		if (strlen(sql_getvaluebyname(sqr1, i, "password"))!=34) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> has a corrupt password!</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr1, i, "groupid"))==atoi(sql_getvaluebyname(sqr2, j, "groupid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> is not in a valid group!</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr1, i, "domainid"))==atoi(sql_getvaluebyname(sqr3, j, "domainid"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> is not in a valid domain!</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr5);j++) {
			if (atoi(sql_getvaluebyname(sqr1, i, "prefgeozone"))==atoi(sql_getvaluebyname(sqr5, j, "zoneid"))) break;
		}
		if (j==sql_numtuples(sqr5)) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> is not in a valid zone!</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	// CHECK CONTACTS
	for (i=0;i<sql_numtuples(sqr4);i++) {
		recordid=atoi(sql_getvaluebyname(sqr4, i, "contactid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr4, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: contact <A HREF=%s/contacts/edit?contactid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr4, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: contact <A HREF=%s/contacts/edit?contactid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr4, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: contact <A HREF=%s/contacts/edit?contactid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1; // DROP ZONES
	// KEEP THE USER, GROUP AND CONTACT TABLES LOADED.  WE'LL BE USING THEM QUITE A BIT

	// CHECK ACTIVITY
	if ((sqr5=sql_query("SELECT * FROM gw_activity"))<0) {
		prints(sid, "<B>ERROR: could not query gw_activity!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "activityid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "userid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: activity log <A HREF=%s/admin/activityview?logid=%d>%d</A> doesn't refer to a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: activity log <A HREF=%s/admin/activityview?logid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK BOOKMARKFOLDERS
	if ((sqr5=sql_query("SELECT * FROM gw_bookmarkfolders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_bookmarkfolders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "folderid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: bookmark folder <A HREF=%s/bookmarks/folderedit?folderid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: bookmark folder <A HREF=%s/bookmarks/folderedit?folderid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: bookmark folder <A HREF=%s/bookmarks/folderedit?folderid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK BOOKMARKS
	if ((sqr5=sql_query("SELECT * FROM gw_bookmarks"))<0) {
		prints(sid, "<B>ERROR: could not query gw_bookmarks!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "bookmarkid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: bookmark <A HREF=%s/bookmarks/edit?bookmarkid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: bookmark <A HREF=%s/bookmarks/edit?bookmarkid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: bookmark <A HREF=%s/bookmarks/edit?bookmarkid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK CALLS
	if ((sqr5=sql_query("SELECT * FROM gw_calls"))<0) {
		prints(sid, "<B>ERROR: could not query gw_calls!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "callid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "assignedto"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK EVENTS
	if ((sqr5=sql_query("SELECT * FROM gw_events"))<0) {
		prints(sid, "<B>ERROR: could not query gw_events!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "eventid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "assignedto"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK FILES
	if ((sqr5=sql_query("SELECT * FROM gw_files"))<0) {
		prints(sid, "<B>ERROR: could not query gw_files!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "fileid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: file <A HREF=%s/fileinfoedit?fileid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: file <A HREF=%s/fileinfoedit?fileid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: file <A HREF=%s/fileinfoedit?fileid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK MAILACCOUNTS
	if ((sqr5=sql_query("SELECT * FROM gw_mailaccounts"))<0) {
		prints(sid, "<B>ERROR: could not query gw_mailaccounts!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "mailaccountid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: mail account %d is not owned by a valid user</B><BR>", recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>WARNING: mail account %d is not owned by a valid domain</B><BR>", recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK MAILFOLDERS
	if ((sqr5=sql_query("SELECT * FROM gw_mailfolders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_mailfolders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "mailfolderid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: mail folder %d is not owned by a valid user</B><BR>", recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>WARNING: mail folder %d is not owned by a valid domain</B><BR>", recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK MAILHEADERS
	if ((sqr5=sql_query("SELECT * FROM gw_mailheaders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_mailheaders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "mailheaderid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: mail header %d is not owned by a valid user</B><BR>", recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>WARNING: mail header %d is not owned by a valid domain</B><BR>", recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK MESSAGES
	if ((sqr5=sql_query("SELECT * FROM gw_messages"))<0) {
		prints(sid, "<B>ERROR: could not query gw_messages!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "messageid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: message %d is not owned by a valid user</B><BR>", recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>WARNING: message %d is not owned by a valid domain</B><BR>", recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "sender"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: message %d does not have a valid sender</B><BR>", recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "rcpt"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: message %d does not have a valid recipient</B><BR>", recordid);
			warnings++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK NOTES
	if ((sqr5=sql_query("SELECT * FROM gw_notes"))<0) {
		prints(sid, "<B>ERROR: could not query gw_notes!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "noteid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: note <A HREF=%s/notes/edit?noteid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: note <A HREF=%s/notes/edit?noteid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: note <A HREF=%s/notes/edit?noteid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK ORDERS
	if ((sqr5=sql_query("SELECT * FROM gw_orders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_orders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "orderid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: order <A HREF=%s/orders/edit?orderid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: order <A HREF=%s/orders/edit?orderid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "userid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: order <A HREF=%s/orders/edit?orderid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK QUERIES
	if ((sqr5=sql_query("SELECT * FROM gw_queries"))<0) {
		prints(sid, "<B>ERROR: could not query gw_queries!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "queryid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: query <A HREF=%s/search/sqladd?queryid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: query <A HREF=%s/search/sqladd?queryid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
	// CHECK TASKS
	if ((sqr5=sql_query("SELECT * FROM gw_tasks"))<0) {
		prints(sid, "<B>ERROR: could not query gw_tasks!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr5);i++) {
		recordid=atoi(sql_getvaluebyname(sqr5, i, "taskid"));
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr3, j, "domainid"))==atoi(sql_getvaluebyname(sqr5, i, "obj_did"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not owned by a valid domain</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr5, i, "assignedto"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, recordid, recordid);
			warnings++;
		}
	}
	sql_freeresult(sqr5); sqr5=-1;
cleanup:
	if ((errors)||(warnings)) prints(sid, "<BR>");
	prints(sid, "<B>%d errors</B><BR>", errors);
	prints(sid, "<B>%d warnings</B><BR>", warnings);
	if (sqr5>-1) sql_freeresult(sqr5);
	if (sqr4>-1) sql_freeresult(sqr4);
	if (sqr3>-1) sql_freeresult(sqr3);
	if (sqr2>-1) sql_freeresult(sqr2);
	if (sqr1>-1) sql_freeresult(sqr1);
*/	return;
}
