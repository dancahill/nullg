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
#include "mod_contacts.h"

void contacts_vcardexport(CONN *sid)
{
	REC_CONTACT contact;
	char *ptemp;
	char mtime[32];
	int contactid=0;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, MENU_CONTACTS);
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "CONTACTID"))==NULL) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, MENU_CONTACTS);
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", lang.err_noaccess);
		return;
	}
	contactid=atoi(ptemp);
	if (dbread_contact(sid, 1, contactid, &contact)<0) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, MENU_CONTACTS);
		prints(sid, "<BR><CENTER>No matching record found for %d</CENTER>\n", contactid);
		return;
	}
	snprintf(sid->dat->out_ContentDisposition, sizeof(sid->dat->out_ContentDisposition)-1, "attachment; filename=\"%s%s%s.vcf\"", contact.givenname, strlen(contact.givenname)?" ":"", contact.surname);
	send_header(sid, 1, 200, "1", "application/octet-stream", -1, -1);
	flushbuffer(sid);
	memset(mtime, 0, sizeof(mtime));
	strftime(mtime, 30, "%Y%m%dT%H%M%SZ", gmtime(&contact.obj_mtime));
	prints(sid, "BEGIN:VCARD\r\n");
	prints(sid, "VERSION:2.1\r\n");
	prints(sid, "N:%s;%s;;%s\r\n", contact.surname, contact.givenname, contact.salutation);
	prints(sid, "FN:%s%s%s\r\n", str2html(sid, contact.givenname), strlen(contact.givenname)?" ":"", str2html(sid, contact.surname));
	if (strlen(contact.username))     prints(sid, "NICKNAME:%s\r\n", contact.username);
	if (strlen(contact.organization)) prints(sid, "ORG:%s\r\n", contact.organization);
	if (strlen(contact.jobtitle))     prints(sid, "TITLE:%s\r\n", contact.jobtitle);
	if (strlen(contact.worknumber))   prints(sid, "TEL;WORK;VOICE:%s\r\n", contact.worknumber);
	if (strlen(contact.homenumber))   prints(sid, "TEL;HOME;VOICE:%s\r\n", contact.homenumber);
	if (strlen(contact.mobilenumber)) prints(sid, "TEL;CELL;VOICE:%s\r\n", contact.mobilenumber);
	if (strlen(contact.faxnumber))    prints(sid, "TEL;WORK;FAX:%s\r\n", contact.faxnumber);
	prints(sid, "ADR;HOME:;;%s;%s;%s;%s;%s\r\n", contact.homeaddress, contact.homelocality, contact.homeregion, contact.homepostalcode, contact.homecountry);
	prints(sid, "ADR;WORK:;;%s;%s;%s;%s;%s\r\n", contact.workaddress, contact.worklocality, contact.workregion, contact.workpostalcode, contact.workcountry);
	if (strlen(contact.email))        prints(sid, "EMAIL;PREF;INTERNET:%s\r\n", contact.email);
	prints(sid, "REV:%s\r\n", mtime);
	prints(sid, "END:VCARD\r\n");
	return;
}

