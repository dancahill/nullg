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
#define SRVMOD_MAIN 1
#include "mod_contacts.h"

void contactsearch1(CONN *sid)
{
/*
	char *ptemp;
	int i;
	SQLRES sqr;

	prints(sid, "<BR />\r\n");
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	if (auth_priv(sid, "contacts")&A_READ) {
		prints(sid, "<FORM METHOD=GET ACTION=%s/contacts/search2 NAME=contactsearch>\r\n", sid->dat->in_ScriptName);
		prints(sid, "<TR><TH COLSPAN=3>Contact Search Form</TH></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><SELECT NAME=column>\r\n");
		prints(sid, "<OPTION SELECTED>All Columns\r\n");
		if (sql_query(&sqr, "SELECT * FROM gw_contacts WHERE contactid = 1")<0) return;
		for (i=0;i<sql_numfields(&sqr);i++) {
			if (strcmp(sql_getname(&sqr, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_did")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "obj_operm")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "loginip")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "logintime")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "logintoken")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "password")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "enabled")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "geozone")==0) continue;
			if (strcmp(sql_getname(&sqr, i), "timezone")==0) continue;
			prints(sid, "<OPTION>%s\r\n", sql_getname(&sqr, i));
		}
		sql_freeresult(&sqr);
		prints(sid, "</SELECT></TD>\r\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=string value='%s' SIZE=20></TD>\r\n", (ptemp=getgetenv(sid, "STRING"))!=NULL?ptemp:"");
		prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Search'></TD></TR>\r\n");
		prints(sid, "</FORM>\r\n");
		prints(sid, "<TR><TD COLSPAN=3>&nbsp;</TD></TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.contactsearch.string.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void contactsearch2(CONN *sid)
{
/*
	char query[2048];
	char column[100];
	char string[100];
	char string2[100];
	char *ptemp;
	int offset=0;
	int i;
	SQLRES sqr1;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	contactsearch1(sid);
	memset(column, 0, sizeof(column));
	memset(string, 0, sizeof(string));
	memset(string2, 0, sizeof(string2));
	if ((ptemp=getgetenv(sid, "COLUMN"))!=NULL) snprintf(column, sizeof(column)-1, "%s", ptemp);
	if ((ptemp=getgetenv(sid, "STRING"))!=NULL) snprintf(string, sizeof(string)-1, "%s", ptemp);
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	if (offset<0) offset=0;
	snprintf(string2, sizeof(string2)-1, "%%%s%%", string);
	ptemp=string2;
	while (*ptemp) {
		if (*ptemp=='*') *ptemp='%';
		if (*ptemp=='?') *ptemp='_';
		ptemp++;
	}
	if (sql_query(&sqr1, "SELECT * FROM gw_contacts WHERE contactid = 1")<0) return;
	strcpy(query, "SELECT contactid, surname, givenname, organization, worknumber, homenumber from gw_contacts WHERE (");
	if (strcmp(column, "All Columns")==0) {
		for (i=0;i<sql_numfields(&sqr1);i++) {
			if (strcmp(sql_getname(&sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "obj_operm")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "loginip")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "logintime")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "logintoken")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "password")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "enabled")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "geozone")==0) continue;
			if (strcmp(sql_getname(&sqr1, i), "timezone")==0) continue;
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s') or ", sql_getname(&sqr1, i), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
		}
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid like '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", column, str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, string2));
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		strncatf(query, sizeof(query)-strlen(query)-1, ") AND obj_did = %d ORDER BY surname, givenname ASC", sid->dat->did);
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, ") and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) AND obj_did = %d ORDER BY surname, givenname ASC", sid->dat->uid, sid->dat->gid, sid->dat->did);
	}
	sql_freeresult(&sqr1);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ContactSync(contactid) {\r\n");
	prints(sid, "	if (window.opener.document.calledit) {\r\n");
	prints(sid, "		window.opener.document.calledit.contactid.value=contactid;\r\n");
	prints(sid, "	} else if (window.opener.document.eventedit) {\r\n");
	prints(sid, "		window.opener.document.eventedit.contactid.value=contactid;\r\n");
	prints(sid, "	} else if (window.opener.document.taskedit) {\r\n");
	prints(sid, "		window.opener.document.taskedit.contactid.value=contactid;\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	window.close();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (sql_query(&sqr1, query)<0) return;
	if (sql_numtuples(&sqr1)<1) {
		prints(sid, "<B>Found %d matching contact%s</B></CENTER>\r\n", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s");
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "<B>Found %d matching contact%s</B>\r\n", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailform>\r\n");
	prints(sid, "<TR><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Contact Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Company Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Work Number&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Home Number&nbsp;</TH><TH STYLE='border-style:solid'>&nbsp;</TH></TR>\r\n");
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/contacts/view?contactid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), str2html(sid, sql_getvalue(&sqr1, i, 1)));
		if (strlen(sql_getvalue(&sqr1, i, 1))&&strlen(sql_getvalue(&sqr1, i, 2))) prints(sid, ", ");
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 2)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 3)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 4)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 5)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=javascript:ContactSync('%d')>&gt;&gt;</A></TD>", atoi(sql_getvalue(&sqr1, i, 0)));
		prints(sid, "</TR>\r\n");
	}
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
*/
	return;
}

void contactview(CONN *sid, obj_t **xobj)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL;
//	obj_t *tobj;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	HTMOD_NOTES_SUBLIST mod_notes_sublist;
//	REC_CONTACT contactrec;
	char *ptemp;
	char *greytoggle;
	char baddr[250];
	char maddr[250];
	int perm=0;
	int is_editable;
	int is_recycled=0;
//	int editperms=0;
	int id;
