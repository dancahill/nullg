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
#include "mod_substub.h"
#include "mod_admin.h"

void admin_syscheck(CONN *sid)
{
	int errors=0;
	int warnings=0;
	int i, j;
	int sqr1=-1;
	int sqr2=-1;
	int sqr3=-1;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((sqr1=sql_query(sid, "SELECT * FROM gw_users"))<0) {
		prints(sid, "<B>ERROR: could not query gw_users!</B><BR>");
		errors++;
		goto cleanup;
	}
	if ((sqr2=sql_query(sid, "SELECT * FROM gw_groups"))<0) {
		prints(sid, "<B>ERROR: could not query gw_groups!</B><BR>");
		errors++;
		goto cleanup;
	}
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_zones"))<0) {
		prints(sid, "<B>ERROR: could not query gw_zones!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr1);i++) {
		if (strlen(sql_getvaluebyname(sqr1, i, "username"))==0) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> has a blank username!</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr1, i, "userid")), atoi(sql_getvaluebyname(sqr1, i, "userid")));
			errors++;
		}
		if (strlen(sql_getvaluebyname(sqr1, i, "password"))!=34) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> has a corrupt password!</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr1, i, "userid")), atoi(sql_getvaluebyname(sqr1, i, "userid")));
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr1, i, "groupid"))==atoi(sql_getvaluebyname(sqr2, j, "groupid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> is not in a valid group!</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr1, i, "userid")), atoi(sql_getvaluebyname(sqr1, i, "userid")));
			errors++;
		}
		for (j=0;j<sql_numtuples(sqr3);j++) {
			if (atoi(sql_getvaluebyname(sqr1, i, "prefgeozone"))==atoi(sql_getvaluebyname(sqr3, j, "zoneid"))) break;
		}
		if (j==sql_numtuples(sqr3)) {
			prints(sid, "<B>ERROR: user <A HREF=%s/admin/useredit?userid=%d>%d</A> is not in a valid zone!</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr1, i, "userid")), atoi(sql_getvaluebyname(sqr1, i, "userid")));
			errors++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// KEEP THE USER AND GROUP TABLES LOADED.  WE'LL BE USING THEM QUITE A BIT

	// CHECK ACTIVITY
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_activity"))<0) {
		prints(sid, "<B>ERROR: could not query gw_activity!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "userid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: activity log <A HREF=%s/admin/activityview?logid=%d>%d</A> doesn't refer to a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "activityid")), atoi(sql_getvaluebyname(sqr3, i, "activityid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK BOOKMARKFOLDERS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_bookmarkfolders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_bookmarkfolders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: bookmark folder <A HREF=%s/bookmarks/folderedit?folderid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "folderid")), atoi(sql_getvaluebyname(sqr3, i, "folderid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: bookmark folder <A HREF=%s/bookmarks/folderedit?folderid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "folderid")), atoi(sql_getvaluebyname(sqr3, i, "folderid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK BOOKMARKS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_bookmarks"))<0) {
		prints(sid, "<B>ERROR: could not query gw_bookmarks!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: bookmark <A HREF=%s/bookmarks/edit?bookmarkid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "bookmarkid")), atoi(sql_getvaluebyname(sqr3, i, "bookmarkid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: bookmark <A HREF=%s/bookmarks/edit?bookmarkid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "bookmarkid")), atoi(sql_getvaluebyname(sqr3, i, "bookmarkid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK CALLS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_calls"))<0) {
		prints(sid, "<B>ERROR: could not query gw_calls!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "callid")), atoi(sql_getvaluebyname(sqr3, i, "callid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "callid")), atoi(sql_getvaluebyname(sqr3, i, "callid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "assignedto"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: call <A HREF=%s/calls/edit?callid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "callid")), atoi(sql_getvaluebyname(sqr3, i, "callid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK CONTACTS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_contacts"))<0) {
		prints(sid, "<B>ERROR: could not query gw_contacts!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: contact <A HREF=%s/contacts/edit?contactid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "contactid")), atoi(sql_getvaluebyname(sqr3, i, "contactid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: contact <A HREF=%s/contacts/edit?contactid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "contactid")), atoi(sql_getvaluebyname(sqr3, i, "contactid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK EVENTS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_events"))<0) {
		prints(sid, "<B>ERROR: could not query gw_events!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "eventid")), atoi(sql_getvaluebyname(sqr3, i, "eventid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "eventid")), atoi(sql_getvaluebyname(sqr3, i, "eventid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "assignedto"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: event <A HREF=%s/calendar/edit?eventid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "eventid")), atoi(sql_getvaluebyname(sqr3, i, "eventid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK FILES
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_files"))<0) {
		prints(sid, "<B>ERROR: could not query gw_files!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: file <A HREF=%s/fileinfoedit?fileid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "fileid")), atoi(sql_getvaluebyname(sqr3, i, "fileid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: file <A HREF=%s/fileinfoedit?fileid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "fileid")), atoi(sql_getvaluebyname(sqr3, i, "fileid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK MAILACCOUNTS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_mailaccounts"))<0) {
		prints(sid, "<B>ERROR: could not query gw_mailaccounts!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: mail account %d is not owned by a valid user</B><BR>", atoi(sql_getvaluebyname(sqr3, i, "mailaccountid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK MAILHEADERS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_mailheaders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_mailheaders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: mail header %d is not owned by a valid user</B><BR>", atoi(sql_getvaluebyname(sqr3, i, "mailheaderid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK MESSAGES
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_messages"))<0) {
		prints(sid, "<B>ERROR: could not query gw_messages!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: message %d is not owned by a valid user</B><BR>", atoi(sql_getvaluebyname(sqr3, i, "messageid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "sender"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: message %d does not have a valid sender</B><BR>", atoi(sql_getvaluebyname(sqr3, i, "messageid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "rcpt"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: message %d does not have a valid recipient</B><BR>", atoi(sql_getvaluebyname(sqr3, i, "messageid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK NOTES
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_notes"))<0) {
		prints(sid, "<B>ERROR: could not query gw_notes!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: note <A HREF=%s/notes/edit?noteid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "noteid")), atoi(sql_getvaluebyname(sqr3, i, "noteid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: note <A HREF=%s/notes/edit?noteid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "noteid")), atoi(sql_getvaluebyname(sqr3, i, "noteid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK ORDERS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_orders"))<0) {
		prints(sid, "<B>ERROR: could not query gw_orders!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: order <A HREF=%s/orders/edit?orderid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "orderid")), atoi(sql_getvaluebyname(sqr3, i, "orderid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "userid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: order <A HREF=%s/orders/edit?orderid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "orderid")), atoi(sql_getvaluebyname(sqr3, i, "orderid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK QUERIES
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_queries"))<0) {
		prints(sid, "<B>ERROR: could not query gw_queries!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: query <A HREF=%s/search/sqladd?queryid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "queryid")), atoi(sql_getvaluebyname(sqr3, i, "queryid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: query <A HREF=%s/search/sqladd?queryid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "queryid")), atoi(sql_getvaluebyname(sqr3, i, "queryid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
	// CHECK TASKS
	if ((sqr3=sql_query(sid, "SELECT * FROM gw_tasks"))<0) {
		prints(sid, "<B>ERROR: could not query gw_tasks!</B><BR>");
		errors++;
		goto cleanup;
	}
	for (i=0;i<sql_numtuples(sqr3);i++) {
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_uid"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not owned by a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "taskid")), atoi(sql_getvaluebyname(sqr3, i, "taskid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr2);j++) {
			if (atoi(sql_getvaluebyname(sqr2, j, "groupid"))==atoi(sql_getvaluebyname(sqr3, i, "obj_gid"))) break;
		}
		if (j==sql_numtuples(sqr2)) {
			prints(sid, "<B>WARNING: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not owned by a valid group</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "taskid")), atoi(sql_getvaluebyname(sqr3, i, "taskid")));
			warnings++;
		}
		for (j=0;j<sql_numtuples(sqr1);j++) {
			if (atoi(sql_getvaluebyname(sqr1, j, "userid"))==atoi(sql_getvaluebyname(sqr3, i, "assignedto"))) break;
		}
		if (j==sql_numtuples(sqr1)) {
			prints(sid, "<B>WARNING: task <A HREF=%s/tasks/edit?taskid=%d>%d</A> is not assigned to a valid user</B><BR>", sid->dat->in_ScriptName, atoi(sql_getvaluebyname(sqr3, i, "taskid")), atoi(sql_getvaluebyname(sqr3, i, "taskid")));
			warnings++;
		}
	}
	sql_freeresult(sqr3); sqr3=-1;
cleanup:
	if ((errors)||(warnings)) prints(sid, "<BR>");
	prints(sid, "<B>%d errors</B><BR>", errors);
	prints(sid, "<B>%d warnings</B><BR>", warnings);
	if (sqr3>-1) sql_freeresult(sqr3);
	if (sqr2>-1) sql_freeresult(sqr2);
	if (sqr1>-1) sql_freeresult(sqr1);
	return;
}
