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
#include "main.h"

void contactedit(CONNECTION *sid)
{
	REC_CONTACT contact;
	int contactid;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_CONTACTS)&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/contacts/editnew")==0) {
		contactid=0;
		db_read(sid, 2, DB_CONTACTS, 0, &contact);
	} else {
		if (getgetenv(sid, "CONTACTID")==NULL) return;
		contactid=atoi(getgetenv(sid, "CONTACTID"));
		if (db_read(sid, 2, DB_CONTACTS, contactid, &contact)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", contactid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/contacts/save NAME=contactedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=contactid VALUE='%d'>\n", contact.contactid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4><FONT COLOR=%s>", COLOR_TH, COLOR_THTEXT);
	if (contactid>0) {
		prints(sid, "<A HREF=%s/contacts/view?contactid=%d STYLE='color: %s'>Contact %d</FONT></TH></TR>\n", sid->dat->in_ScriptName, contactid, COLOR_THTEXT, contactid);
	} else {
		prints(sid, "New Contact</FONT></TH></TR>\n");
	}
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Username       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=username value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.username));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Password       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=PASSWORD NAME=password value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.password));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Allow Login    &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=enabled style='width:182px'>\n", COLOR_EDITFORM);
	if (contact.enabled) {
		prints(sid, "<OPTION VALUE=0>No\n<OPTION VALUE=1 SELECTED>Yes\n");
	} else {
		prints(sid, "<OPTION VALUE=0 SELECTED>No\n<OPTION VALUE=1>Yes\n");
	}
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Geographic Zone&nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=geozone style='width:182px'>\n", COLOR_EDITFORM);
	htselect_zone(sid, contact.geozone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Time Zone      &nbsp;</B></TD><TD ALIGN=RIGHT><SELECT NAME=timezone style='width:182px'>\n", COLOR_EDITFORM);
	htselect_timezone(sid, contact.timezone);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "</TABLE>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4><FONT COLOR=%s>Contact Information</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Surname            &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=surname        value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.surname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Given Name         &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=givenname      value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.givenname));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Salutation         &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=salutation     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.salutation));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Contact Type       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=contacttype    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.contacttype));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Referred By        &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=referredby     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.referredby));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Alternate Contact  &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=altcontact     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.altcontact));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Billing Method     &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=prefbilling    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.prefbilling));
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Organization       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=organization   value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.organization));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Job Title          &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=jobtitle       value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.jobtitle));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Phone         &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worknumber     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.worknumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Phone         &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homenumber     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.homenumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Fax Number         &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=faxnumber      value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.faxnumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Mobile Number      &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=mobilenumber   value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.mobilenumber));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;E-mail             &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=email          value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.email));
	prints(sid, "</TABLE>\n</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4><FONT COLOR=%s>Address Information</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR BGCOLOR=%s><TD VALIGN=TOP>\n", COLOR_EDITFORM);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Address    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homeaddress    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.homeaddress));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home City       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homelocality   value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.homelocality));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Province   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homeregion     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.homeregion));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Country    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homecountry    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.homecountry));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Home Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=homepostalcode value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.homepostalcode));
	prints(sid, "</TABLE></TD><TD VALIGN=TOP><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Address    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workaddress    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.workaddress));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work City       &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=worklocality   value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.worklocality));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Province   &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workregion     value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.workregion));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Country    &nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workcountry    value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.workcountry));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP><B>&nbsp;Work Postal Code&nbsp;</B></TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=workpostalcode value=\"%s\" SIZE=25 style='width:182px'></TD></TR>\n", COLOR_EDITFORM, str2html(sid, contact.workpostalcode));
	prints(sid, "</TABLE>\n</TD></TR>\n");
	if ((contact.obj_uid==sid->dat->user_uid)||(auth_priv(sid, AUTH_CONTACTS)&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CONTACTS)&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, contact.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", COLOR_EDITFORM);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, AUTH_CONTACTS)&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, contact.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", contact.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", contact.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", contact.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", COLOR_EDITFORM);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", contact.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", contact.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", contact.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, AUTH_CONTACTS)&A_DELETE)&&(contactid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.contactedit.username.focus();\n// -->\n</SCRIPT>\n");
	return;
}