//	SQLRES sqr;
	int tz1;
	int tz2;
	time_t t;
	int priv=auth_priv(sid, "contacts");

	if (!(priv&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (qobj==NULL) {
		qobj=nes_settable(sid->N, &sid->N->g, "CONTACT");
		prints(sid, "<BR />\r\n");
		if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/contacts/viewnew", 17)==0) {
			id=0;
			if ((perm=dbread_contact(sid, 2, 0, &qobj))<0) {
				prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
				return;
			}
		} else {
			if ((ptemp=getgetenv(sid, "ID"))==NULL) return;
			id=atoi(ptemp);
			if ((perm=dbread_contact(sid, 1, id, &qobj))<0) {
				prints(sid, "<CENTER>");
				prints(sid, lang_gets(sid, "common", "err_norecord"), id);
				prints(sid, "</CENTER>\r\n");
				return;
			}
		}
//		contact=&contactrec;
	} else {
		id=atoi(ldir_getval(&qobj, 0, "id"));
//		id=contact->contactid;
		if ((ptemp=getgetenv(sid, "email"))==NULL) is_recycled=1;
	}
	memset(maddr, 0, sizeof(maddr));
	memset(baddr, 0, sizeof(baddr));
	if (strlen(ldir_getval(&qobj, 0, "homeaddress"))&&strlen(ldir_getval(&qobj, 0, "homelocality"))&&strlen(ldir_getval(&qobj, 0, "homeregion"))&&strlen(ldir_getval(&qobj, 0, "homecountry"))) {
//		snprintf(maddr, sizeof(maddr)-1, "http://maps.google.com/maps?q=%s %s %s %s", contact->homeaddress, contact->homelocality, contact->homeregion, contact->homecountry);
		snprintf(maddr, sizeof(maddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", ldir_getval(&qobj, 0, "homecountry"), ldir_getval(&qobj, 0, "homeaddress"), ldir_getval(&qobj, 0, "homelocality"), ldir_getval(&qobj, 0, "homeregion"));
	}
	ptemp=maddr;
	while (*ptemp!=0) {
		if (*ptemp==' ') *ptemp='+';
		ptemp++;
	}
	if (strlen(ldir_getval(&qobj, 0, "workaddress"))&&strlen(ldir_getval(&qobj, 0, "worklocality"))&&strlen(ldir_getval(&qobj, 0, "workregion"))&&strlen(ldir_getval(&qobj, 0, "workcountry"))) {
		snprintf(maddr, sizeof(maddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", ldir_getval(&qobj, 0, "workcountry"), ldir_getval(&qobj, 0, "workaddress"), ldir_getval(&qobj, 0, "worklocality"), ldir_getval(&qobj, 0, "workregion"));
	}
	ptemp=baddr;
	while (*ptemp!=0) {
		if (*ptemp==' ') *ptemp='+';
		ptemp++;
	}
	greytoggle=" DISABLED";
	is_editable=0;
	if (priv&A_ADMIN) {
		greytoggle="";
		is_editable=1;
//	} else if ((priv&A_MODIFY)&&(perm>=2)) {
	} else if (priv&A_MODIFY) {
		greytoggle="";
		is_editable=1;
	}
	tz1=time_tzoffset(sid, time(NULL));
	tz2=time_tzoffsetcon(sid, time(NULL), id);
	if (id<1) tz2=tz1;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function vcardImport() {\r\n");
	prints(sid, "	window.open('%s/contacts/vcardimport?id=%d','vcardimport','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=150');\r\n", ScriptName, id);
	prints(sid, "}\r\n");
	htscript_showpage(sid, 7);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	if (id>0) {
		prints(sid, "<B>%s%s", str2html(sid, ldir_getval(&qobj, 0, "gn")), strlen(ldir_getval(&qobj, 0, "gn"))?" ":"");
		prints(sid, "%s%s</B>", str2html(sid, ldir_getval(&qobj, 0, "sn")), strlen(ldir_getval(&qobj, 0, "sn"))?" ":"");
	} else {
		prints(sid, "<B>New Contact</B>\r\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/contacts/save NAME=contactedit>\r\n", ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=id VALUE='%d'>\r\n", id);
	if (strncmp(nes_getstr(sid->N, htobj, "REQUEST_URI"), "/contacts/viewnew2", 18)==0) {
		prints(sid, "<INPUT TYPE=hidden NAME=callset VALUE='1'>\r\n");
	}
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR CLASS=\"FIELDNAME\">\r\n");
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=1 HREF=javascript:showpage(1)>SUMMARY</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=2 HREF=javascript:showpage(2)>NAME</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=3 HREF=javascript:showpage(3)>HOME</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=4 HREF=javascript:showpage(4)>WORK</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page5tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=5 HREF=javascript:showpage(5)>USER</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page6tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=6 HREF=javascript:showpage(6)>PERMISSIONS</A>&nbsp;</TD>\r\n");
	prints(sid, "<TD ID=page7tab NOWRAP STYLE='border-style:solid'>&nbsp;<A ACCESSKEY=7 HREF=javascript:showpage(7)>OTHER</A>&nbsp;</TD>\r\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD VALIGN=TOP STYLE='padding:3px'>");
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Name&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
	prints(sid, "%s%s", str2html(sid, ldir_getval(&qobj, 0, "personaltitle")), strlen(ldir_getval(&qobj, 0, "personaltitle"))?" ":"");
	prints(sid, "%s%s", str2html(sid, ldir_getval(&qobj, 0, "gn")), strlen(ldir_getval(&qobj, 0, "gn"))?" ":"");
	prints(sid, "%s%s", str2html(sid, ldir_getval(&qobj, 0, "sn")), strlen(ldir_getval(&qobj, 0, "sn"))?" ":"");
	if (strlen(ldir_getval(&qobj, 0, "uid"))) {
		prints(sid, " (%s)", str2html(sid, ldir_getval(&qobj, 0, "uid")));
	}
	prints(sid, "</TD>\r\n");
	if (id>0) {
		prints(sid, "<TD ALIGN=right><A HREF=%s/contacts/vcardexport?id=%d>vCard</A></TD>\r\n", ScriptName, id);
	} else {
		prints(sid, "<TD ALIGN=right>&nbsp;</TD>\r\n");
	}
	prints(sid, "</TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;E-Mail Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>");
	if (strlen(ldir_getval(&qobj, 0, "mail"))==0) {
		prints(sid, "&nbsp;</TD></TR>\r\n");
//	} else if (sid->dat->maildefault==0) {
		prints(sid, "<A HREF=\"mailto:%s\">%s</A>&nbsp;</TD></TR>\r\n", ldir_getval(&qobj, 0, "mail"), str2html(sid, ldir_getval(&qobj, 0, "mail")));
//	} else {
//		if (sid->dat->menustyle>0) {
//			prints(sid, "<A HREF=\"javascript:MsgTo('");
//			prints(sid, "&quot;%s%s%s&quot;", str2html(sid, contact->givenname), strlen(contact->givenname)?" ":"", str2html(sid, contact->surname));
//			prints(sid, " <%s>')\">%s</A>&nbsp;</TD></TR>\r\n", contact->email, str2html(sid, contact->email));
//		} else {
//			prints(sid, "<A HREF=\"mailwrite?to=%s\">%s</A>&nbsp;</TD></TR>\r\n", contact->email, str2html(sid, contact->email));
//		}
	}
	if (strlen(ldir_getval(&qobj, 0, "labeleduri"))) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Web Site&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>%s</A>&nbsp;</TD></TR>\r\n", ldir_getval(&qobj, 0, "labeleduri"), ldir_getval(&qobj, 0, "labeleduri"));
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Home Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "hometelephonenumber")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Mobile Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "mobiletelephonenumber")));
	if (strlen(maddr)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Home Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>Map Available</A>&nbsp;</TD></TR>\r\n", maddr);
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Organization&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "organization")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Job Title&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "title")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Work Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "worknumber")));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;FAX Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "facsimiletelephonenumber")));
	if (strlen(baddr)) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Work Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>Map Available</A>&nbsp;</TD></TR>\r\n", baddr);
	}
	if (tz1!=tz2) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Time&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><SPAN ID=contz></SPAN>&nbsp;</TD></TR>\r\n");
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Surname           &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname      value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "sn")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Given Name        &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "gn")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Title             &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=salutation   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "personaltitle")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Contact Type      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=contacttype  value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "contacttype")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Referred By       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=referredby   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "referredby")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Alternate Contact &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=altcontact   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "altcontact")), greytoggle);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Phone      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "homenumber")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Mobile Number   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=mobilenumber   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "mobilenumber")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;E-mail          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email          value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "mail")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Web Site        &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=website        value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "labeleduri")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Address    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homeaddress    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "homeaddress")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home City       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homelocality   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "homelocality")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Province   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homeregion     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "homeregion")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Country    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homecountry    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "homecountry")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Home Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homepostalcode value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "homepostalcode")), greytoggle);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Organization    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=organization   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "organization")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Job Title       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle       value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "title")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Phone      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "worknumber")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Fax Number      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber      value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "faxnumber")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Address    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workaddress    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "workaddress")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work City       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worklocality   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "worklocality")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Province   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workregion     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "workregion")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Country    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workcountry    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "workcountry")), greytoggle);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Work Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workpostalcode value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "workpostalcode")), greytoggle);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page5 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if (module_exists("mod_xmlrpc")) {
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Username          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "uid")), greytoggle);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Password          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "userpassword")), greytoggle);
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Allow Login       &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:255px'%s>\r\n", greytoggle);
		prints(sid, "<OPTION VALUE=0%s>No\r\n", atoi(ldir_getval(&qobj, 0, "enabled"))?"":" SELECTED");
		prints(sid, "<OPTION VALUE=1%s>Yes\r\n", atoi(ldir_getval(&qobj, 0, "enabled"))?" SELECTED":"");
		prints(sid, "</SELECT></TD></TR>\r\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=geozone style='width:255px'%s>\r\n", greytoggle);
	htselect_zone(sid, atoi(ldir_getval(&qobj, 0, "geozone")), sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Time Zone      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=timezone style='width:255px'%s>\r\n", greytoggle);
	htselect_timezone(sid, atoi(ldir_getval(&qobj, 0, "timezone")));
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP><B>&nbsp;Billing Method &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=prefbilling    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\r\n", str2html(sid, ldir_getval(&qobj, 0, "prefbilling")), greytoggle);
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page6 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
/*	if ((contact->obj_uid==sid->dat->uid)||(priv&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:255px'%s>\r\n", (priv&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, contact->obj_uid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:255px'%s>\r\n", ((priv&A_ADMIN)||(contact->obj_uid==sid->dat->did))?"":" DISABLED");
	htselect_group(sid, priv, contact->obj_gid, sid->dat->did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\r\n", contact->obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\r\n", contact->obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\r\n", contact->obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\r\n", contact->obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\r\n", contact->obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\r\n", contact->obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
*/
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page7 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	if (id>0) {
/*
		if (module_exists("mod_calls")&&(auth_priv(sid, "calls")&A_READ)) {
			if (sql_queryf(&sqr, "SELECT count(callid) FROM gw_calls WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d", id, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
			prints(sid, "<TR CLASS=\"EDITFORM\">");
			prints(sid, "<TD NOWRAP WIDTH=100%%>%d Calls</TD>\r\n", atoi(sql_getvalue(&sqr, 0, 0)));
			prints(sid, "<TD>");
			if (atoi(sql_getvalue(&sqr, 0, 0))>0) {
				prints(sid, "<A HREF=%s/contacts/callslist?contactid=%d>list</A>", ScriptName, id);
			}
			prints(sid, "&nbsp;</TD>\r\n");
			prints(sid, "<TD><A HREF=%s/calls/editnew?contactid=%d>new</A></TD>\r\n", ScriptName, id);
			prints(sid, "</TR>\r\n");
			sql_freeresult(&sqr);
		}
		if (module_exists("mod_calendar")&&(auth_priv(sid, "calendar")&A_READ)) {
			if (sql_queryf(&sqr, "SELECT count(eventid) FROM gw_events WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) AND obj_did = %d", id, sid->dat->uid, sid->dat->uid, sid->dat->gid, sid->dat->did)<0) return;
			prints(sid, "<TR CLASS=\"EDITFORM\">");
			prints(sid, "<TD NOWRAP WIDTH=100%%>%d Events</TD>\r\n", atoi(sql_getvalue(&sqr, 0, 0)));
			prints(sid, "<TD>");
			if (atoi(sql_getvalue(&sqr, 0, 0))>0) {
				prints(sid, "<A HREF=%s/contacts/eventlist?contactid=%d>list</A>", ScriptName, id);
			}
			prints(sid, "&nbsp;</TD>\r\n");
			prints(sid, "<TD><A HREF=%s/calendar/editnew?contactid=%d>new</A></TD>\r\n", ScriptName, id);
			prints(sid, "</TR>\r\n");
			sql_freeresult(&sqr);
		}
		if (module_exists("mod_finance")&&(auth_priv(sid, "finance")&A_READ)) {
			if (sql_queryf(&sqr, "SELECT count(invoiceid) FROM gw_finance_invoices WHERE contactid = %d AND obj_did = %d", id, sid->dat->did)<0) return;
			prints(sid, "<TR CLASS=\"EDITFORM\">");
			prints(sid, "<TD NOWRAP WIDTH=100%%>%d Invoices</TD>\r\n", atoi(sql_getvalue(&sqr, 0, 0)));
			prints(sid, "<TD>");
			if (atoi(sql_getvalue(&sqr, 0, 0))>0) {
				prints(sid, "<A HREF=%s/contacts/orderlist?contactid=%d>list</A>", ScriptName, id);
			}
			prints(sid, "&nbsp;</TD>\r\n");
			prints(sid, "<TD><A HREF=%s/finance/invoices/editnew?contactid=%d>new</A></TD>\r\n", ScriptName, id);
			prints(sid, "</TR>\r\n");
			sql_freeresult(&sqr);
		}
*/
	}
/*
	if (module_exists("mod_email")&&(auth_priv(sid, "email")&A_READ)&&(strchr(contact->email, '@')!=NULL)) {
		if (sql_queryf(&sqr, "SELECT count(mailheaderid) FROM gw_email_headers WHERE obj_uid = %d AND hdr_from LIKE '%%%s%%' OR hdr_to LIKE '%%%s%%' OR hdr_cc LIKE '%%%s%%' AND status != 'd'", sid->dat->uid, contact->email, contact->email, contact->email)<0) return;
		prints(sid, "<TR CLASS=\"EDITFORM\">");
		prints(sid, "<TD NOWRAP WIDTH=100%%>%d E-Mails</TD>\r\n", atoi(sql_getvalue(&sqr, 0, 0)));
		prints(sid, "<TD>");
		if (atoi(sql_getvalue(&sqr, 0, 0))>0) {
			if (sid->dat->menustyle>0) {
				prints(sid, "<A HREF=%s/mail/main?c=addr&text=%s TARGET=gwmain>list</A>", ScriptName, contact->email);
			} else {
				prints(sid, "<A HREF=%s/mail/main?c=addr&text=%s>list</A>", ScriptName, contact->email);
			}
		}
		prints(sid, "&nbsp;</TD>\r\n");
		if (strlen(contact->email)==0) {
			prints(sid, "<TD>&nbsp;</TD></TR>\r\n");
		} else if (sid->dat->maildefault==0) {
			prints(sid, "<TD><A HREF=\"mailto:%s\">new</A></TD>\r\n", contact->email);
		} else {
			if (sid->dat->menustyle>0) {
				prints(sid, "<TD><A HREF=\"javascript:MsgTo('");
				prints(sid, "&quot;%s%s%s&quot;", str2html(sid, contact->givenname), strlen(contact->givenname)?" ":"", str2html(sid, contact->surname));
				prints(sid, " <%s>')\">new</A></TD>\r\n", contact->email);
			} else {
				prints(sid, "<TD><A HREF=\"mailwrite?to=%s\">new</A></TD>\r\n", contact->email);
			}
		}
		prints(sid, "</TR>\r\n");
		sql_freeresult(&sqr);
	}
*/
	prints(sid, "<TD COLSPAN=3><BR /></TD>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=3><A HREF=javascript:vcardImport();>Import vCard</A></TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\r\n");
	if (id>0) {
		if ((mod_notes_sublist=module_call("mod_notes_sublist"))!=NULL) {
			prints(sid, "<TR><TD NOWRAP>");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR><TH NOWRAP STYLE='border-style:solid'>Notes");
			prints(sid, " [<A HREF=%s/notes/editnew?table=contacts&index=%d>new</A>]", ScriptName, id);
			prints(sid, "</FONT></TH></TR>\r\n");
			mod_notes_sublist(sid, "contacts", id, 1);
			prints(sid, "</TABLE>\r\n");
			prints(sid, "</TD></TR>\r\n");
		}
	}
	if (is_editable) {
		prints(sid, "<TR><TD ALIGN=CENTER>\r\n");
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\r\n");
		if (is_recycled) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Force Save'>\r\n");
		}
		if ((priv&A_DELETE)&&(id!=0)) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
		}
		prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	if ((id<1)||(is_recycled)) {
		prints(sid, "showpage(2);\r\n");
		prints(sid, "document.contactedit.surname.focus();\r\n");
	} else {
		prints(sid, "showpage(1);\r\n");
	}
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
//	if (qobj) ldir_freeresult(&qobj);
	return;
}

