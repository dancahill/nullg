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
#include "mod_stub.h"
#include "mod_contacts.h"

void contactsearch1(CONN *sid)
{
	char *ptemp;
	int i;
	int sqr;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (auth_priv(sid, "contacts")&A_READ) {
		prints(sid, "<FORM METHOD=GET ACTION=%s/contacts/search2 NAME=contactsearch>\n", sid->dat->in_ScriptName);
		prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=3><FONT COLOR=%s>Contact Search Form</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD><SELECT NAME=column>\n", config->colour_editform);
		prints(sid, "<OPTION SELECTED>All Columns\n");
		if ((sqr=sql_query(sid, "SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
		for (i=0;i<sql_numfields(sqr);i++) {
			if (strcmp(sql_getname(sqr, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(sqr, i), "obj_operm")==0) continue;
			if (strcmp(sql_getname(sqr, i), "loginip")==0) continue;
			if (strcmp(sql_getname(sqr, i), "logintime")==0) continue;
			if (strcmp(sql_getname(sqr, i), "logintoken")==0) continue;
			if (strcmp(sql_getname(sqr, i), "password")==0) continue;
			if (strcmp(sql_getname(sqr, i), "enabled")==0) continue;
			if (strcmp(sql_getname(sqr, i), "geozone")==0) continue;
			if (strcmp(sql_getname(sqr, i), "timezone")==0) continue;
			prints(sid, "<OPTION>%s\n", sql_getname(sqr, i));
		}
		sql_freeresult(sqr);
		prints(sid, "</SELECT></TD>\n");
		prints(sid, "<TD><INPUT TYPE=TEXT NAME=string value='%s' SIZE=20></TD>\n", (ptemp=getgetenv(sid, "STRING"))!=NULL?ptemp:"");
		prints(sid, "<TD><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Search'></TD></TR>\n");
		prints(sid, "</FORM>\n");
		prints(sid, "<TR><TD COLSPAN=3>&nbsp;</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.contactsearch.string.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void contactsearch2(CONN *sid)
{
	char query[2048];
	char column[100];
	char string[100];
	char string2[100];
	char *ptemp;
	int offset=0;
	int i;
	int sqr1;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
	if ((sqr1=sql_query(sid, "SELECT * FROM gw_contacts WHERE contactid = 1"))<0) return;
	strcpy(query, "SELECT contactid, surname, givenname, organization, worknumber, homenumber from gw_contacts WHERE (");
	if (strcmp(column, "All Columns")==0) {
		for (i=0;i<sql_numfields(sqr1);i++) {
			if (strcmp(sql_getname(sqr1, i), "obj_ctime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_mtime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_uid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gid")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_gperm")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "obj_operm")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "loginip")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "logintime")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "logintoken")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "password")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "enabled")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "geozone")==0) continue;
			if (strcmp(sql_getname(sqr1, i), "timezone")==0) continue;
			if (strcmp(config->sql_type, "ODBC")==0) {
				strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s' or ", sql_getname(sqr1, i), str2sql(sid, string2));
			} else {
				strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s') or ", sql_getname(sqr1, i), str2sql(sid, string2));
			}
		}
		strncatf(query, sizeof(query)-strlen(query)-1, "contactid like '%s'", str2sql(sid, string2));
	} else {
		if (strcmp(config->sql_type, "ODBC")==0) {
			strncatf(query, sizeof(query)-strlen(query)-1, "%s like '%s'", column, str2sql(sid, string2));
		} else {
			strncatf(query, sizeof(query)-strlen(query)-1, "lower(%s) like lower('%s')", column, str2sql(sid, string2));
		}
	}
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		strncatf(query, sizeof(query)-strlen(query)-1, ") ORDER BY surname, givenname ASC");
	} else {
		strncatf(query, sizeof(query)-strlen(query)-1, ") and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", sid->dat->user_uid, sid->dat->user_gid);
	}
	sql_freeresult(sqr1);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ContactSync(contactid) {\r\n");
	prints(sid, "	if (window.opener.document.calledit) {\r\n");
	prints(sid, "		window.opener.document.calledit.contactid.value=contactid;\r\n");
	prints(sid, "	} else if (window.opener.document.eventedit) {\r\n");
	prints(sid, "		window.opener.document.eventedit.contactid.value=contactid;\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	window.close();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if ((sqr1=sql_query(sid, query))<0) return;
	if (sql_numtuples(sqr1)<1) {
		prints(sid, "<B>Found %d matching contact%s</B></CENTER>\n", sql_numtuples(sqr1), sql_numtuples(sqr1)==1?"":"s");
		sql_freeresult(sqr1);
		return;
	}
	prints(sid, "<B>Found %d matching contact%s</B>\n", sql_numtuples(sqr1), sql_numtuples(sqr1)==1?"":"s");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<FORM METHOD=GET NAME=mailform>\n");
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Contact Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Company Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Work Number&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Home Number&nbsp;</FONT></TH><TH STYLE='border-style:solid'>&nbsp;</TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
	for (i=0;i<sql_numtuples(sqr1);i++) {
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/contacts/view?contactid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), str2html(sid, sql_getvalue(sqr1, i, 1)));
		if (strlen(sql_getvalue(sqr1, i, 1))&&strlen(sql_getvalue(sqr1, i, 2))) prints(sid, ", ");
		prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 2)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 3)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 4)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 5)));
		prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=javascript:ContactSync('%d')>&gt;&gt;</A></TD>", atoi(sql_getvalue(sqr1, i, 0)));
		prints(sid, "</TR>\n");
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr1);
	return;
}

void contactview(CONN *sid, REC_CONTACT *contact)
{
	MOD_NOTES_SUBLIST mod_notes_sublist;
	REC_CONTACT contactrec;
	char *ptemp;
	char *greytoggle;
	char baddr[250];
	char maddr[250];
	int is_editable;
	int is_recycled=0;
	int editperms=0;
	int contactid;
	int sqr;
	int tz1;
	int tz2;
	time_t t;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (contact==NULL) {
		prints(sid, "<BR>\r\n");
		if (strncmp(sid->dat->in_RequestURI, "/contacts/viewnew", 17)==0) {
			contactid=0;
			if (dbread_contact(sid, 2, 0, &contactrec)!=0) {
				prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
				return;
			}
		} else {
			if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) return;
			contactid=atoi(ptemp);
			if (dbread_contact(sid, 1, contactid, &contactrec)!=0) {
				prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", contactid);
				return;
			}
		}
		contact=&contactrec;
	} else {
		is_recycled=1;
		contactid=contact->contactid;
	}
	memset(maddr, 0, sizeof(maddr));
	memset(baddr, 0, sizeof(baddr));
	if (strlen(contact->homeaddress)&&strlen(contact->homelocality)&&strlen(contact->homeregion)&&strlen(contact->homecountry)) {
//		snprintf(maddr, sizeof(maddr)-1, "http://mapblast.com/myblast/map.mb?CMD=GEO&req_action=crmap&AD2_street=%s&AD2=%s&AD3=%s, %s&AD4=%s", contact->mailaddress, contact->mailaddress, contact->maillocality, contact->mailregion, contact->mailcountry);
		snprintf(maddr, sizeof(maddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", contact->homecountry, contact->homeaddress, contact->homelocality, contact->homeregion);
	}
	ptemp=maddr;
	while (*ptemp!=0) {
		if (*ptemp==' ') *ptemp='+';
		ptemp++;
	}
	if (strlen(contact->workaddress)&&strlen(contact->worklocality)&&strlen(contact->workregion)&&strlen(contact->workcountry)) {
//		snprintf(baddr, sizeof(baddr)-1, "http://mapblast.com/myblast/map.mb?CMD=GEO&req_action=crmap&AD2_street=%s&AD2=%s&AD3=%s, %s&AD4=%s", contact->billaddress, contact->billaddress, contact->billlocality, contact->billregion, contact->billcountry);
		snprintf(baddr, sizeof(baddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", contact->workcountry, contact->workaddress, contact->worklocality, contact->workregion);
	}
	ptemp=baddr;
	while (*ptemp!=0) {
		if (*ptemp==' ') *ptemp='+';
		ptemp++;
	}
	greytoggle=" DISABLED";
	is_editable=0;
	if (auth_priv(sid, "contacts")&A_MODIFY) {
		if (auth_priv(sid, "contacts")&A_ADMIN) {
			greytoggle="";
			is_editable=1;
		} else if ((contact->obj_uid==sid->dat->user_uid)||((contact->obj_gid==sid->dat->user_gid)&&(contact->obj_gperm>=2))||(contact->obj_operm>=2)) {
			greytoggle="";
			is_editable=1;
		}
	}
	tz1=time_tzoffset(sid, time(NULL));
	tz2=time_tzoffsetcon(sid, time(NULL), contact->contactid);
	if (contact->contactid<1) tz2=tz1;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "function vcardImport() {\r\n");
	prints(sid, "	window.open('%s/contacts/vcardimport?contactid=%d','vcardimport','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=150');\r\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "}\r\n");
	htscript_showpage(sid, 7);
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	if (contact->contactid>0) {
		prints(sid, "<B>%s%s", str2html(sid, contact->givenname), strlen(contact->givenname)?" ":"");
		prints(sid, "%s%s</B>", str2html(sid, contact->surname), strlen(contact->surname)?" ":"");
	} else {
		prints(sid, "<B>New Contact</B>\n");
	}
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=425>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/contacts/save NAME=contactedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=contactid VALUE='%d'>\n", contact->contactid);
	if (strncmp(sid->dat->in_RequestURI, "/contacts/viewnew2", 18)==0) {
		prints(sid, "<INPUT TYPE=hidden NAME=callset VALUE='1'>\n");
	}
	prints(sid, "<TR><TD ALIGN=LEFT>");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 STYLE='border-style:solid'>\n<TR BGCOLOR=%s>\n", config->colour_fieldname);
	prints(sid, "<TD ID=page1tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(1)>SUMMARY</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page2tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(2)>NAME</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page3tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(3)>HOME</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page4tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(4)>WORK</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page5tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(5)>USER</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page6tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(6)>PERMISSIONS</A>&nbsp;</TD>\n");
	prints(sid, "<TD ID=page7tab NOWRAP STYLE='border-style:solid'>&nbsp;<A HREF=javascript:showpage(7)>OTHER</A>&nbsp;</TD>\n");
	prints(sid, "</TR></TABLE>");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP STYLE='padding:3px'>", config->colour_editform);
	prints(sid, "<HR>\r\n");
	prints(sid, "<DIV ID=page1 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Name&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>", config->colour_editform);
	prints(sid, "%s%s", str2html(sid, contact->salutation), strlen(contact->salutation)?" ":"");
	prints(sid, "%s%s", str2html(sid, contact->givenname), strlen(contact->givenname)?" ":"");
	prints(sid, "%s%s", str2html(sid, contact->surname), strlen(contact->surname)?" ":"");
	if (strlen(contact->username)) {
		prints(sid, " (%s)", str2html(sid, contact->username));
	}
	prints(sid, "</TD>\n");
	if (contact->contactid>0) {
		prints(sid, "<TD ALIGN=right><A HREF=%s/contacts/vcardexport?contactid=%d>vCard</A></TD>\n", sid->dat->in_ScriptName, contact->contactid);
	} else {
		prints(sid, "<TD ALIGN=right>&nbsp;</TD>\n");
	}
	prints(sid, "</TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;E-Mail Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>", config->colour_editform);
	if (strlen(contact->email)==0) {
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (sid->dat->user_maildefault==0) {
		prints(sid, "<A HREF=\"mailto:%s\">%s</A>&nbsp;</TD></TR>\n", contact->email, str2html(sid, contact->email));
	} else {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "<A HREF=\"javascript:MsgTo('");
			prints(sid, "&quot;%s%s%s&quot;", str2html(sid, contact->givenname), strlen(contact->givenname)?" ":"", str2html(sid, contact->surname));
			prints(sid, " <%s>')\">%s</A>&nbsp;</TD></TR>\n", contact->email, str2html(sid, contact->email));
		} else {
			prints(sid, "<A HREF=\"mailwrite?to=%s\">%s</A>&nbsp;</TD></TR>\n", contact->email, str2html(sid, contact->email));
		}
	}
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Home Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_editform, str2html(sid, contact->homenumber));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Mobile Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_editform, str2html(sid, contact->mobilenumber));
	if (strlen(maddr)) {
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Home Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>Map Available</A>&nbsp;</TD></TR>\n", config->colour_editform, maddr);
	}
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Organization&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_editform, str2html(sid, contact->organization));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Job Title&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_editform, str2html(sid, contact->jobtitle));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Work Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_editform, str2html(sid, contact->worknumber));
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;FAX Number&nbsp;</B></TD><TD NOWRAP WIDTH=100%%>%s&nbsp;</TD></TR>\n", config->colour_editform, str2html(sid, contact->faxnumber));
	if (strlen(baddr)) {
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Work Address&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><A HREF=\"%s\" TARGET=_blank>Map Available</A>&nbsp;</TD></TR>\n", config->colour_editform, baddr);
	}
	if (tz1!=tz2) {
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=RIGHT NOWRAP><B>&nbsp;Time&nbsp;</B></TD><TD NOWRAP WIDTH=100%%><SPAN ID=contz></SPAN>&nbsp;</TD></TR>\n", config->colour_editform);
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page2 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Surname           &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname        value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->surname), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Given Name        &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname      value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->givenname), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Salutation        &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=salutation     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->salutation), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Contact Type      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=contacttype    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->contacttype), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Referred By       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=referredby     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->referredby), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Alternate Contact &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=altcontact     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->altcontact), greytoggle);
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page3 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Phone      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->homenumber), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Mobile Number   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=mobilenumber   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->mobilenumber), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;E-mail          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email          value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->email), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Address    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homeaddress    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->homeaddress), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home City       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homelocality   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->homelocality), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Province   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homeregion     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->homeregion), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Country    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homecountry    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->homecountry), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homepostalcode value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->homepostalcode), greytoggle);
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page4 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Organization    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=organization   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->organization), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Job Title       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle       value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->jobtitle), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Phone      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->worknumber), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Fax Number      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber      value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->faxnumber), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Address    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workaddress    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->workaddress), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work City       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worklocality   value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->worklocality), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Province   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workregion     value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->workregion), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Country    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workcountry    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->workcountry), greytoggle);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workpostalcode value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->workpostalcode), greytoggle);
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page5 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if (module_exists(sid, "mod_xmlrpc")) {
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Username          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->username), greytoggle);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Password          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->password), greytoggle);
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Allow Login       &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:255px'%s>\n", config->colour_editform, greytoggle);
		prints(sid, "<OPTION VALUE=0%s>No\n", contact->enabled?"":" SELECTED");
		prints(sid, "<OPTION VALUE=1%s>Yes\n", contact->enabled?" SELECTED":"");
		prints(sid, "</SELECT></TD></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=geozone style='width:255px'%s>\n", config->colour_editform, greytoggle);
	htselect_zone(sid, contact->geozone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Time Zone      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=timezone style='width:255px'%s>\n", config->colour_editform, greytoggle);
	htselect_timezone(sid, contact->timezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Billing Method    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=prefbilling    value=\"%s\" SIZE=45 STYLE='width:255px'%s></TD></TR>\n", config->colour_editform, str2html(sid, contact->prefbilling), greytoggle);
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page6 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if ((contact->obj_uid==sid->dat->user_uid)||(auth_priv(sid, "contacts")&A_ADMIN)) editperms=1;
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:255px'%s>\n", (auth_priv(sid, "contacts")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, contact->obj_uid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:255px'%s>\n", (auth_priv(sid, "contacts")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, contact->obj_gid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\n", contact->obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\n", contact->obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\n", contact->obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\n", contact->obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\n", contact->obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\n", contact->obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<DIV ID=page7 STYLE='display: block'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	if (contact->contactid>0) {
		if (module_exists(sid, "mod_calls")&&(auth_priv(sid, "calls")&A_READ)) {
			if ((sqr=sql_queryf(sid, "SELECT count(callid) FROM gw_calls WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0)", contact->contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
			prints(sid, "<TR BGCOLOR=%s>", config->colour_editform);
			prints(sid, "<TD NOWRAP WIDTH=100%%>%d Calls</TD>\n", atoi(sql_getvalue(sqr, 0, 0)));
			prints(sid, "<TD>");
			if (atoi(sql_getvalue(sqr, 0, 0))>0) {
				prints(sid, "<A HREF=%s/contacts/callslist?contactid=%d>list</A>", sid->dat->in_ScriptName, contact->contactid);
			}
			prints(sid, "&nbsp;</TD>\n");
			prints(sid, "<TD><A HREF=%s/calls/editnew?contactid=%d>new</A></TD>\n", sid->dat->in_ScriptName, contact->contactid);
			prints(sid, "</TR>\n");
			sql_freeresult(sqr);
		}
		if (module_exists(sid, "mod_calendar")&&(auth_priv(sid, "calendar")&A_READ)) {
			if ((sqr=sql_queryf(sid, "SELECT count(eventid) FROM gw_events WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0)", contact->contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
			prints(sid, "<TR BGCOLOR=%s>", config->colour_editform);
			prints(sid, "<TD NOWRAP WIDTH=100%%>%d Events</TD>\n", atoi(sql_getvalue(sqr, 0, 0)));
			prints(sid, "<TD>");
			if (atoi(sql_getvalue(sqr, 0, 0))>0) {
				prints(sid, "<A HREF=%s/contacts/eventlist?contactid=%d>list</A>", sid->dat->in_ScriptName, contact->contactid);
			}
			prints(sid, "&nbsp;</TD>\n");
			prints(sid, "<TD><A HREF=%s/calendar/editnew?contactid=%d>new</A></TD>\n", sid->dat->in_ScriptName, contact->contactid);
			prints(sid, "</TR>\n");
			sql_freeresult(sqr);
		}
		if (module_exists(sid, "mod_orders")&&(auth_priv(sid, "orders")&A_READ)) {
			if ((sqr=sql_queryf(sid, "SELECT count(orderid) FROM gw_orders WHERE contactid = %d", contact->contactid))<0) return;
			prints(sid, "<TR BGCOLOR=%s>", config->colour_editform);
			prints(sid, "<TD NOWRAP WIDTH=100%%>%d Orders</TD>\n", atoi(sql_getvalue(sqr, 0, 0)));
			prints(sid, "<TD>");
			if (atoi(sql_getvalue(sqr, 0, 0))>0) {
				prints(sid, "<A HREF=%s/contacts/orderlist?contactid=%d>list</A>", sid->dat->in_ScriptName, contact->contactid);
			}
			prints(sid, "&nbsp;</TD>\n");
			prints(sid, "<TD><A HREF=%s/orders/editnew?contactid=%d>new</A></TD>\n", sid->dat->in_ScriptName, contact->contactid);
			prints(sid, "</TR>\n");
			sql_freeresult(sqr);
		}
		prints(sid, "<TD COLSPAN=3><BR></TD>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=3><A HREF=javascript:vcardImport();>Import vCard</A></TD></TR>\n", config->colour_editform);
	prints(sid, "</TABLE>\n");
	prints(sid, "</DIV>\r\n");
	prints(sid, "<HR>\r\n");
	prints(sid, "</TD></TR>\n");
	if (contact->contactid>0) {
		if ((mod_notes_sublist=module_call(sid, "mod_notes_sublist"))!=NULL) {
			prints(sid, "<TR><TD NOWRAP>");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'>\r\n");
			prints(sid, "<TR BGCOLOR=%s><TH NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>Notes", config->colour_th, config->colour_thtext);
			prints(sid, " [<A HREF=%s/notes/editnew?table=contacts&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, contact->contactid, config->colour_thlink);
			prints(sid, "</FONT></TH></TR>\n");
			mod_notes_sublist(sid, "contacts", contact->contactid, 1);
			prints(sid, "</TABLE>\n");
			prints(sid, "</TD></TR>\n");
		}
	}
	if (is_editable) {
		prints(sid, "<TR><TD ALIGN=CENTER>\n");
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
		if (is_recycled) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Force Save'>\n");
		}
		if ((auth_priv(sid, "contacts")&A_DELETE)&&(contact->contactid!=0)) {
			prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
		}
		prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	if ((contact->contactid<1)||(is_recycled)) {
		prints(sid, "showpage(2);\n");
		prints(sid, "document.contactedit.surname.focus();\n");
	} else {
		prints(sid, "showpage(1);\n");
	}
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

void contactlist(CONN *sid)
{
	char *ptemp;
	char searchstring[10];
	int offset=0;
	int i;
	int sqr1;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
	if (sid->dat->user_maildefault==0) {
		prints(sid, "	var output=\"mailto:\";\r\n");
	} else {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "	var output=\"javascript:MailTo('\";\r\n");
		} else {
			prints(sid, "	var output=\"%s/mail/write?\";\r\n", sid->dat->in_ScriptName);
		}
	}
	prints(sid, "	var cc=0;\r\n");
	prints(sid, "	var j=0;\r\n\r\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "	output+='to=';\r\n");
	}
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='TO') {\n");
	prints(sid, "			if (j>0) output+='%s';\r\n", (sid->dat->user_maildefault>0)?",":";");
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='CC') {\n");
	prints(sid, "			if (j==0) output+='%scc=';\r\n", (sid->dat->user_maildefault>0)?"&":"?");
	prints(sid, "			else if (j>0) output+='%s';\r\n", (sid->dat->user_maildefault>0)?",":";");
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			cc=1;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (document.mailform[\"option\"+i].value=='BCC') {\n");
	prints(sid, "			if (j==0) {\r\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "				if (j==0) output+='&bcc=';\r\n");
	} else {
		prints(sid, "				if (cc==1) {\r\n");
		prints(sid, "					output+='&bcc=';\r\n");
		prints(sid, "				} else {\r\n");
		prints(sid, "					output+='?bcc=';\r\n");
		prints(sid, "				}\r\n");
	}
	prints(sid, "			} else if (j>0) output+='%s';\r\n", (sid->dat->user_maildefault>0)?",":";");
	prints(sid, "			output+=document.mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	if ((sid->dat->user_maildefault>0)&&(sid->dat->user_menustyle>0)) {
		prints(sid, "	output+=\"')\";\r\n");
	}
	prints(sid, "	document.links[mailtoLink2].href=output;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function MailTo(rcptlist) {\r\n");
	prints(sid, "	window.open('%s/mail/write?'+rcptlist,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=455');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "//--></SCRIPT>\r\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD ALIGN=CENTER VALIGN=TOP WIDTH=100%%>\n");
	if ((ptemp=getgetenv(sid, "S"))!=NULL) {
		strncpy(searchstring, getgetenv(sid, "S"), sizeof(searchstring)-1);
	} else {
		strncpy(searchstring, "All", sizeof(searchstring)-1);
	}
	if (strcasecmp(searchstring, "All")==0) {
		if (auth_priv(sid, "contacts")&A_ADMIN) {
			if ((sqr1=sql_queryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts ORDER BY surname, givenname ASC", searchstring))<0) return;
		} else {
			if ((sqr1=sql_queryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
	} else {
		if (auth_priv(sid, "contacts")&A_ADMIN) {
			if (strcmp(config->sql_type, "ODBC")==0) {
				if ((sqr1=sql_queryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (surname like '%s%%') ORDER BY surname, givenname ASC", searchstring))<0) return;
			} else {
				if ((sqr1=sql_queryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (lower(surname) like lower('%s%%')) ORDER BY surname, givenname ASC", searchstring))<0) return;
			}
		} else {
			if (strcmp(config->sql_type, "ODBC")==0) {
				if ((sqr1=sql_queryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (surname like '%s%%')  and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", searchstring, sid->dat->user_uid, sid->dat->user_gid))<0) return;
			} else {
				if ((sqr1=sql_queryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (lower(surname) like lower('%s%%'))  and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", searchstring, sid->dat->user_uid, sid->dat->user_gid))<0) return;
			}
		}
	}
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) offset=atoi(ptemp);
	prints(sid, "<CENTER><BR>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\n");
	prints(sid, "<TD ALIGN=LEFT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "<TD ALIGN=CENTER NOWRAP><B>Found %d matching contact%s</B></TD>\n", sql_numtuples(sqr1), sql_numtuples(sqr1)==1?"":"s");
	prints(sid, "<TD ALIGN=RIGHT NOWRAP WIDTH=150>&nbsp;</TD>\n");
	prints(sid, "</TR>\n");
	if (sql_numtuples(sqr1)>0) {
		prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3>\n");
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=600 STYLE='border-style:solid'>\r\n");
		prints(sid, "<FORM METHOD=GET NAME=mailform>\n");
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP WIDTH=100%% STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Contact Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Company Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Work Number&nbsp;</FONT></TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;E-Mail&nbsp;</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=offset;(i<sql_numtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/contacts/view?contactid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)));
			prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr1, i, 0)), str2html(sid, sql_getvalue(sqr1, i, 1)));
			if (strlen(sql_getvalue(sqr1, i, 1))&&strlen(sql_getvalue(sqr1, i, 2))) prints(sid, ", ");
			if (strlen(sql_getvalue(sqr1, i, 1))==0&&strlen(sql_getvalue(sqr1, i, 2))==0) prints(sid, "&nbsp;");
			prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 2)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 3)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr1, i, 4)));
			if (strlen(sql_getvalue(sqr1, i, 5))==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
			} else if (sid->dat->user_maildefault==0) {
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=\"mailto:%s\">%s</A>&nbsp;</TD>", sql_getvalue(sqr1, i, 5), sql_getvalue(sqr1, i, 5));
			} else {
				if (sid->dat->user_menustyle>0) {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=\"javascript:MsgTo('%s')\">%s</A>&nbsp;</TD>", sql_getvalue(sqr1, i, 5), str2html(sid, sql_getvalue(sqr1, i, 5)));
				} else {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=\"%s/mail/write?to=%s\">%s</A>&nbsp;</TD>", sid->dat->in_ScriptName, sql_getvalue(sqr1, i, 5), str2html(sid, sql_getvalue(sqr1, i, 5)));
				}
			}
			prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", i-offset, sql_getvalue(sqr1, i, 5));
			prints(sid, "<TD NOWRAP STYLE='padding:0px; border-style:solid''><SELECT NAME=option%d onchange=MailUpdate(); STYLE='font-size:11px; width:44px'>", i-offset);
			prints(sid, "<OPTION VALUE=''>");
			if (strchr(sql_getvalue(sqr1, i, 5), '@')) {
				prints(sid, "<OPTION VALUE='TO'>TO");
				prints(sid, "<OPTION VALUE='CC'>CC");
				prints(sid, "<OPTION VALUE='BCC'>BCC");
			}
			prints(sid, "</SELECT></TD></TR>\n");
		}
		prints(sid, "</FORM>\n");
		prints(sid, "</TABLE>\n</TD></TR>\n");
		prints(sid, "<TR>\n<TD ALIGN=LEFT NOWRAP WIDTH=150>&nbsp;</TD>\n");
		prints(sid, "<TD ALIGN=CENTER NOWRAP>");
		if (sql_numtuples(sqr1)>sid->dat->user_maxlist) {
			if (offset>sid->dat->user_maxlist-1) {
				prints(sid, "[<A HREF=%s/contacts/list?s=%s&offset=%d>Previous Page</A>]", sid->dat->in_ScriptName, searchstring, offset-sid->dat->user_maxlist);
			} else {
				prints(sid, "[Previous Page]");
			}
			if (offset+sid->dat->user_maxlist<sql_numtuples(sqr1)) {
				prints(sid, "[<A HREF=%s/contacts/list?s=%s&offset=%d>Next Page</A>]", sid->dat->in_ScriptName, searchstring, offset+sid->dat->user_maxlist);
			} else {
				prints(sid, "[Next Page]");
			}
		}
		prints(sid, "</TD>\n<TD ALIGN=RIGHT NOWRAP><A HREF=\"mailto:list\" onClick=\"MailUpdate()\">Send E-Mail</A></TD>\n</TR>");
		prints(sid, "</TABLE>\n");
		prints(sid, "<SCRIPT TYPE=\"text/javascript\" LANGUAGE=\"JavaScript\"><!--\n");
		prints(sid, "var mailtoLink2 = findMailtoLink('mailto:list');\n");
		prints(sid, "//--></SCRIPT>\n");
	} else {
		prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3 WIDTH=100%%>&nbsp;</TD></TR>\n");
		prints(sid, "</TABLE>\n");
	}
	sql_freeresult(sqr1);
	prints(sid, "</TD><TD BGCOLOR=%s VALIGN=TOP>\n", config->colour_fieldname);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><FONT SIZE=2><B><A HREF=%s/contacts/list?s=all>&nbsp;All&nbsp;</A></B></FONT></TD></TR>\n", config->colour_fieldname, sid->dat->in_ScriptName);
	for (i=65;i<91;i++) {
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><FONT SIZE=2><B><A HREF=%s/contacts/list?s=%c>&nbsp;%c&nbsp;</A></B></FONT></TD></TR>\n", config->colour_fieldname, sid->dat->in_ScriptName, i, i);
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "</TD></TR></TABLE>\n");
	prints(sid, "</CENTER>\n");
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
	int contactid=0;
	int i;
	int sqr;
	int sqr2;
	time_t calldate;
	int duration;

	prints(sid, "<BR>\r\n");
	if (!module_exists(sid, "mod_calls")) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (!(auth_priv(sid, "calls")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	contactid=atoi(getgetenv(sid, "CONTACTID"));
	if (contactid<1) {
		prints(sid, "<CENTER>No matching events found</CENTER>\n");
		return;
	}
	prints(sid, "<CENTER>\n");
	if ((sqr=sql_queryf(sid, "SELECT callid, contactid, action, callstart, callfinish, status FROM gw_calls WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY callstart DESC", contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "Found %d call%s for <A HREF=%s/contacts/view?contactid=%s>%s", sql_numtuples(sqr), sql_numtuples(sqr)==1?"":"s", sid->dat->in_ScriptName, sql_getvalue(sqr2, 0, 0), str2html(sid, sql_getvalue(sqr2, 0, 1)));
	if (strlen(sql_getvalue(sqr2, 0, 1))&&strlen(sql_getvalue(sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR>\n", str2html(sid, sql_getvalue(sqr2, 0, 2)));
	sql_freeresult(sqr2);
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s>", config->colour_th);
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Call ID&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Action&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Time&nbsp;</FONT><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Duration&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Status&nbsp;</FONT></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calls/view?callid=%s'\">", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
			prints(sid, "&nbsp;<A HREF=%s/calls/view?callid=%s>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0), sql_getvalue(sqr, i, 0));
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD>", htview_callaction(sid, atoi(sql_getvalue(sqr, i, 2))));
			calldate=time_sql2unix(sql_getvalue(sqr, i, 3));
			calldate+=time_tzoffset(sid, calldate);
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2datetext(sid, calldate));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_unix2timetext(sid, calldate));
			duration=time_sql2unix(sql_getvalue(sqr, i, 4))-time_sql2unix(sql_getvalue(sqr, i, 3));
			if (duration<0) duration=0;
			duration/=60;
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%d Minutes</TD>", duration);
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s</TD>", atoi(sql_getvalue(sqr, i, 5))==1?"Closed":"Open");
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/calls/editnew?contactid=%d>New Call</A>\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
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
	char *priority[5]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int contactid=0;
	int i;
	int sqr;
	int sqr2;

	prints(sid, "<BR>\r\n");
	if (!module_exists(sid, "mod_calendar")) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (!(auth_priv(sid, "calendar")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	contactid=atoi(getgetenv(sid, "CONTACTID"));
	if (contactid<1) {
		prints(sid, "<CENTER>No matching events found</CENTER>\n");
		return;
	}
	prints(sid, "<CENTER>\n");
	if ((sqr=sql_queryf(sid, "SELECT eventid, eventname, eventstart, status, priority, contactid FROM gw_events WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart DESC", contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "Found %d event%s for <A HREF=%s/contacts/view?contactid=%s>%s", sql_numtuples(sqr), sql_numtuples(sqr)==1?"":"s", sid->dat->in_ScriptName, sql_getvalue(sqr2, 0, 0), str2html(sid, sql_getvalue(sqr2, 0, 1)));
	if (strlen(sql_getvalue(sqr2, 0, 1))&&strlen(sql_getvalue(sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR>\n", str2html(sid, sql_getvalue(sqr2, 0, 2)));
	sql_freeresult(sqr2);
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s>", config->colour_th);
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Event ID&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Event Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Date&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Status&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Priority&nbsp;</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/calendar/view?eventid=%d'\">", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/calendar/view?eventid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)), atoi(sql_getvalue(sqr, i, 0)));
			prints(sid, "<TD NOWRAP WIDTH=100%% STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, 1)));
			prints(sid, "<TD ALIGN=right NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(sqr, i, 2)));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", htview_eventstatus(sid, atoi(sql_getvalue(sqr, i, 3))));
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", priority[atoi(sql_getvalue(sqr, i, 4))]);
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/calendar/editnew?contactid=%d>New Event</A>\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

/****************************************************************************
 *	contactorderlist()
 *
 *	Purpose	: Display a list of orders for a given contact
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void contactorderlist(CONN *sid)
{
	int contactid=0;
	int i;
	int sqr;
	int sqr2;

	prints(sid, "<BR>\r\n");
	if (!module_exists(sid, "mod_orders")) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (!(auth_priv(sid, "orders")&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	contactid=atoi(getgetenv(sid, "CONTACTID"));
	if (contactid<1) {
		prints(sid, "<CENTER>No matching orders found</CENTER>\n");
		return;
	}
	prints(sid, "<CENTER>\n");
	if ((sqr=sql_queryf(sid, "SELECT orderid, orderdate, paymentdue, paymentreceived from gw_orders WHERE contactid = %d ORDER BY orderdate DESC", contactid))<0) return;
	if ((sqr2=sql_queryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "Found %d order%s for <A HREF=%s/contacts/view?contactid=%s>%s", sql_numtuples(sqr), sql_numtuples(sqr)==1?"":"s", sid->dat->in_ScriptName, sql_getvalue(sqr2, 0, 0), str2html(sid, sql_getvalue(sqr2, 0, 1)));
	if (strlen(sql_getvalue(sqr2, 0, 1))&&strlen(sql_getvalue(sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR>\n", str2html(sid, sql_getvalue(sqr2, 0, 2)));
	sql_freeresult(sqr2);
	if (sql_numtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350 STYLE='border-style:solid'>\r\n<TR BGCOLOR=%s>", config->colour_th);
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Order ID&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Order Date&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Amount Due&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'><FONT COLOR=%s>&nbsp;Amount Received&nbsp;</FONT></TH></TR>\n", config->colour_thtext, config->colour_thtext, config->colour_thtext, config->colour_thtext);
		for (i=0;i<sql_numtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style='cursor:hand; border-style:solid' onClick=\"window.location.href='%s/orders/view?orderid=%s'\">", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
			prints(sid, "&nbsp;<A HREF=%s/orders/view?orderid=%s>%s</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0), sql_getvalue(sqr, i, 0));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", time_sql2datetext(sid, sql_getvalue(sqr, i, 1)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP STYLE='border-style:solid'>$%1.2f&nbsp;</TD>", atof(sql_getvalue(sqr, i, 3)));
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/orders/editnew?contactid=%d>New Order</A>\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
	return;
}

void contactsave(CONN *sid)
{
	REC_CONTACT contact;
	char opassword[50];
	char *ptemp;
	int contactid;
	int sqr;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, "contacts")&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "CONTACTID"))==NULL) return;
	contactid=atoi(ptemp);
	if (dbread_contact(sid, 2, contactid, &contact)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", contact.password);
	if (auth_priv(sid, "contacts")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) contact.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) contact.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "contacts")&A_ADMIN)||(contact.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) contact.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) contact.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "USERNAME"))!=NULL) snprintf(contact.username, sizeof(contact.username)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PASSWORD"))!=NULL) snprintf(contact.password, sizeof(contact.password)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ENABLED"))!=NULL) contact.enabled=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "GEOZONE"))!=NULL) contact.geozone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "TIMEZONE"))!=NULL) contact.timezone=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "SURNAME"))!=NULL) snprintf(contact.surname, sizeof(contact.surname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "GIVENNAME"))!=NULL) snprintf(contact.givenname, sizeof(contact.givenname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "SALUTATION"))!=NULL) snprintf(contact.salutation, sizeof(contact.salutation)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "CONTACTTYPE"))!=NULL) snprintf(contact.contacttype, sizeof(contact.contacttype)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "REFERREDBY"))!=NULL) snprintf(contact.referredby, sizeof(contact.referredby)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ALTCONTACT"))!=NULL) snprintf(contact.altcontact, sizeof(contact.altcontact)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "PREFBILLING"))!=NULL) snprintf(contact.prefbilling, sizeof(contact.prefbilling)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "EMAIL"))!=NULL) snprintf(contact.email, sizeof(contact.email)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMENUMBER"))!=NULL) snprintf(contact.homenumber, sizeof(contact.homenumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKNUMBER"))!=NULL) snprintf(contact.worknumber, sizeof(contact.worknumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "FAXNUMBER"))!=NULL) snprintf(contact.faxnumber, sizeof(contact.faxnumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "MOBILENUMBER"))!=NULL) snprintf(contact.mobilenumber, sizeof(contact.mobilenumber)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "JOBTITLE"))!=NULL) snprintf(contact.jobtitle, sizeof(contact.jobtitle)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ORGANIZATION"))!=NULL) snprintf(contact.organization, sizeof(contact.organization)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMEADDRESS"))!=NULL) snprintf(contact.homeaddress, sizeof(contact.homeaddress)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMELOCALITY"))!=NULL) snprintf(contact.homelocality, sizeof(contact.homelocality)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMEREGION"))!=NULL) snprintf(contact.homeregion, sizeof(contact.homeregion)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMECOUNTRY"))!=NULL) snprintf(contact.homecountry, sizeof(contact.homecountry)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "HOMEPOSTALCODE"))!=NULL) snprintf(contact.homepostalcode, sizeof(contact.homepostalcode)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKADDRESS"))!=NULL) snprintf(contact.workaddress, sizeof(contact.workaddress)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKLOCALITY"))!=NULL) snprintf(contact.worklocality, sizeof(contact.worklocality)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKREGION"))!=NULL) snprintf(contact.workregion, sizeof(contact.workregion)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKCOUNTRY"))!=NULL) snprintf(contact.workcountry, sizeof(contact.workcountry)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "WORKPOSTALCODE"))!=NULL) snprintf(contact.workpostalcode, sizeof(contact.workpostalcode)-1, "%s", ptemp);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "contacts")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef(sid, "DELETE FROM gw_contacts WHERE contactid = %d", contact.contactid)<0) return;
		prints(sid, "<CENTER>Contact %d deleted successfully</CENTER><BR>\n", contactid);
		db_log_activity(sid, 1, "contacts", contact.contactid, "delete", "%s - %s deleted contact %d", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/list\">\n", sid->dat->in_ScriptName);
	} else if (contact.contactid==0) {
		if (!(auth_priv(sid, "contacts")&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Force Save")==0)) goto skipcheck1;
		if ((strlen(contact.surname)<1)||(strlen(contact.givenname)<1)) {
			prints(sid, "<CENTER><B>Surname and Given Name should not be left blank.</B></CENTER>\n");
			contactview(sid, &contact);
			return;
		}
		if (strcmp(config->sql_type, "ODBC")==0) {
			if ((sqr=sql_queryf(sid, "SELECT contactid FROM gw_contacts WHERE surname LIKE '%s' AND givenname LIKE '%s'", contact.surname, contact.givenname))<0) return;
		} else {
			if ((sqr=sql_queryf(sid, "SELECT contactid FROM gw_contacts WHERE lower(surname) LIKE lower('%s') AND lower(givenname) LIKE lower('%s')", contact.surname, contact.givenname))<0) return;
		}
		if (sql_numtuples(sqr)>0) {
			prints(sid, "<CENTER><B>This contact appears to be a duplicate of <A HREF=%s/contacts/view?contactid=%d>contact #%d</A>.</B></CENTER>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, 0, 0)), atoi(sql_getvalue(sqr, 0, 0)));
			sql_freeresult(sqr);
			contactview(sid, &contact);
			return;
		}
		sql_freeresult(sqr);
skipcheck1:
		snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		if ((contact.contactid=dbwrite_contact(sid, 0, &contact))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Contact %d added successfully</CENTER><BR>\n", contact.contactid);
		db_log_activity(sid, 1, "contacts", contact.contactid, "insert", "%s - %s added contact %d", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
		if ((ptemp=getpostenv(sid, "CALLSET"))!=NULL) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
			prints(sid, "	if (navigator.appVersion.indexOf(\"MSIE \")>-1) {\r\n");
			prints(sid, "		var newoption=window.opener.document.createElement(\"OPTION\");\r\n");
			prints(sid, "		newoption.text='%s';\r\n", htview_contact(sid, contact.contactid));
			prints(sid, "		newoption.value=%d;\r\n", contact.contactid);
			prints(sid, "		if (window.opener.document.calledit) {\r\n");
			prints(sid, "			window.opener.document.calledit.contactid.add(newoption);\r\n");
			prints(sid, "		} else if (window.opener.document.eventedit) {\r\n");
			prints(sid, "			window.opener.document.eventedit.contactid.add(newoption);\r\n");
			prints(sid, "		}\r\n");
			prints(sid, "	} else {\r\n");
			prints(sid, "		if (window.opener.document.calledit) {\r\n");
			prints(sid, "			obj=window.opener.document.forms[\"calledit\"][\"contactid\"];\r\n");
			prints(sid, "		} else if (window.opener.document.eventedit) {\r\n");
			prints(sid, "			obj=window.opener.document.forms[\"eventedit\"][\"contactid\"];\r\n");
			prints(sid, "		}\r\n");
			prints(sid, "		text='%s';\r\n", htview_contact(sid, contact.contactid));
			prints(sid, "		value=%d;\r\n", contact.contactid);
			prints(sid, "		obj.options[obj.options.length] = new Option(text, value);\r\n");
			prints(sid, "	}\r\n");
			prints(sid, "	if (window.opener.document.calledit) {\r\n");
			prints(sid, "		window.opener.document.calledit.contactid.value=%d;\r\n", contact.contactid);
			prints(sid, "	} else if (window.opener.document.eventedit) {\r\n");
			prints(sid, "		window.opener.document.eventedit.contactid.value=%d;\r\n", contact.contactid);
			prints(sid, "	}\r\n");
			prints(sid, "	window.close();\r\n");
			prints(sid, "// -->\n</SCRIPT>\n");
		} else {
			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\">\n", sid->dat->in_ScriptName, contact.contactid);
		}
	} else {
		if (!(auth_priv(sid, "contacts")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Force Save")==0)) goto skipcheck2;
		if ((strlen(contact.surname)<1)||(strlen(contact.givenname)<1)) {
			prints(sid, "<CENTER><B>Surname and Given Name should not be left blank.</B></CENTER>\n");
			contactview(sid, &contact);
			return;
		}
skipcheck2:
		if (strcmp(opassword, contact.password)!=0) {
			snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		}
		if (dbwrite_contact(sid, contactid, &contact)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Contact %d modified successfully</CENTER><BR>\n", contact.contactid);
		db_log_activity(sid, 1, "contacts", contact.contactid, "modify", "%s - %s modified contact %d", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\">\n", sid->dat->in_ScriptName, contact.contactid);
	}
	return;
}

void mod_main(CONN *sid)
{
	if (strncmp(sid->dat->in_RequestURI, "/contacts/vcardexport", 21)==0) {
		contacts_vcardexport(sid);
		return;
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/vcardimport", 21)==0) {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		contacts_vcardimport(sid);
		return;
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_CONTACTS);
	if (strncmp(sid->dat->in_RequestURI, "/contacts/search1", 17)==0) {
		contactsearch1(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/search2", 17)==0) {
		contactsearch2(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/view", 14)==0) {
		contactview(sid, NULL);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/list", 14)==0) {
		contactlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/callslist", 19)==0) {
		contactcallslist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/eventlist", 19)==0) {
		contacteventlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/orderlist", 19)==0) {
		contactorderlist(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/save", 14)==0) {
		contactsave(sid);
	}
	htpage_footer(sid);
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod;

	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	memset((char *)&newmod, 0, sizeof(newmod));
	newmod.mod_submenu=2;
	snprintf(newmod.mod_name,     sizeof(newmod.mod_name)-1,     "mod_contacts");
	snprintf(newmod.mod_menuname, sizeof(newmod.mod_menuname)-1, "CONTACTS");
	snprintf(newmod.mod_menuperm, sizeof(newmod.mod_menuperm)-1, "contacts");
	snprintf(newmod.mod_menuuri,  sizeof(newmod.mod_menuuri)-1,  "/contacts/list");
	snprintf(newmod.fn_name,      sizeof(newmod.fn_name)-1,      "mod_main");
	snprintf(newmod.fn_uri,       sizeof(newmod.fn_uri)-1,       "/contacts/");
	newmod.fn_ptr=mod_main;
	if (mod_export_main(&newmod)!=0) return -1;
	if (mod_export_function("mod_contacts", "mod_contacts_read", dbread_contact)!=0) return -1;
	if (mod_export_function("mod_contacts", "mod_contacts_write", dbwrite_contact)!=0) return -1;
	return 0;
}
