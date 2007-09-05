/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
#include "mod_contacts.h"

void contacts_vcardexport(CONN *sid)
{
	obj_t *rtobj=nes_getobj(sid->N, &sid->N->g, "_HEADER");
	obj_t *qobj=NULL;
	char *ptemp;
	char disp[255];
	char mtime[32];
	int id=0;

	if (!(auth_priv(sid, "contacts")&A_READ)) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, "contacts");
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	if ((ptemp=getgetenv(sid, "ID"))==NULL) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, "contacts");
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang_gets(sid, "common", "err_noaccess"));
		return;
	}
	id=atoi(ptemp);
	if (dbread_contact(sid, 1, id, &qobj)<0) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, "contacts");
		prints(sid, "<BR /><CENTER>No matching record found for %d</CENTER>\r\n", id);
		return;
	}
	snprintf(disp, sizeof(disp)-1, "attachment; filename=\"%s%s%s.vcf\"", ldir_getval(&qobj, 0, "gn"), strlen(ldir_getval(&qobj, 0, "gn"))?" ":"", ldir_getval(&qobj, 0, "sn"));
	nes_setstr(sid->N, rtobj, "CONTENT_DISPOSITION", disp, strlen(disp));
	send_header(sid, 1, 200, "1", "application/octet-stream", -1, -1);
	flushbuffer(sid);
	prints(sid, "BEGIN:VCARD\r\n");
	prints(sid, "VERSION:2.1\r\n");
	prints(sid, "N:%s;%s;;%s\r\n", ldir_getval(&qobj, 0, "sn"), ldir_getval(&qobj, 0, "gn"), ldir_getval(&qobj, 0, "personaltitle"));
	prints(sid, "FN:%s%s%s\r\n", str2html(sid, ldir_getval(&qobj, 0, "gn")), strlen(ldir_getval(&qobj, 0, "gn"))?" ":"", str2html(sid, ldir_getval(&qobj, 0, "sn")));
	if (strlen(ldir_getval(&qobj, 0, "uid")))                      prints(sid, "NICKNAME:%s\r\n", ldir_getval(&qobj, 0, "uid"));
	if (strlen(ldir_getval(&qobj, 0, "organization")))             prints(sid, "ORG:%s\r\n", ldir_getval(&qobj, 0, "organization"));
	if (strlen(ldir_getval(&qobj, 0, "title")))                    prints(sid, "TITLE:%s\r\n", ldir_getval(&qobj, 0, "title"));
	if (strlen(ldir_getval(&qobj, 0, "worknumber")))               prints(sid, "TEL;WORK;VOICE:%s\r\n", ldir_getval(&qobj, 0, "worknumber"));
	if (strlen(ldir_getval(&qobj, 0, "homenumber")))               prints(sid, "TEL;HOME;VOICE:%s\r\n", ldir_getval(&qobj, 0, "homenumber"));
	if (strlen(ldir_getval(&qobj, 0, "mobiletelephonenumber")))    prints(sid, "TEL;CELL;VOICE:%s\r\n", ldir_getval(&qobj, 0, "mobiletelephonenumber"));
	if (strlen(ldir_getval(&qobj, 0, "facsimiletelephonenumber"))) prints(sid, "TEL;WORK;FAX:%s\r\n", ldir_getval(&qobj, 0, "facsimiletelephonenumber"));
	prints(sid, "ADR;HOME:;;%s;%s;%s;%s;%s\r\n", ldir_getval(&qobj, 0, "homeaddress"), ldir_getval(&qobj, 0, "homelocality"), ldir_getval(&qobj, 0, "homeregion"), ldir_getval(&qobj, 0, "homepostalcode"), ldir_getval(&qobj, 0, "homecountry"));
	prints(sid, "ADR;WORK:;;%s;%s;%s;%s;%s\r\n", ldir_getval(&qobj, 0, "workaddress"), ldir_getval(&qobj, 0, "worklocality"), ldir_getval(&qobj, 0, "workregion"), ldir_getval(&qobj, 0, "workpostalcode"), ldir_getval(&qobj, 0, "workcountry"));
	if (strlen(ldir_getval(&qobj, 0, "mail")))                     prints(sid, "EMAIL;PREF;INTERNET:%s\r\n", ldir_getval(&qobj, 0, "mail"));
	memset(mtime, 0, sizeof(mtime));
//	strftime(mtime, 30, "%Y%m%dT%H%M%SZ", gmtime(&contact.obj_mtime));
	prints(sid, "REV:%s\r\n", mtime);
	prints(sid, "END:VCARD\r\n");
	return;
}

void contacts_vcardimport(CONN *sid)
{
	obj_t *htobj=nes_getobj(sid->N, &sid->N->g, "_SERVER");
	char line[256];
	char linetemp[256];
	char *pbuffer;
	char *ptemp;
	char *ptemp2;
	unsigned int lineindex;
	unsigned int newfields=0;

	htpage_header(sid, "vCard Import");
	memset(line, 0, sizeof(line));
	memset(linetemp, 0, sizeof(linetemp));
	if (strcasecmp(nes_getstr(sid->N, htobj, "REQUEST_METHOD"), "POST")!=0) {
		prints(sid, "<CENTER><BR />\r\n");
		prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n");
		prints(sid, "<FORM METHOD=POST ACTION=%s/contacts/vcardimport NAME=vcardimport ENCTYPE=multipart/form-data>\r\n", nes_getstr(sid->N, htobj, "SCRIPT_NAME"));
		prints(sid, "<TR><TH COLSPAN=2>vCard Import</TH></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD><INPUT TYPE=file NAME=userfile SIZE=35></TD><TD><INPUT TYPE=submit VALUE='Import'></TD></TR>\r\n");
		prints(sid, "</FORM>\r\n");
		prints(sid, "</TABLE>\r\n");
		prints(sid, "</CENTER>\r\n");
	} else {
		pbuffer=getpostenv(sid, "USERFILE");
		if (pbuffer==NULL) pbuffer="";
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
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
		prints(sid, "// -->\r\n</SCRIPT>\r\n");
		prints(sid, "<CENTER>\r\n");
		prints(sid, "%d fields imported<BR />\r\n", newfields);
		prints(sid, "<A HREF=javascript:window.close()>Close Window</A>\r\n");
		prints(sid, "</CENTER>\r\n");
	}
	prints(sid, "</BODY></HTML>\r\n");
	return;
}