void contact_mailview(CONN *sid)
{
/*
	REC_CONTACT contactrec;
	char email[100];
	char givenname[100];
	char surname[100];
	char *ptemp;
	int contactid;
	SQLRES sqr;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	memset(email, 0, sizeof(email));
	memset(givenname, 0, sizeof(givenname));
	memset(surname, 0, sizeof(surname));
	if ((ptemp=getgetenv(sid, "email"))!=NULL) {
		snprintf(email, sizeof(email)-1, "%s", ptemp);
	}
	if ((ptemp=getgetenv(sid, "name"))!=NULL) {
		while ((*ptemp)&&(strlen(givenname)<sizeof(givenname)-1)) {
			if (*ptemp==' ') break;
			givenname[strlen(givenname)]=*ptemp++;
		}
		while (*ptemp==' ') ptemp++;
		while ((*ptemp)&&(strlen(surname)<sizeof(surname)-1)) {
			if (*ptemp==' ') break;
			surname[strlen(surname)]=*ptemp++;
		}
	}
	if (sql_queryf(&sqr, "SELECT contactid FROM gw_contacts WHERE email = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, email))<0) return;
	if (sql_numtuples(&sqr)>0) {
		contactid=atoi(sql_getvalue(&sqr, 0, 0));
		sql_freeresult(&sqr);
		if (dbread_contact(sid, 1, contactid, &contactrec)<0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", contactid);
			return;
		}
	} else {
		contactid=0;
		sql_freeresult(&sqr);
		if (dbread_contact(sid, 2, 0, &contactrec)<0) {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			return;
		}
		snprintf(contactrec.surname, sizeof(contactrec.surname)-1, "%s", surname);
		snprintf(contactrec.givenname, sizeof(contactrec.givenname)-1, "%s", givenname);
		snprintf(contactrec.email, sizeof(contactrec.email)-1, "%s", email);
	}
	contactview(sid, &contactrec);
*/
	return;
}