void contacts_vcardimport(CONN *sid)
{
	char buffer[4096];
	char line[256];
	char linetemp[256];
	char *pbuffer;
	char *ptemp;
	char *ptemp2;
	unsigned int lineindex;
	unsigned int mimesize;
	unsigned int newfields=0;

	htpage_header(sid, "vCard Import");
	memset(buffer, 0, sizeof(buffer));
	memset(line, 0, sizeof(line));
	memset(linetemp, 0, sizeof(linetemp));
	if (strcasecmp(sid->dat->in_RequestMethod, "POST")!=0) {
		prints(sid, "<CENTER><BR>\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
		prints(sid, "<FORM METHOD=POST ACTION=%s/contacts/vcardimport NAME=vcardimport ENCTYPE=multipart/form-data>\n", sid->dat->in_ScriptName);
		prints(sid, "<TR><TH COLSPAN=2>vCard Import</TH></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><INPUT TYPE=file NAME=userfile SIZE=35></TD><TD><INPUT TYPE=submit VALUE='Import'></TD></TR>\n");
		prints(sid, "</FORM>\n");
		prints(sid, "</TABLE>\n");
		prints(sid, "</CENTER>\n");
	} else {
		if ((ptemp=getmimeenv(sid, "USERFILE", &mimesize))!=NULL) {
			strncpy(buffer, ptemp, sizeof(buffer)-1);
			if (mimesize<strlen(buffer)) buffer[mimesize]='\0';
		}
		pbuffer=buffer;
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
		prints(sid, "if (window.opener.document.contactedit) {\r\n");
		while (strlen(pbuffer)>0) {
			memset(line, 0, sizeof(line));
			lineindex=0;
			while ((*pbuffer)&&(lineindex<sizeof(line)-1)&&(*pbuffer!='\r')&&(*pbuffer!='\n')) {
				line[lineindex++]=*pbuffer++;
			}
			while ((*pbuffer=='\r')||(*pbuffer=='\n')) pbuffer++;
			ptemp=line;
			if ((strncasecmp(ptemp, "ADR:", 4)==0)||(strncasecmp(ptemp, "ADR;", 4)==0)) {
				if ((strncasecmp(ptemp, "ADR;HOME:", 9)==0)||(strncasecmp(ptemp, "ADR;HOME;", 9)==0)) {
					while ((*ptemp)&&(*ptemp!=':')) ptemp++;
					if (*ptemp==':') ptemp++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.homeaddress.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.homelocality.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.homeregion.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.homepostalcode.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.homecountry.value=\"%s\";\r\n", linetemp);
					newfields++;
				} else if ((strncasecmp(ptemp, "ADR;WORK:", 9)==0)||(strncasecmp(ptemp, "ADR;WORK;", 9)==0)) {
					while ((*ptemp)&&(*ptemp!=':')) ptemp++;
					if (*ptemp==':') ptemp++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.workaddress.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.worklocality.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.workregion.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.workpostalcode.value=\"%s\";\r\n", linetemp);
					newfields++;
					while ((*ptemp)&&(*ptemp!=';')) ptemp++;
					if (*ptemp==';') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.workcountry.value=\"%s\";\r\n", linetemp);
					newfields++;
				}
			} else if ((strncasecmp(ptemp, "EMAIL:", 6)==0)||(strncasecmp(ptemp, "EMAIL;", 6)==0)) {
				while ((*ptemp)&&(*ptemp!=':')) ptemp++;
				if (*ptemp==':') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.email.value=\"%s\";\r\n", linetemp);
				newfields++;
			} else if ((strncasecmp(ptemp, "N:", 2)==0)||(strncasecmp(ptemp, "N;", 2)==0)) {
				while ((*ptemp)&&(*ptemp!=':')) ptemp++;
				if (*ptemp==':') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.surname.value=\"%s\";\r\n", linetemp);
				newfields++;
				while ((*ptemp)&&(*ptemp!=';')) ptemp++;
				if (*ptemp==';') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.givenname.value=\"%s\";\r\n", linetemp);
				newfields++;
				while ((*ptemp)&&(*ptemp!=';')) ptemp++;
				if (*ptemp==';') ptemp++;
				while ((*ptemp)&&(*ptemp!=';')) ptemp++;
				if (*ptemp==';') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.salutation.value=\"%s\";\r\n", linetemp);
				newfields++;
			} else if ((strncasecmp(ptemp, "NICKNAME:", 9)==0)||(strncasecmp(ptemp, "NICKNAME;", 9)==0)) {
				while ((*ptemp)&&(*ptemp!=':')) ptemp++;
				if (*ptemp==':') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.username.value=\"%s\";\r\n", linetemp);
				newfields++;
			} else if ((strncasecmp(ptemp, "ORG:", 4)==0)||(strncasecmp(ptemp, "ORG;", 4)==0)) {
				while ((*ptemp)&&(*ptemp!=':')) ptemp++;
				if (*ptemp==':') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.organization.value=\"%s\";\r\n", linetemp);
				newfields++;
			} else if ((strncasecmp(ptemp, "TEL:", 4)==0)||(strncasecmp(ptemp, "TEL;", 4)==0)) {
				if ((strncasecmp(ptemp, "TEL;CELL;VOICE:", 15)==0)||(strncasecmp(ptemp, "TEL;CELL;VOICE;", 15)==0)) {
					while ((*ptemp)&&(*ptemp!=':')) ptemp++;
					if (*ptemp==':') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.mobilenumber.value=\"%s\";\r\n", linetemp);
					newfields++;
				} else if ((strncasecmp(ptemp, "TEL;HOME;VOICE:", 15)==0)||(strncasecmp(ptemp, "TEL;HOME;VOICE;", 15)==0)) {
					while ((*ptemp)&&(*ptemp!=':')) ptemp++;
					if (*ptemp==':') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.homenumber.value=\"%s\";\r\n", linetemp);
					newfields++;
				} else if ((strncasecmp(ptemp, "TEL;WORK;FAX:", 13)==0)||(strncasecmp(ptemp, "TEL;WORK;FAX;", 13)==0)) {
					while ((*ptemp)&&(*ptemp!=':')) ptemp++;
					if (*ptemp==':') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.faxnumber.value=\"%s\";\r\n", linetemp);
					newfields++;
				} else if ((strncasecmp(ptemp, "TEL;WORK;VOICE:", 15)==0)||(strncasecmp(ptemp, "TEL;WORK;VOICE;", 15)==0)) {
					while ((*ptemp)&&(*ptemp!=':')) ptemp++;
					if (*ptemp==':') ptemp++;
					snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
					if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
					prints(sid, "	window.opener.document.contactedit.worknumber.value=\"%s\";\r\n", linetemp);
					newfields++;
				}
			} else if ((strncasecmp(ptemp, "TITLE:", 6)==0)||(strncasecmp(ptemp, "TITLE;", 6)==0)) {
				while ((*ptemp)&&(*ptemp!=':')) ptemp++;
				if (*ptemp==':') ptemp++;
				snprintf(linetemp, sizeof(linetemp)-1, "%s", ptemp);
				if ((ptemp2=strchr(linetemp, ';'))!=NULL) *ptemp2='\0';
				prints(sid, "	window.opener.document.contactedit.jobtitle.value=\"%s\";\r\n", linetemp);
				newfields++;
			}
		}
		prints(sid, "}\r\n");
		prints(sid, "// -->\n</SCRIPT>\n");
		prints(sid, "<CENTER>\n");
		prints(sid, "%d fields imported<BR>\r\n", newfields);
		prints(sid, "<A HREF=javascript:window.close()>Close Window</A>\n");
		prints(sid, "</CENTER>\n");
	}
	prints(sid, "</BODY></HTML>\n");
	return;
}