void contactview(CONNECTION *sid)
{
	REC_CONTACT contact;
	char baddr[250];
	char maddr[250];
	char *ptemp;
	int contactid;
	int sqr;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_CONTACTS)&A_READ)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "CONTACTID")==NULL) return;
	contactid=atoi(getgetenv(sid, "CONTACTID"));
	if (db_read(sid, 1, DB_CONTACTS, contactid, &contact)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", contactid);
		return;
	}
	memset(maddr, 0, sizeof(maddr));
	memset(baddr, 0, sizeof(baddr));
	if (strlen(contact.homeaddress)&&strlen(contact.homelocality)&&strlen(contact.homeregion)&&strlen(contact.homecountry)) {
//		snprintf(maddr, sizeof(maddr)-1, "http://mapblast.com/myblast/map.mb?CMD=GEO&req_action=crmap&AD2_street=%s&AD2=%s&AD3=%s, %s&AD4=%s", contact.mailaddress, contact.mailaddress, contact.maillocality, contact.mailregion, contact.mailcountry);
		snprintf(maddr, sizeof(maddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", contact.homecountry, contact.homeaddress, contact.homelocality, contact.homeregion);
	}
	ptemp=maddr;
	while (*ptemp!=0) {
		if (*ptemp==' ') *ptemp='+';
		ptemp++;
	}
	if (strlen(contact.workaddress)&&strlen(contact.worklocality)&&strlen(contact.workregion)&&strlen(contact.workcountry)) {
//		snprintf(baddr, sizeof(baddr)-1, "http://mapblast.com/myblast/map.mb?CMD=GEO&req_action=crmap&AD2_street=%s&AD2=%s&AD3=%s, %s&AD4=%s", contact.billaddress, contact.billaddress, contact.billlocality, contact.billregion, contact.billcountry);
		snprintf(baddr, sizeof(baddr)-1, "http://www.mapquest.com/maps/map.adp?country=%s&address=%s&city=%s&state=%s", contact.workcountry, contact.workaddress, contact.worklocality, contact.workregion);
	}
	ptemp=baddr;
	while (*ptemp!=0) {
		if (*ptemp==' ') *ptemp='+';
		ptemp++;
	}
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
//	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>Contact %d", COLOR_TH, COLOR_THTEXT, contactid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>%s%s", COLOR_TH, COLOR_THTEXT, str2html(sid, contact.salutation), strlen(contact.salutation)?" ":"");
	prints(sid, "%s%s", str2html(sid, contact.givenname), strlen(contact.givenname)?" ":"");
	prints(sid, "%s%s", str2html(sid, contact.surname), strlen(contact.surname)?" ":"");
	if (auth_priv(sid, AUTH_CONTACTS)&A_MODIFY) {
		prints(sid, " [<A HREF=%s/contacts/edit?contactid=%d STYLE='color: %s'>edit</A>]", sid->dat->in_ScriptName, contactid, COLOR_THLINK);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Surname          </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100><NOBR>%s&nbsp;</NOBR></TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.surname));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Organization     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.organization));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Given Name       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.givenname));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Job Title        </B></TD><TD BGCOLOR=%s NOWRAP WIDTH=100><NOBR>%s&nbsp;</NOBR></TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.jobtitle));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Salutation       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.salutation));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Work Phone       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.worknumber));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Contact Type     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.contacttype));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Home Phone       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.homenumber));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Referred By      </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.referredby));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>FAX Number       </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.faxnumber));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Alternate Contact</B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.altcontact));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Mobile Number    </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.mobilenumber));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Billing Method   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.prefbilling));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>E-mail           </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	if (strlen(contact.email)==0) {
		prints(sid, "&nbsp;</TD></TR>\n");
	} else if (sid->dat->user_maildefault==0) {
		prints(sid, "<A HREF=\"mailto:%s\">%s</A>&nbsp;</TD></TR>\n", contact.email, str2html(sid, contact.email));
	} else {
		if (sid->dat->user_menustyle>0) {
			prints(sid, "<A HREF=\"javascript:MsgTo('%s')\">%s</A>&nbsp;</TD></TR>\n", contact.email, str2html(sid, contact.email));
		} else {
			prints(sid, "<A HREF=\"mailwrite?msgto=%s\">%s</A>&nbsp;</TD></TR>\n", contact.email, str2html(sid, contact.email));
		}
	}
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>Addresses</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT);
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Home Address  </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	if (strlen(maddr)) {
		prints(sid, "<A HREF=\"%s\" TARGET=_blank>%s</A>&nbsp;</TD>\n", maddr, str2html(sid, contact.homeaddress));
	} else {
		prints(sid, "%s&nbsp;</TD>\n", str2html(sid, contact.homeaddress));
	}
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Work Address  </B></TD><TD BGCOLOR=%s NOWRAP>", COLOR_FNAME, COLOR_FVAL);
	if (strlen(baddr)) {
		prints(sid, "<A HREF=\"%s\" TARGET=_blank>%s</A>&nbsp;</TD></TR>\n", baddr, contact.workaddress);
	} else {
		prints(sid, "%s&nbsp;</TD></TR>\n", contact.workaddress);
	}
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Home City     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.homelocality));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Work City     </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.worklocality));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Home Province </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.homeregion));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Work Province </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.workregion));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Home Country  </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.homecountry));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Work Country  </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.workcountry));
	prints(sid, "<TR><TD BGCOLOR=%s NOWRAP><B>Home Postal   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.homepostalcode));
	prints(sid, "    <TD BGCOLOR=%s NOWRAP><B>Work Postal   </B></TD><TD BGCOLOR=%s NOWRAP>%s&nbsp;</TD></TR>\n", COLOR_FNAME, COLOR_FVAL, str2html(sid, contact.workpostalcode));
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=4 NOWRAP><FONT COLOR=%s>Notes", COLOR_TH, COLOR_THTEXT);
	prints(sid, " [<A HREF=%s/notes/editnew?table=contacts&index=%d STYLE='color: %s'>new</A>]", sid->dat->in_ScriptName, contact.contactid, COLOR_THLINK);
	prints(sid, "</FONT></TH></TR>\n");
	notessublist(sid, "contacts", contact.contactid, 4);
	prints(sid, "</TABLE>\n");
	if (auth_priv(sid, AUTH_CALLS)&A_READ) {
		if ((sqr=sqlQueryf(sid, "SELECT count(callid) FROM gw_calls WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0)", contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		prints(sid, "[<A HREF=%s/contacts/callslist?contactid=%d>%d Calls</A>]\n", sid->dat->in_ScriptName, contactid, atoi(sqlGetvalue(sqr, 0, 0)));
		sqlFreeconnect(sqr);
	}
	if (auth_priv(sid, AUTH_CALENDAR)&A_READ) {
		if ((sqr=sqlQueryf(sid, "SELECT count(eventid) FROM gw_events WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0)", contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
		prints(sid, "[<A HREF=%s/contacts/eventlist?contactid=%d>%d Events</A>]\n", sid->dat->in_ScriptName, contactid, atoi(sqlGetvalue(sqr, 0, 0)));
		sqlFreeconnect(sqr);
	}
	if (auth_priv(sid, AUTH_ORDERS)&A_READ) {
		if ((sqr=sqlQueryf(sid, "SELECT count(orderid) FROM gw_orders WHERE contactid = %d", contactid))<0) return;
		prints(sid, "[<A HREF=%s/contacts/orderlist?contactid=%d>%d Orders</A>]\n", sid->dat->in_ScriptName, contactid, atoi(sqlGetvalue(sqr, 0, 0)));
		sqlFreeconnect(sqr);
	}
	prints(sid, "</CENTER>\n");
	return;
}

void contactlist(CONNECTION *sid)
{
	char searchstring[10];
	int offset=0;
	int i;
	int sqr1;

	if (!(auth_priv(sid, AUTH_CONTACTS)&A_READ)) {
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
	prints(sid, "		if (mailform[\"option\"+i].value=='TO') {\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			if (j>0) output+=',';\r\n");
	} else {
		prints(sid, "			if (j>0) output+=';';\r\n");
	}
	prints(sid, "			output+=mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (mailform[\"option\"+i].value=='CC') {\n");
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			if (j==0) output+='&cc=';\r\n");
	} else {
		prints(sid, "			if (j==0) output+='?cc=';\r\n");
	}
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			else if (j>0) output+=',';\r\n");
	} else {
		prints(sid, "			else if (j>0) output+=';';\r\n");
	}
	prints(sid, "			output+=mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			cc=1;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	var j=0;\r\n");
	prints(sid, "	for (var i=0;i<entries;i++) {\r\n");
	prints(sid, "		if (mailform[\"option\"+i].value=='BCC') {\n");
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
	if (sid->dat->user_maildefault>0) {
		prints(sid, "			} else if (j>0) output+=',';\r\n");
	} else {
		prints(sid, "			} else if (j>0) output+=';';\r\n");
	}
	prints(sid, "			output+=mailform[\"addr\"+i].value;\r\n");
	prints(sid, "			j++;\r\n");
	prints(sid, "		}\r\n");
	prints(sid, "	}\r\n");
	if ((sid->dat->user_maildefault>0)&&(sid->dat->user_menustyle>0)) {
		prints(sid, "	output+=\"')\";\r\n");
	}
	prints(sid, "	document.links[mailtoLink2].href=output;\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function MailTo(rcptlist) {\r\n");
	prints(sid, "	window.open('%s/mail/write?'+rcptlist,'_blank','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=no,resizable=no,width=663,height=440');\r\n", sid->dat->in_ScriptName);
	prints(sid, "}\r\n");
	prints(sid, "//--></SCRIPT>\r\n");
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 HEIGHT=100%% WIDTH=100%%><TR><TD ALIGN=CENTER VALIGN=TOP WIDTH=100%%>\n");
	if (getgetenv(sid, "s")==NULL) {
		if ((sqr1=sqlQuery(sid, "SELECT COUNT(contactid) FROM gw_contacts"))<0) return;
		prints(sid, "<BR>There are %d contacts in the database.<BR>\n", atoi(sqlGetvalue(sqr1, 0, 0)));
		sqlFreeconnect(sqr1);
	} else {
		strncpy(searchstring, getgetenv(sid, "S"), sizeof(searchstring)-1);
		if (strcasecmp(searchstring, "All")==0) {
			if (auth_priv(sid, AUTH_CONTACTS)&A_ADMIN) {
				if ((sqr1=sqlQueryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts ORDER BY surname, givenname ASC", searchstring))<0) return;
			} else {
				if ((sqr1=sqlQueryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
			}
		} else {
			if (auth_priv(sid, AUTH_CONTACTS)&A_ADMIN) {
				if (strcmp(config.sql_type, "ODBC")==0) {
					if ((sqr1=sqlQueryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (surname like '%s%%') ORDER BY surname, givenname ASC", searchstring))<0) return;
				} else {
					if ((sqr1=sqlQueryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (lower(surname) like lower('%s%%')) ORDER BY surname, givenname ASC", searchstring))<0) return;
				}
			} else {
				if (strcmp(config.sql_type, "ODBC")==0) {
					if ((sqr1=sqlQueryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (surname like '%s%%')  and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", searchstring, sid->dat->user_uid, sid->dat->user_gid))<0) return;
				} else {
					if ((sqr1=sqlQueryf(sid, "SELECT contactid, surname, givenname, organization, worknumber, email from gw_contacts WHERE (lower(surname) like lower('%s%%'))  and (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY surname, givenname ASC", searchstring, sid->dat->user_uid, sid->dat->user_gid))<0) return;
				}
			}
		}
		if (getgetenv(sid, "OFFSET")!=NULL) {
			offset=atoi(getgetenv(sid, "OFFSET"));
		}
		prints(sid, "<CENTER><BR>\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>\n");
		prints(sid, "<TD ALIGN=LEFT NOWRAP WIDTH=150>&nbsp;</TD>\n");
		prints(sid, "<TD ALIGN=CENTER NOWRAP>Found %d matching contacts</TD>\n", sqlNumtuples(sqr1));
		prints(sid, "<TD ALIGN=RIGHT NOWRAP WIDTH=150>&nbsp;</TD>\n");
		prints(sid, "</TR>\n");
		if (sqlNumtuples(sqr1)>0) {
			prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=3>\n");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
			prints(sid, "<FORM METHOD=GET NAME=mailform>\n");
			prints(sid, "<TR BGCOLOR=%s><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Contact Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Company Name&nbsp;</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>&nbsp;Work Number&nbsp;</FONT></TH><TH ALIGN=LEFT COLSPAN=2 NOWRAP><FONT COLOR=%s>&nbsp;E-Mail&nbsp;</FONT></TH></TR>\n", COLOR_TH, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
			for (i=offset;(i<sqlNumtuples(sqr1))&&(i<offset+sid->dat->user_maxlist);i++) {
				prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
				prints(sid, "<TD NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/contacts/view?contactid=%d'\">", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)));
				prints(sid, "<A HREF=%s/contacts/view?contactid=%d>%s", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr1, i, 0)), str2html(sid, sqlGetvalue(sqr1, i, 1)));
				if (strlen(sqlGetvalue(sqr1, i, 1))&&strlen(sqlGetvalue(sqr1, i, 2))) prints(sid, ", ");
				if (strlen(sqlGetvalue(sqr1, i, 1))==0&&strlen(sqlGetvalue(sqr1, i, 2))==0) prints(sid, "&nbsp;");
				prints(sid, "%s</A>&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr1, i, 2)));
				prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr1, i, 3)));
				prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr1, i, 4)));
				if (strlen(sqlGetvalue(sqr1, i, 5))==0) {
					prints(sid, "<TD NOWRAP>&nbsp;</TD>");
				} else if (sid->dat->user_maildefault==0) {
					prints(sid, "<TD NOWRAP><A HREF=\"mailto:%s\">%s</A>&nbsp;</TD>", sqlGetvalue(sqr1, i, 5), sqlGetvalue(sqr1, i, 5));
				} else {
					if (sid->dat->user_menustyle>0) {
						prints(sid, "<TD NOWRAP><A HREF=\"javascript:MsgTo('%s')\">%s</A>&nbsp;</TD>", sqlGetvalue(sqr1, i, 5), str2html(sid, sqlGetvalue(sqr1, i, 5)));
					} else {
						prints(sid, "<TD NOWRAP><A HREF=\"%s/mail/write?msgto=%s\">%s</A>&nbsp;</TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr1, i, 5), str2html(sid, sqlGetvalue(sqr1, i, 5)));
					}
				}
				prints(sid, "<INPUT TYPE=hidden NAME=addr%d VALUE=\"%s\">", i-offset, sqlGetvalue(sqr1, i, 5));
				prints(sid, "<TD NOWRAP STYLE='padding:0px'><SELECT NAME=option%d onchange=MailUpdate(); STYLE='font-size:11px; width:44px'>", i-offset);
				prints(sid, "<OPTION VALUE=''>");
				if (strchr(sqlGetvalue(sqr1, i, 5), '@')) {
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
			if (sqlNumtuples(sqr1)>sid->dat->user_maxlist) {
				if (offset>sid->dat->user_maxlist-1) {
					prints(sid, "[<A HREF=%s/contacts/list?s=%s&offset=%d>Previous Page</A>]", sid->dat->in_ScriptName, searchstring, offset-sid->dat->user_maxlist);
				} else {
					prints(sid, "[Previous Page]");
				}
				if (offset+sid->dat->user_maxlist<sqlNumtuples(sqr1)) {
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
		sqlFreeconnect(sqr1);
	}
	prints(sid, "</TD><TD BGCOLOR=%s VALIGN=TOP>\n", COLOR_FNAME);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><FONT SIZE=2><B><A HREF=%s/contacts/list?s=all>&nbsp;All&nbsp;</A></B></FONT></TD></TR>\n", COLOR_FNAME, sid->dat->in_ScriptName);
	for (i=65;i<91;i++) {
		prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><FONT SIZE=2><B><A HREF=%s/contacts/list?s=%c>&nbsp;%c&nbsp;</A></B></FONT></TD></TR>\n", COLOR_FNAME, sid->dat->in_ScriptName, i, i);
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
void contactcallslist(CONNECTION *sid)
{
	int contactid=0;
	int i;
	int sqr;
	int sqr2;
	time_t calldate;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_CALLS)&A_READ)) {
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
	if ((sqr=sqlQueryf(sid, "SELECT callid, contactid, calldate, action FROM gw_calls WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY calldate DESC", contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	if ((sqr2=sqlQueryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sqlFreeconnect(sqr);
		return;
	}
	prints(sid, "Found %d calls for <A HREF=%s/contacts/view?contactid=%s>%s", sqlNumtuples(sqr), sid->dat->in_ScriptName, sqlGetvalue(sqr2, 0, 0), str2html(sid, sqlGetvalue(sqr2, 0, 1)));
	if (strlen(sqlGetvalue(sqr2, 0, 1))&&strlen(sqlGetvalue(sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR>\n", str2html(sid, sqlGetvalue(sqr2, 0, 2)));
	sqlFreeconnect(sqr2);
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n<TR BGCOLOR=%s>", COLOR_TH);
		prints(sid, "<TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Call ID</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Action</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Date</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Time</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/calls/view?callid=%s'\">", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints(sid, "&nbsp;<A HREF=%s/calls/view?callid=%s>%s</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 0));
			prints(sid, "<TD NOWRAP WIDTH=100%%>%s&nbsp;</TD>", htview_callaction(sid, atoi(sqlGetvalue(sqr, i, 3))));
			calldate=time_sql2unix(sqlGetvalue(sqr, i, 2));
			calldate+=time_tzoffset(sid, calldate);
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2datetext(sid, calldate));
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_unix2timetext(sid, calldate));
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/calls/editnew?contactid=%d>New Call</A>\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
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
void contacteventlist(CONNECTION *sid)
{
	char *priority[5]={ "Lowest", "Low", "Normal", "High", "Highest" };
	int contactid=0;
	int i;
	int sqr;
	int sqr2;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_CALENDAR)&A_READ)) {
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
	if ((sqr=sqlQueryf(sid, "SELECT eventid, eventname, eventstart, status, priority, contactid FROM gw_events WHERE contactid = %d and (assignedto = %d or obj_uid = %d or (obj_gid = %d and obj_gperm>0) or obj_operm>0) ORDER BY eventstart DESC", contactid, sid->dat->user_uid, sid->dat->user_uid, sid->dat->user_gid))<0) return;
	if ((sqr2=sqlQueryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sqlFreeconnect(sqr);
		return;
	}
	prints(sid, "Found %d events for <A HREF=%s/contacts/view?contactid=%s>%s", sqlNumtuples(sqr), sid->dat->in_ScriptName, sqlGetvalue(sqr2, 0, 0), str2html(sid, sqlGetvalue(sqr2, 0, 1)));
	if (strlen(sqlGetvalue(sqr2, 0, 1))&&strlen(sqlGetvalue(sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR>\n", str2html(sid, sqlGetvalue(sqr2, 0, 2)));
	sqlFreeconnect(sqr2);
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n<TR BGCOLOR=%s>", COLOR_TH);
		prints(sid, "<TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Event ID</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Event Name</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Date</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Status</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Priority</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/calendar/view?eventid=%d'\">", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "&nbsp;<A HREF=%s/calendar/view?eventid=%d>%d</A></TD>", sid->dat->in_ScriptName, atoi(sqlGetvalue(sqr, i, 0)), atoi(sqlGetvalue(sqr, i, 0)));
			prints(sid, "<TD NOWRAP WIDTH=100%%>%s&nbsp;</TD>", str2html(sid, sqlGetvalue(sqr, i, 1)));
			prints(sid, "<TD ALIGN=right NOWRAP>%s&nbsp;</TD>", time_sql2datetext(sid, sqlGetvalue(sqr, i, 2)));
			prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", htview_eventstatus(sid, atoi(sqlGetvalue(sqr, i, 3))));
			prints(sid, "<TD NOWRAP>%s&nbsp;</TD>", priority[atoi(sqlGetvalue(sqr, i, 4))]);
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/calendar/editnew?contactid=%d>New Event</A>\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
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
void contactorderlist(CONNECTION *sid)
{
	int contactid=0;
	int i;
	int sqr;
	int sqr2;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_ORDERS)&A_READ)) {
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
	if ((sqr=sqlQueryf(sid, "SELECT orderid, orderdate, paymentdue, paymentreceived from gw_orders WHERE contactid = %d ORDER BY orderdate DESC", contactid))<0) return;
	if ((sqr2=sqlQueryf(sid, "SELECT contactid, surname, givenname FROM gw_contacts WHERE contactid = %d", contactid))<0) {
		sqlFreeconnect(sqr);
		return;
	}
	prints(sid, "Found %d orders for <A HREF=%s/contacts/view?contactid=%s>%s", sqlNumtuples(sqr), sid->dat->in_ScriptName, sqlGetvalue(sqr2, 0, 0), str2html(sid, sqlGetvalue(sqr2, 0, 1)));
	if (strlen(sqlGetvalue(sqr2, 0, 1))&&strlen(sqlGetvalue(sqr2, 0, 2))) prints(sid, ", ");
	prints(sid, "%s</A><BR>\n", str2html(sid, sqlGetvalue(sqr2, 0, 2)));
	sqlFreeconnect(sqr2);
	if (sqlNumtuples(sqr)>0) {
		prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=350>\n<TR BGCOLOR=%s>", COLOR_TH);
		prints(sid, "<TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Order ID</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Order Date</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Amount Due</FONT></TH><TH ALIGN=LEFT NOWRAP><FONT COLOR=%s>Amount Received</FONT></TH></TR>\n", COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT, COLOR_THTEXT);
		for (i=0;i<sqlNumtuples(sqr);i++) {
			prints(sid, "<TR BGCOLOR=%s>", COLOR_FVAL);
			prints(sid, "<TD ALIGN=RIGHT NOWRAP style=\"cursor:hand\" onClick=\"window.location.href='%s/orders/view?orderid=%s'\">", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
			prints(sid, "&nbsp;<A HREF=%s/orders/view?orderid=%s>%s</A></TD>", sid->dat->in_ScriptName, sqlGetvalue(sqr, i, 0), sqlGetvalue(sqr, i, 0));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>%s&nbsp;</TD>", time_sql2datetext(sid, sqlGetvalue(sqr, i, 1)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 2)));
			prints(sid, "<TD ALIGN=RIGHT NOWRAP>$%1.2f&nbsp;</TD>", atof(sqlGetvalue(sqr, i, 3)));
			prints(sid, "</TR>\n");
		}
		prints(sid, "</TABLE>\n");
	}
	prints(sid, "<A HREF=%s/orders/editnew?contactid=%d>New Order</A>\n", sid->dat->in_ScriptName, contactid);
	prints(sid, "</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

void contactsave(CONNECTION *sid)
{
	REC_CONTACT contact;
	char opassword[50];
	char *ptemp;
	int contactid;

	prints(sid, "<BR>\r\n");
	if (!(auth_priv(sid, AUTH_CONTACTS)&A_MODIFY)) {
		prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "CONTACTID"))==NULL) return;
	contactid=atoi(ptemp);
	if (db_read(sid, 2, DB_CONTACTS, contactid, &contact)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	snprintf(opassword, sizeof(opassword)-1, "%s", contact.password);
	if (auth_priv(sid, AUTH_CONTACTS)&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) contact.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) contact.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, AUTH_CONTACTS)&A_ADMIN)||(contact.obj_uid==sid->dat->user_uid)) {
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
		if (!(auth_priv(sid, AUTH_CONTACTS)&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sqlUpdatef(sid, "DELETE FROM gw_contacts WHERE contactid = %d", contact.contactid)<0) return;
		prints(sid, "<CENTER>Contact %d deleted successfully</CENTER><BR>\n", contactid);
		logaccess(sid, 1, "%s - %s deleted contact %d", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/list\">\n", sid->dat->in_ScriptName);
	} else if (contact.contactid==0) {
		if (!(auth_priv(sid, AUTH_CONTACTS)&A_INSERT)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		if ((contact.contactid=db_write(sid, DB_CONTACTS, 0, &contact))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Contact %d added successfully</CENTER><BR>\n", contact.contactid);
		logaccess(sid, 1, "%s - %s added contact %d", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\">\n", sid->dat->in_ScriptName, contact.contactid);
	} else {
		if (!(auth_priv(sid, AUTH_CONTACTS)&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (strcmp(opassword, contact.password)!=0) {
			snprintf(contact.password, sizeof(contact.password)-1, "%s", auth_setpass(sid, contact.password));
		}
		if (db_write(sid, DB_CONTACTS, contactid, &contact)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Contact %d modified successfully</CENTER><BR>\n", contact.contactid);
		logaccess(sid, 1, "%s - %s modified contact %d", sid->dat->in_RemoteAddr, sid->dat->user_username, contact.contactid);
		prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/contacts/view?contactid=%d\">\n", sid->dat->in_ScriptName, contact.contactid);
	}
	return;
}

void contactmain(CONNECTION *sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_CONTACTS);
	if (strncmp(sid->dat->in_RequestURI, "/contacts/edit", 14)==0) {
		contactedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/contacts/view", 14)==0) {
		contactview(sid);
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