void contactlist(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	obj_t *qobj=NULL;
	char *ptemp;
	char searchstring[10];
	int offset=0;
	int i;
//	SQLRES sqr1;
	int maildefault=0;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	prints(sid, "<SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"JavaScript\"><!--\r\n");
	prints(sid, "function findMailtoLink(text) {\r\n");
	prints(sid, "	for (var i=0;i<document.links.length;i++) {\r\n");
	prints(sid, "		if (document.links[i].href==text) return i;\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	return null;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function MailUpdate() {\r\n");
	prints(sid, "	var entries=document.mailform.elements.length/2;\r\n");
	if (maildefault==0) {
		prints(sid, "	var output=\"mailto:\";\r\n");
	} else {
//		if (sid->dat->menustyle>0) {
			prints(sid, "	var output=\"javascript:MailTo('\";\r\n");
//		} else {
//			prints(sid, "	var output=\"%s/mail/write?\";\r\n", ScriptName);
//		}
	}
	prints(sid, "	var cc=0;\r\n");
	prints(sid, "	var j=0;\r\n\r\n");
	if (maildefault>0) {
		prints(sid, "	output+='to=';\r\n");
	}
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='TO') {\r\n");
	prints(sid, "			if (j>0) output+='%s';\r\n", (maildefault>0)?",":";");
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='CC') {\r\n");
	prints(sid, "			if (j==0) output+='%scc=';\r\n", (maildefault>0)?"&":"?");
	prints(sid, "			else if (j>0) output+='%s';\r\n", (maildefault>0)?",":";");
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			cc=1;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='BCC') {\r\n");
	prints(sid, "			if (j==0) {\r\n");
	if (maildefault>0) {
		prints(sid, "				if (j==0) output+='&bcc=';\r\n");
	} else {
		prints(sid, "				if (cc==1) {\r\n");
		prints(sid, "					output+='&bcc=';\r\n");
		prints(sid, "				} else {\r\n");
		prints(sid, "					output+='?bcc=';\r\n");
		prints(sid, "				}\r\n");
	}
	prints(sid, "			} else if (j>0) output+='%s';\r\n", (maildefault>0)?",":";");
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
//	if ((maildefault>0)&&(sid->dat->menustyle>0)) {
		prints(sid, "	output+=\"')\";\r\n");
//	}
	prints(sid, "	document.links[mailtoLink2].href=output;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function MailTo(rcptlist) {\r\n");
	prints(sid, "	window.open('%s/mail/write?'+rcptlist,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "//--></SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD ALIGN=CENTER VALIGN=TOP WIDTH=100%%>\r\n");
	if ((ptemp=getgetenv(sid, "S"))==NULL) ptemp="All";
	strncpy(searchstring, ptemp, sizeof(searchstring)-1);
	if (dblist_contacts(sid, &qobj, "surname", searchstring)<0) return;
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	prints(sid, "<CENTER><BR />\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\r\n");
	prints(sid, "<TD ALIGN=LEFT NOWRAP WIDTH=125>&nbsp;</TD>\r\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP><B>Found %d matching contact%s</B></TD>\r\n", ldir_numentries(&qobj), ldir_numentries(&qobj)==1?"":"s");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP WIDTH=125>&nbsp;</TD>\r\n");
	prints(sid, "</TR>\r\n");
	if (ldir_numentries(&qobj)>0) {
		prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3>\r\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=500 STYLE='border-style:solid'>\r\n");
		prints(sid, "<FORM METHOD=GET NAME=mailform>\r\n");
		prints(sid, "<TR><TH ALIGN=LEFT NOWRAP WIDTH=125 STYLE='border-style:solid'>&nbsp;Contact Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP WIDTH=125 STYLE='border-style:solid'>&nbsp;Company Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP WIDTH=125 STYLE='border-style:solid'>&nbsp;Work Number&nbsp;</TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP WIDTH=125 STYLE='border-style:solid'>&nbsp;E-Mail&nbsp;</TH></TR>\r\n");
		for (i=offset;(i<ldir_numentries(&qobj))&&(i<offset+sid->dat->maxlist);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/contacts/view?id=%d'\">", ScriptName, atoi(ldir_getval(&qobj, i, "id")));
			prints(sid, "<A HREF=%s/contacts/view?id=%d>%s", ScriptName, atoi(ldir_getval(&qobj, i, "id")), str2html(sid, ldir_getval(&qobj, i, "sn")));
			if (strlen(ldir_getval(&qobj, i, "sn"))&&strlen(ldir_getval(&qobj, i, "gn"))) prints(sid, ", ");
			if (strlen(ldir_getval(&qobj, i, "sn"))==0&&strlen(ldir_getval(&qobj, i, "gn"))==0) prints(sid, "&nbsp;");
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, ldir_getval(&qobj, i, "gn")));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, ldir_getval(&qobj, i, "organization")));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, ldir_getval(&qobj, i, "worknumber")));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid;border-right-width:0px'>");
			if (strlen(ldir_getval(&qobj, i, "mail"))==0) {
				prints(sid, "&nbsp;</TD>");
			} else if (maildefault==0) {
				prints(sid, "<A HREF=\"mailto:%s\">%s</A>&nbsp;</TD>", ldir_getval(&qobj, i, "mail"), ldir_getval(&qobj, i, "mail"));
			} else {
//				if (sid->dat->menustyle>0) {
					prints(sid, "<A HREF=\"javascript:MsgTo('&quot;");
					prints(sid, "%s%s%s", str2html(sid, ldir_getval(&qobj, i, "gn")), strlen(ldir_getval(&qobj, i, "gn"))?" ":"", str2html(sid, ldir_getval(&qobj, i, "sn")));
					prints(sid, "&quot; <%s>')\">", ldir_getval(&qobj, i, "mail"));
//				} else {
//					prints(sid, "<A HREF=\"%s/mail/write?to=%s\">", ScriptName, ldir_getval(&qobj, i, "mail"));
//				}
				prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, ldir_getval(&qobj, i, "mail")));
			}
			prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", i-offset, ldir_getval(&qobj, i, "mail"));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='padding:0px; border-style:solid'><SELECT NAME=option%d onchange=MailUpdate(); STYLE='font-size:11px; width:44px'>", i-offset);
			prints(sid, "<OPTION VALUE=''>");
			if (strchr(ldir_getval(&qobj, i, "mail"), '@')) {
				prints(sid, "<OPTION VALUE='TO'>TO");
				prints(sid, "<OPTION VALUE='CC'>CC");
				prints(sid, "<OPTION VALUE='BCC'>BCC");
			}
			prints(sid, "</SELECT></TD></TR>\r\n");
		}
		prints(sid, "</FORM>\r\n");
		prints(sid, "</TABLE>\r\n</TD></TR>\r\n");
		prints(sid, "<TR>\r\n<TD ALIGN=LEFT NOWRAP WIDTH=125>&nbsp;</TD>\r\n");
		prints(sid, "<TD ALIGN=CENTER NOWRAP>");
		if (ldir_numentries(&qobj)>sid->dat->maxlist) {
			if (offset>sid->dat->maxlist-1) {
				prints(sid, "[<A HREF=%s/contacts/list?s=%s&offset=%d>Previous Page</A>]", ScriptName, searchstring, offset-sid->dat->maxlist);
			} else {
				prints(sid, "[Previous Page]");
			}
			if (offset+sid->dat->maxlist<ldir_numentries(&qobj)) {
				prints(sid, "[<A HREF=%s/contacts/list?s=%s&offset=%d>Next Page</A>]", ScriptName, searchstring, offset+sid->dat->maxlist);
			} else {
				prints(sid, "[Next Page]");
			}
		}
		prints(sid, "</TD>\r\n<TD ALIGN=RIGHT NOWRAP><A HREF=\"mailto:list\" onClick=\"MailUpdate()\">Send E-Mail</A></TD>\r\n</TR>");
		prints(sid, "</TABLE>\r\n");
		prints(sid, "<SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"JavaScript\"><!--\r\n");
		prints(sid, "var mailtoLink2 = findMailtoLink('mailto:list');\r\n");
		prints(sid, "//--></SCRIPT>\r\n");
	} else {
		prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3 WIDTH=100%%>&nbsp;</TD></TR>\r\n");
		prints(sid, "</TABLE>\r\n");
	}
	ldir_freeresult(&qobj);
	prints(sid, "</TD><TD CLASS=\"FIELDNAME\" VALIGN=TOP>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\r\n");
	prints(sid, "<TR CLASS=\"FIELDNAME\"><TD ALIGN=CENTER><FONT SIZE=2><B><A HREF=%s/contacts/list?s=all>&nbsp;All&nbsp;</A></B></FONT></TD></TR>\r\n", ScriptName);
	for (i=65;i<91;i++) {
		prints(sid, "<TR CLASS=\"FIELDNAME\"><TD ALIGN=CENTER><FONT SIZE=2><B><A HREF=%s/contacts/list?s=%c>&nbsp;%c&nbsp;</A></B></FONT></TD></TR>\r\n", ScriptName, i, i);
	}
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</TD></TR></TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	return;
}

