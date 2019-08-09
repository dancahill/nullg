/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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

void adminzoneedit(CONN *sid)
{
	REC_ZONE zone;
	char *ptemp;
	int zoneid;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/admin/zoneeditnew")==0) {
		zoneid=0;
		dbread_zone(sid, 2, 0, &zone);
	} else {
		if ((ptemp=getgetenv(sid, "ZONEID"))==NULL) return;
		zoneid=atoi(ptemp);
		if (dbread_zone(sid, 2, zoneid, &zone)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", zoneid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/admin/zonesave NAME=zoneedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=zoneid VALUE='%d'>\r\n", zone.zoneid);
	if (zoneid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>zone %d</TH></TR>\r\n", zoneid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Zone</TH></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Domain</B></TD><TD ALIGN=RIGHT><SELECT NAME=domainid style='width:255px'%s>\r\n", !(auth_priv(sid, "domainadmin")&A_ADMIN)||(zoneid>0)?" DISABLED":"");
	htselect_domain(sid, zone.obj_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>Zone Name    </B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=zonename value=\"%s\" SIZE=25 style='width:255px'></TD></TR>\r\n", str2html(sid, zone.zonename));
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "admin")&A_ADMIN)&&(zoneid>0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.zoneedit.zonename.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	return;
}

void adminzonelist(CONN *sid)
{
	int i, j;
	int rc;
	int lastdomain;
	int thisdomain;
	SQLRES sqr1;
	SQLRES sqr2;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if (sql_queryf(&sqr1, "SELECT zoneid, obj_did, zonename FROM gw_zones ORDER BY obj_did, zonename ASC")<0) return;
	} else {
		if (sql_queryf(&sqr1, "SELECT zoneid, obj_did, zonename FROM gw_zones WHERE obj_did = %d ORDER BY obj_did, zonename ASC", sid->dat->user_did)<0) return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		rc=sql_queryf(&sqr2, "SELECT domainid, domainname FROM gw_domains ORDER BY domainname ASC");
	} else {
		rc=sql_queryf(&sqr2, "SELECT domainid, domainname FROM gw_domains WHERE domainid = %d", sid->dat->user_did);
	}
	if (rc<0) {
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "<CENTER>\r\n");
	if (sql_numtuples(&sqr1)<1) {
		prints(sid, "There are no zones<BR>\r\n");
		prints(sid, "<A HREF=%s/admin/zoneeditnew>New Zone</A>\r\n", sid->dat->in_ScriptName);
		prints(sid, "</CENTER>\r\n");
		sql_freeresult(&sqr2);
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=200 STYLE='border-style:solid'>&nbsp;Zone Name&nbsp;</TH></TR>\r\n");
	lastdomain=-1;
	if (sql_numtuples(&sqr2)<2) {
		lastdomain=atoi(sql_getvalue(&sqr1, 0, 1));
	}
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		thisdomain=atoi(sql_getvalue(&sqr1, i, 1));
		if (lastdomain!=thisdomain) {
			lastdomain=thisdomain;
			prints(sid, "<TR CLASS=\"FIELDNAME\"><TD COLSPAN=4 NOWRAP style='border-style:solid'>");
			for (j=0;j<sql_numtuples(&sqr2);j++) {
				if (atoi(sql_getvalue(&sqr2, j, 0))==atoi(sql_getvalue(&sqr1, i, 1))) {
					prints(sid, "<B>%s</B></TD>", str2html(sid, sql_getvalue(&sqr2, j, 1)));
					break;
				}
			}
			if (j==sql_numtuples(&sqr2)) {
				prints(sid, "&nbsp;</TD>");
			}
		}
		prints(sid, "<TR CLASS=\"FIELDVAL\"><TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/admin/zoneedit?zoneid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "<A HREF=%s/admin/zoneedit?zoneid=%d>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "%s</A>&nbsp;</TD></TR>\r\n", str2html(sid, sql_getvalue(&sqr1, i, 2)));
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "<A HREF=%s/admin/zoneeditnew>New Zone</A>\r\n", sid->dat->in_ScriptName);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr2);
	sql_freeresult(&sqr1);
	return;
}

void adminzonesave(CONN *sid)
{
	REC_ZONE zone;
	char query[2048];
	char curdate[40];
	char *ptemp;
	time_t t;
	int zoneid;
	SQLRES sqr;

	if (!(auth_priv(sid, "admin")&A_ADMIN)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "ZONEID"))==NULL) return;
	zoneid=atoi(ptemp);
	if (dbread_zone(sid, 2, zoneid, &zone)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "domainadmin")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "DOMAINID"))!=NULL) zone.obj_did=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "ZONENAME"))!=NULL) snprintf(zone.zonename, sizeof(zone.zonename)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_updatef("DELETE FROM gw_zones WHERE zoneid = %d", zone.zoneid)<0) return;
		prints(sid, "<CENTER>Zone %d deleted successfully</CENTER><BR>\r\n", zone.zoneid);
		db_log_activity(sid, "zones", zone.zoneid, "delete", "%s - %s deleted zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	} else if (zone.zoneid==0) {
		if (sql_queryf(&sqr, "SELECT zonename FROM gw_zones where zonename = '%s' AND obj_did = %d", zone.zonename, zone.obj_did)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>Zone %s already exists</CENTER><BR>\r\n", zone.zonename);
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(zone.zonename)<1) {
			prints(sid, "<CENTER>Zone name is too short</CENTER><BR>\r\n");
			return;
		}
		if (sql_query(&sqr, "SELECT max(zoneid) FROM gw_zones")<0) return;
		zone.zoneid=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (zone.zoneid<1) zone.zoneid=1;
		strcpy(query, "INSERT INTO gw_zones (zoneid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, zonename) values (");
		strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", zone.zoneid, curdate, curdate, zone.obj_uid, zone.obj_gid, zone.obj_did, zone.obj_gperm, zone.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, zone.zonename));
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Zone %d added successfully</CENTER><BR>\r\n", zone.zoneid);
		db_log_activity(sid, "zones", zone.zoneid, "insert", "%s - %s added zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	} else {
		if (!(auth_priv(sid, "admin")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if (sql_queryf(&sqr, "SELECT zonename FROM gw_zones where zonename = '%s' AND zoneid <> %d AND obj_did = %d", zone.zonename, zone.zoneid, zone.obj_did)<0) return;
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER>Zone %s already exists</CENTER><BR>\r\n", zone.zonename);
			sql_freeresult(&sqr);
			return;
		}
		sql_freeresult(&sqr);
		if (strlen(zone.zonename)<1) {
			prints(sid, "<CENTER>Zone name is too short</CENTER><BR>\r\n");
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_zones SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, zone.obj_uid, zone.obj_gid, zone.obj_gperm, zone.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "zonename = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, zone.zonename));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE zoneid = %d AND obj_did = %d", zone.zoneid, zone.obj_did);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>Zone %d modified successfully</CENTER><BR>\r\n", zone.zoneid);
		db_log_activity(sid, "zones", zone.zoneid, "modify", "%s - %s modified zone %d", sid->dat->in_RemoteAddr, sid->dat->user_username, zone.zoneid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/admin/zonelist\">\r\n", sid->dat->in_ScriptName);
	return;
}