/****************************************************************************
 *	contactcallslist()
 *
 *	Purpose	: Display a list of events for a given contact
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void contactcallslist(CONN *sid)
{
/*
	char *ptemp;
	int contactid=0;
	int duration;
	int i;
	SQLRES sqr1;
	SQLRES sqr2;
	time_t calldate;

	prints(sid, "<BR />\r\n");
	if (!module_exists("mod_calls")) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (!(auth_priv(sid, "calls")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) return;
	contactid=atoi(ptemp);
	if (contactid<1) {
		prints(sid, "<CENTER>No matching events found</CENTER>\r\n");
		return;
	}
	prints(sid, "<CENTER>\r\n");
	if (sql_queryf(&sqr1, "SELECT callid, contactid, action, callstart, callfinish, status FROM gw_calls WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY callstart DESC", contactid, sid->dat->uid, sid->dat->uid, sid->dat->gid)<0) return;
	if (sql_queryf(&sqr2, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid)<0) {
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "Found %d call%s for <A HREF=%s/contacts/view?contactid=%s>%s", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s", sid->dat->in_ScriptName, sql_getvalue(&sqr2, 0, 0), str2html(sid, sql_getvalue(&sqr2, 0, 1)));
	if (strlen(sql_getvalue(&sqr2, 0, 1))&&strlen(sql_getvalue(&sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR />\r\n", str2html(sid, sql_getvalue(&sqr2, 0, 2)));
	sql_freeresult(&sqr2);
	if (sql_numtuples(&sqr1)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350 STYLE='border-style:solid'>\r\n<TR>");
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Call ID&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Action&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Date&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Time&nbsp;<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Duration&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Status&nbsp;</TR>\r\n");
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calls/view?callid=%s'\">", sid->dat->in_ScriptName, sql_getvalue(&sqr1, i, 0));
			prints(sid, "&nbsp;<A HREF=%s/calls/view?callid=%s>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(&sqr1, i, 0), sql_getvalue(&sqr1, i, 0));
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD>", htview_callaction(sid, atoi(sql_getvalue(&sqr1, i, 2))));
			calldate=time_sql2unix(sql_getvalue(&sqr1, i, 3));
			calldate+=time_tzoffset(sid, calldate);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, calldate));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2timetext(sid, calldate));
			duration=time_sql2unix(sql_getvalue(&sqr1, i, 4))-time_sql2unix(sql_getvalue(&sqr1, i, 3));
			if (duration<0) duration=0;
			duration/=60;
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%d Minutes</TD>", duration);
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", atoi(sql_getvalue(&sqr1, i, 5))==1?"Closed":"Open");
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
	}
	prints(sid, "<A HREF=%s/calls/editnew?contactid=%d>New Call</A>\r\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
*/
	return;
}

/****************************************************************************
 *	contacteventlist()
 *
 *	Purpose	: Display a list of events for a given contact
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void contacteventlist(CONN *sid)
{
/*
	char *priority[5]={ "Lowest", "Low", "Normal", "High", "Highest" };
	char *ptemp;
	int contactid=0;
	int i;
	SQLRES sqr1;
	SQLRES sqr2;

	prints(sid, "<BR />\r\n");
	if (!module_exists("mod_calendar")) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) return;
	contactid=atoi(ptemp);
	if (contactid<1) {
		prints(sid, "<CENTER>No matching events found</CENTER>\r\n");
		return;
	}
	prints(sid, "<CENTER>\r\n");
	if (sql_queryf(&sqr1, "SELECT eventid, eventname, eventstart, status, priority, contactid FROM gw_events WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart DESC", contactid, sid->dat->uid, sid->dat->uid, sid->dat->gid)<0) return;
	if (sql_queryf(&sqr2, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid)<0) {
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "Found %d event%s for <A HREF=%s/contacts/view?contactid=%s>%s", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s", sid->dat->in_ScriptName, sql_getvalue(&sqr2, 0, 0), str2html(sid, sql_getvalue(&sqr2, 0, 1)));
	if (strlen(sql_getvalue(&sqr2, 0, 1))&&strlen(sql_getvalue(&sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR />\r\n", str2html(sid, sql_getvalue(&sqr2, 0, 2)));
	sql_freeresult(&sqr2);
	if (sql_numtuples(&sqr1)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350 STYLE='border-style:solid'>\r\n<TR>");
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Event ID&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Event Name&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Date&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Status&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Priority&nbsp;</TH></TR>\r\n");
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calendar/view?eventid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/calendar/view?eventid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)), atoi(sql_getvalue(&sqr1, i, 0)));
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 1)));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(&sqr1, i, 2)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", htview_eventstatus(atoi(sql_getvalue(&sqr1, i, 3))));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", priority[atoi(sql_getvalue(&sqr1, i, 4))]);
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
	}
	prints(sid, "<A HREF=%s/calendar/editnew?contactid=%d>New Event</A>\r\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
*/
	return;
}

/****************************************************************************
 *	contactorderlist()
 *
 *	Purpose	: Display a list of invoices for a given contact
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void contactorderlist(CONN *sid)
{
/*
	char *ptemp;
	int contactid=0;
	int i;
	SQLRES sqr1;
	SQLRES sqr2;

	prints(sid, "<BR />\r\n");
	if (!module_exists("mod_finance")) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if (!(auth_priv(sid, "finance")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) return;
	contactid=atoi(ptemp);
	if (contactid<1) {
		prints(sid, "<CENTER>No matching invoices found</CENTER>\r\n");
		return;
	}
	prints(sid, "<CENTER>\r\n");
	if (sql_queryf(&sqr1, "SELECT invoiceid, invoicedate, paymentdue, paymentreceived from gw_finance_invoices WHERE contactid = %d ORDER BY invoicedate DESC", contactid)<0) return;
	if (sql_queryf(&sqr2, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid)<0) {
		sql_freeresult(&sqr1);
		return;
	}
	prints(sid, "Found %d order%s for <A HREF=%s/contacts/view?contactid=%s>%s", sql_numtuples(&sqr1), sql_numtuples(&sqr1)==1?"":"s", sid->dat->in_ScriptName, sql_getvalue(&sqr2, 0, 0), str2html(sid, sql_getvalue(&sqr2, 0, 1)));
	if (strlen(sql_getvalue(&sqr2, 0, 1))&&strlen(sql_getvalue(&sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR />\r\n", str2html(sid, sql_getvalue(&sqr2, 0, 2)));
	sql_freeresult(&sqr2);
	if (sql_numtuples(&sqr1)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350 STYLE='border-style:solid'>\r\n<TR>");
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Invoice ID&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Order Date&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Amount Due&nbsp;</TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>&nbsp;Amount Received&nbsp;</TH></TR>\r\n");
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			prints(sid, "<TR CLASS=\"FIELDVAL\">");
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/finance/invoices/view?invoiceid=%s'\">", sid->dat->in_ScriptName, sql_getvalue(&sqr1, i, 0));
			prints(sid, "&nbsp;<A HREF=%s/finance/invoices/view?invoiceid=%s>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(&sqr1, i, 0), sql_getvalue(&sqr1, i, 0));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(&sqr1, i, 1)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr1, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(&sqr1, i, 3)));
			prints(sid, "</TR>\r\n");
		}
		prints(sid, "</TABLE>\r\n");
	}
	prints(sid, "<A HREF=%s/finance/invoices/editnew?contactid=%d>New Invoice</A>\r\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\r\n");
	sql_freeresult(&sqr1);
*/
	return;
}

void contactsave(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	obj_t *qobj=NULL, *tobj;
	char *ScriptName=nes_getstr(sid->N, htobj, "SCRIPT_NAME");
	char *ptemp;
	int id, pid;
	int priv=auth_priv(sid, "contacts");
//	char opassword[50];

	prints(sid, "<BR />\r\n");
	if (!(priv&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
	if (strcmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"),"POST")!=0) goto cleanup;
	if ((ptemp=getpostenv(sid, "ID"))==NULL) goto cleanup;
	id=atoi(ptemp);
	if (dbread_contact(sid, 2, id, &qobj)<0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		goto cleanup;
	}
//	snprintf(opassword, sizeof(opassword)-1, "%s", contact.password);
	pid=atoi(sql_getvaluebyname(proc->N, &qobj, 0, "pid"));
//	if (priv&A_ADMIN) {
//		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) contact.obj_uid=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) contact.obj_gid=atoi(ptemp);
//	}
//	if ((priv&A_ADMIN)||(contact.obj_uid==sid->dat->uid)) {
//		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) contact.obj_gperm=atoi(ptemp);
//		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) contact.obj_operm=atoi(ptemp);
//	}
	tobj=nes_getobj(proc->N, qobj, "_rows");
	tobj=nes_getiobj(proc->N, tobj, 0);
	if ((ptemp=getpostenv(sid, "PID"))!=NULL) pid=atoi(ptemp);
	nes_setnum(sid->N, tobj, "pid", pid);
	tobj=nes_settable(proc->N, tobj, "_data");
	if (tobj->val->type!=NT_TABLE) {
		prints(sid, "<BR /><CENTER>blah error 3</CENTER><BR />\r\n");
		goto cleanup;
	}
	if ((ptemp=getpostenv(sid, "USERNAME"))!=NULL)       nes_setstr(sid->N, tobj, "uid", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL)       nes_setstr(sid->N, tobj, "userpassword", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "ENABLED"))!=NULL)        nes_setnum(sid->N, tobj, "enabled", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "GEOZONE"))!=NULL)        nes_setnum(sid->N, tobj, "geozone", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "TIMEZONE"))!=NULL)       nes_setnum(sid->N, tobj, "timezone", atoi(ptemp));
	if ((ptemp=getpostenv(sid, "SURNAME"))!=NULL)        nes_setstr(sid->N, tobj, "sn", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "GIVENNAME"))!=NULL)      nes_setstr(sid->N, tobj, "gn", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "SALUTATION"))!=NULL)     nes_setstr(sid->N, tobj, "personaltitle", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "CONTACTTYPE"))!=NULL)    nes_setstr(sid->N, tobj, "contacttype", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "REFERREDBY"))!=NULL)     nes_setstr(sid->N, tobj, "referredby", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "ALTCONTACT"))!=NULL)     nes_setstr(sid->N, tobj, "altcontact", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "PREFBILLING"))!=NULL)    nes_setstr(sid->N, tobj, "prefbilling", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WEBSITE"))!=NULL)        nes_setstr(sid->N, tobj, "labeleduri", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "EMAIL"))!=NULL)          nes_setstr(sid->N, tobj, "mail", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOMENUMBER"))!=NULL)     nes_setstr(sid->N, tobj, "hometelephonenumber", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WORKNUMBER"))!=NULL)     nes_setstr(sid->N, tobj, "worknumber", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "FAXNUMBER"))!=NULL)      nes_setstr(sid->N, tobj, "facsimiletelephonenumber", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "MOBILENUMBER"))!=NULL)   nes_setstr(sid->N, tobj, "mobiletelephonenumber", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "JOBTITLE"))!=NULL)       nes_setstr(sid->N, tobj, "title", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "ORGANIZATION"))!=NULL)   nes_setstr(sid->N, tobj, "organization", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOMEADDRESS"))!=NULL)    nes_setstr(sid->N, tobj, "homeaddress", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOMELOCALITY"))!=NULL)   nes_setstr(sid->N, tobj, "homelocality", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOMEREGION"))!=NULL)     nes_setstr(sid->N, tobj, "homeregion", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOMECOUNTRY"))!=NULL)    nes_setstr(sid->N, tobj, "homecountry", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "HOMEPOSTALCODE"))!=NULL) nes_setstr(sid->N, tobj, "homepostalcode", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WORKADDRESS"))!=NULL)    nes_setstr(sid->N, tobj, "workaddress", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WORKLOCALITY"))!=NULL)   nes_setstr(sid->N, tobj, "worklocality", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WORKREGION"))!=NULL)     nes_setstr(sid->N, tobj, "workregion", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WORKCOUNTRY"))!=NULL)    nes_setstr(sid->N, tobj, "workcountry", ptemp, strlen(ptemp));
	if ((ptemp=getpostenv(sid, "WORKPOSTALCODE"))!=NULL) nes_setstr(sid->N, tobj, "workpostalcode", ptemp, strlen(ptemp));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(priv&A_DELETE)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (ldir_deleteentry("person", id, sid->dat->did)<0) goto cleanup;
//		if (sql_updatef("DELETE FROM gw_contacts WHERE contactid = %d", id)<0) goto cleanup;
		prints(sid, "<CENTER>Contact %d deleted successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "contacts", id, "delete", "%s - %s deleted contact %d", sid->socket.RemoteAddr, sid->dat->username, id);
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/contacts/list\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName);
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/list\"></NOSCRIPT>\r\n", ScriptName);
		goto cleanup;
	} else if (id==0) {
		if (!(priv&A_INSERT)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Force Save")==0)) goto skipcheck1;
//		if ((strlen(contact.surname)<1)||(strlen(contact.givenname)<1)) {
//			prints(sid, "<CENTER><B>Surname and Given Name should not be left blank.</B></CENTER>\r\n");
//			contactview(sid, &contact);
//			goto cleanup;
//		}
/*
		if (strcmp(config->sql_type, "ODBC")==0) {
			if (sql_queryf(&sqr, "SELECT contactid FROM gw_contacts WHERE surname LIKE '%s' AND givenname LIKE '%s' AND obj_did = %d", contact.surname, contact.givenname, sid->dat->did)<0) return;
		} else {
			if (sql_queryf(&sqr, "SELECT contactid FROM gw_contacts WHERE lower(surname) LIKE lower('%s') AND lower(givenname) LIKE lower('%s') AND obj_did = %d", contact.surname, contact.givenname, sid->dat->did)<0) return;
		}
		if (sql_numtuples(&sqr)>0) {
			prints(sid, "<CENTER><B>This contact appears to be a duplicate of <A HREF=%s/contacts/view?contactid=%d>contact #%d</A>.</B></CENTER>\r\n", ScriptName, atoi(sql_getvalue(&sqr, 0, 0)), atoi(sql_getvalue(&sqr, 0, 0)));
			sql_freeresult(&sqr);
			contactview(sid, &contact);
			goto cleanup;
		}
		sql_freeresult(&sqr);
*/
skipcheck1:
//		snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		if ((id=dbwrite_contact(sid, 0, &qobj))<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Contact %d added successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "contacts", id, "insert", "%s - %s added contact %d", sid->socket.RemoteAddr, sid->dat->username, id);
		if ((ptemp=getpostenv(sid, "CALLSET"))!=NULL) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
			prints(sid, "	if (navigator.appVersion.indexOf(\"MSIE \")>-1) {\r\n");
			prints(sid, "		var newoption=window.opener.document.createElement(\"OPTION\");\r\n");
			prints(sid, "		newoption.text='%s';\r\n", htview_contact(sid, id));
			prints(sid, "		newoption.value=%d;\r\n", id);
			prints(sid, "		if (window.opener.document.calledit) {\r\n");
			prints(sid, "			window.opener.document.calledit.contactid.add(newoption);\r\n");
			prints(sid, "		} else if (window.opener.document.eventedit) {\r\n");
			prints(sid, "			window.opener.document.eventedit.contactid.add(newoption);\r\n");
			prints(sid, "		} else if (window.opener.document.taskedit) {\r\n");
			prints(sid, "			window.opener.document.taskedit.contactid.add(newoption);\r\n");
			prints(sid, "		}\r\n");
			prints(sid, "	} else {\r\n");
			prints(sid, "		if (window.opener.document.calledit) {\r\n");
			prints(sid, "			obj=window.opener.document.forms[\"calledit\"][\"contactid\"];\r\n");
			prints(sid, "		} else if (window.opener.document.eventedit) {\r\n");
			prints(sid, "			obj=window.opener.document.forms[\"eventedit\"][\"contactid\"];\r\n");
			prints(sid, "		} else if (window.opener.document.taskedit) {\r\n");
			prints(sid, "			obj=window.opener.document.forms[\"taskedit\"][\"contactid\"];\r\n");
			prints(sid, "		}\r\n");
			prints(sid, "		text='%s';\r\n", htview_contact(sid, id));
			prints(sid, "		value=%d;\r\n", id);
			prints(sid, "		obj.options[obj.options.length] = new Option(text, value);\r\n");
			prints(sid, "	}\r\n");
			prints(sid, "	if (window.opener.document.calledit) {\r\n");
			prints(sid, "		window.opener.document.calledit.contactid.value=%d;\r\n", id);
			prints(sid, "	} else if (window.opener.document.eventedit) {\r\n");
			prints(sid, "		window.opener.document.eventedit.contactid.value=%d;\r\n", id);
			prints(sid, "	} else if (window.opener.document.taskedit) {\r\n");
			prints(sid, "		window.opener.document.taskedit.contactid.value=%d;\r\n", id);
			prints(sid, "	}\r\n");
			prints(sid, "	window.close();\r\n");
			prints(sid, "// -->\r\n</SCRIPT>\r\n");
			goto cleanup;
		}
	} else {
		if (!(priv&A_MODIFY)) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Force Save")==0)) goto skipcheck2;
//		if ((strlen(contact.surname)<1)||(strlen(contact.givenname)<1)) {
//			prints(sid, "<CENTER><B>Surname and Given Name should not be left blank.</B></CENTER>\r\n");
//			contactview(sid, &contact);
//			goto cleanup;
//		}
skipcheck2:
//		if (strcmp(opassword, contact.password)!=0) {
//			snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
//		}
		if (dbwrite_contact(sid, id, &qobj)<1) {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
			goto cleanup;
		}
		prints(sid, "<CENTER>Contact %d modified successfully</CENTER><BR />\r\n", id);
		db_log_activity(sid, "contacts", id, "modify", "%s - %s modified contact %d", sid->socket.RemoteAddr, sid->dat->username, id);
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nlocation.replace(\"%s/contacts/view?contactid=%d\");\r\n// -->\r\n</SCRIPT>\r\n", ScriptName, id);
	prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\"></NOSCRIPT>\r\n", ScriptName, id);
cleanup:
	if (qobj) sql_freeresult(proc->N, &qobj);
	return;
}

DllExport int mod_main(CONN *sid)
{
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");

	if (strncmp(RequestURI, "/contacts/vcardexport", 21)==0) {
		contacts_vcardexport(sid);
		return 0;
	} else if (strncmp(RequestURI, "/contacts/vcardimport", 21)==0) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		contacts_vcardimport(sid);
		return 0;
	}
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, "contacts");
	if (strncmp(RequestURI, "/contacts/search1", 17)==0) {
		contactsearch1(sid);
	} else if (strncmp(RequestURI, "/contacts/search2", 17)==0) {
		contactsearch2(sid);
	} else if (strncmp(RequestURI, "/contacts/mailview", 18)==0) {
		contact_mailview(sid);
	} else if (strncmp(RequestURI, "/contacts/view", 14)==0) {
		contactview(sid, NULL);
	} else if (strncmp(RequestURI, "/contacts/list", 14)==0) {
		contactlist(sid);
	} else if (strncmp(RequestURI, "/contacts/callslist", 19)==0) {
		contactcallslist(sid);
	} else if (strncmp(RequestURI, "/contacts/eventlist", 19)==0) {
		contacteventlist(sid);
	} else if (strncmp(RequestURI, "/contacts/orderlist", 19)==0) {
		contactorderlist(sid);
	} else if (strncmp(RequestURI, "/contacts/save", 14)==0) {
		contactsave(sid);
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc)
{
	obj_t *modobj, *tobj, *cobj;

	proc=_proc;
	if (mod_import()!=0) return -1;
	modobj=nes_settable(proc->N, &proc->N->g, "GWMODULES");
	tobj=nes_settable(proc->N, modobj, "contacts");
	cobj=nes_setnum(proc->N, tobj, "submenu", 2);
	cobj=nes_setstr(proc->N, tobj, "name",    "CONTACTS",        strlen("CONTACTS"));
	cobj=nes_setstr(proc->N, tobj, "pic",     "contacts.png",    strlen("contacts.png"));
	cobj=nes_setstr(proc->N, tobj, "uri",     "/contacts/list",  strlen("/contacts/list"));
	cobj=nes_setstr(proc->N, tobj, "perm",    "contacts",        strlen("contacts"));
	cobj=nes_setstr(proc->N, tobj, "fn_name", "mod_main",        strlen("mod_main"));
	cobj=nes_setstr(proc->N, tobj, "fn_uri",  "/contacts/",      strlen("/contacts/"));
	cobj=nes_setcfunc(proc->N, tobj, "mod_init", (void *)mod_init);
	cobj=nes_setcfunc(proc->N, tobj, "mod_main", (void *)mod_main);
	cobj=nes_setcfunc(proc->N, tobj, "mod_exit", (void *)mod_exit);
//	if (mod_export_function("mod_contacts", "mod_contacts_read", dbread_contact)!=0) return -1;
//	if (mod_export_function("mod_contacts", "mod_contacts_write", dbwrite_contact)!=0) return -1;
	return 0;
}
