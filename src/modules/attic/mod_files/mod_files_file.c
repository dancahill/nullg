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
#include "mod_files.h"

int fileul(CONN *sid)
{
/*
	REC_FILE file;
	char *ptemp;
	char directory[255];
	char filename[255];
	char location[255];

	htpage_header(sid, "File Upload");
	if (!(auth_priv(sid, "files")&A_INSERT)) return -1;
	if (dbread_file(sid, 2, 0, &file)!=0) return -1;
	memset(location, 0, sizeof(location));
	if ((ptemp=getgetenv(sid, "LOCATION"))!=NULL) {
		snprintf(directory, sizeof(directory)-1, "%s", ptemp);
		snprintf(location, sizeof(location)-1, "%s", ptemp);
	}
	snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s", config->dir_var_domains, sid->dat->did, directory+7);
	decodeurl(filename);
	fixslashes(filename);
	while ((filename[strlen(filename)-1]=='\\')||(filename[strlen(filename)-1]=='/')) { filename[strlen(filename)-1]='\0'; };
	if (strstr(filename, "..")!=NULL) return -1;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "var x=0;\r\n");
	prints(sid, "function UploadStatus() {\r\n");
	prints(sid, "	var output=\"\\nPlease wait while the file is uploaded \";\r\n");
	prints(sid, "	if (x==0) {\r\n");
	prints(sid, "		output+=\"/\"\r\n");
	prints(sid, "		x=1;\r\n");
	prints(sid, "	} else if (x==1) {\r\n");
	prints(sid, "		output+=\"-\"\r\n");
	prints(sid, "		x=2;\r\n");
	prints(sid, "	} else if (x==2) {\r\n");
	prints(sid, "		output+=\"\\\\\"\r\n");
	prints(sid, "		x=3;\r\n");
	prints(sid, "	} else if (x==3) {\r\n");
	prints(sid, "		output+=\"|\"\r\n");
	prints(sid, "		x=0;\r\n");
	prints(sid, "	}\r\n");
	prints(sid, "	document.getElementById('Text').innerHTML=output;\r\n");
	prints(sid, "	window.setTimeout('UploadStatus()', 500);\r\n");
	prints(sid, "}\r\n");
	prints(sid, "function waitForCompletion() {\r\n");
	prints(sid, "	UploadStatus();\r\n");
	prints(sid, "	document.filesend.submit();\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n<BR />\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/filerecv NAME=filesend ENCTYPE=multipart/form-data onSubmit=waitForCompletion();>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=location value='%s'>\r\n", location);
	prints(sid, "<TR><TH COLSPAN=2>Uploading new file to '%s'</TH></TR>\r\n", directory);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD><B>&nbsp;File&nbsp;</B></TD><TD><INPUT TYPE=file NAME=userfile SIZE=35></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2>&nbsp;<B>Description</B>&nbsp;</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=description ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\r\n", str2html(sid, file.description));
	if ((file.obj_uid==sid->dat->uid)||(auth_priv(sid, "files")&A_ADMIN)) {
		prints(sid, "<TR><TH ALIGN=center COLSPAN=2>Permissions</TH></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, file.obj_uid, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, auth_priv(sid, "files"), file.obj_gid, sid->dat->did);
		prints(sid, "</SELECT></TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP STYLE='padding:0px'>&nbsp;<B>Group Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\r\n", file.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\r\n", file.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\r\n", file.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'>&nbsp;<B>Other Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\r\n", file.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\r\n", file.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\r\n", file.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\r\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\r\n");
	prints(sid, "<FONT FACE=\"Courier\">\r\n");
	prints(sid, "<span id=\"Text\">\r\n");
	prints(sid, "<INPUT TYPE=submit VALUE='Upload'>\r\n");
	prints(sid, "</span>\r\n");
	prints(sid, "</FONT>\r\n");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "</BODY></HTML>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.filesend.userfile.focus();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return 0;
}

int filerecv(CONN *sid)
{
/*
	REC_FILE filerec;
	char query[2048];
	char datebuf[40];
	char description[255];
	SQLRES sqr;
	int fileid;
	unsigned int mimesize;
	time_t t=time(NULL);
	char *directory=NULL;
	char filename[1024];
	char file[1024];
	char line[1024];
	char location[1024];
	char boundary[1024];
	char boundary2[1024];
	char boundary3[1024];
	char *pPostData;
	char *ptemp;
	FILE *fp;
	int i;
	unsigned int j;
	int rc;

	DEBUG_IN(sid, "filerecv()");
	htpage_header(sid, "File Upload");
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%% STYLE='border-style:solid'><TR><TD STYLE='border-style:solid'>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\r\n");
	prints(sid, "<TR CLASS=\"TBAR\"><TD ALIGN=left>&nbsp;</TD><TD ALIGN=right>&nbsp;</TD></TR>\r\n");
	prints(sid, "</TABLE>\r\n</TD></TR></TABLE>\r\n<BR />\r\n");
	if (!(auth_priv(sid, "files")&A_INSERT)) {
		DEBUG_OUT(sid, "filerecv()");
		return -1;
	}
	if (dbread_file(sid, 2, 0, &filerec)!=0) {
		DEBUG_OUT(sid, "filerecv()");
		return -1;
	}
	if (auth_priv(sid, "files")&A_ADMIN) {
		if ((ptemp=getmimeenv(sid, "OBJ_UID", &mimesize))!=NULL) filerec.obj_uid=atoi(ptemp);
		if ((ptemp=getmimeenv(sid, "OBJ_GID", &mimesize))!=NULL) filerec.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "files")&A_ADMIN)||(filerec.obj_uid==sid->dat->uid)) {
		if ((ptemp=getmimeenv(sid, "OBJ_GPERM", &mimesize))!=NULL) filerec.obj_gperm=atoi(ptemp);
		if ((ptemp=getmimeenv(sid, "OBJ_OPERM", &mimesize))!=NULL) filerec.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getmimeenv(sid, "DESCRIPTION", &mimesize))!=NULL) {
		strncpy(description, ptemp, sizeof(description)-1);
		if (mimesize<strlen(description)) description[mimesize]='\0';
	}
	if (sid->PostData==NULL) { DEBUG_OUT(sid, "filerecv()"); return -1; }
	pPostData=sid->PostData;
	memset(boundary, 0, sizeof(boundary));
	memset(location, 0, sizeof(location));
	i=0;
	j=0;
	/ * get the mime boundary * /
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<sid->dat->in_ContentLength)&&(j<sizeof(boundary)-1)) {
		boundary[j]=*pPostData;
		pPostData++;
		i++;
		j++;
	}
	/ * eat newline garbage * /
	while ((*pPostData=='\n')||(*pPostData=='\r')) {
		pPostData++;
		i++;
	}
	snprintf(boundary2, sizeof(boundary2)-1, "%s--", boundary);
	snprintf(boundary3, sizeof(boundary3)-1, "\r\n%s", boundary);
	pPostData=sid->PostData;
	memset(filename, 0, sizeof(filename));
	while ((strcmp(line, boundary2)!=0)&&(i<sid->dat->in_ContentLength)) {
		memset(line, 0, sizeof(line));
		j=0;
		while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<sid->dat->in_ContentLength)&&(j<sizeof(line)-1)) {
			line[j]=*pPostData;
			pPostData++;
			i++;
			j++;
		}
		/ * eat newline garbage * /
		while ((*pPostData=='\n')||(*pPostData=='\r')) {
			pPostData++;
			i++;
		}
		ptemp=line;
		if (strncmp(line, "Content-Disposition: form-data; ", 32)==0) {
			ptemp+=32;
			if (strncmp(ptemp, "name=\"userfile\"; ", 17)==0) {
				ptemp+=17;
				if (strncmp(ptemp, "filename=\"", 10)==0) {
					ptemp+=10;
					if (strrchr(ptemp, '\\')!=NULL) {
						ptemp=strrchr(ptemp, '\\')+1;
					}
					snprintf(filename, sizeof(filename)-1, "%s", ptemp);
					while (filename[strlen(filename)-1]=='\"') filename[strlen(filename)-1]='\0';
				}
			} else if (strncmp(ptemp, "name=\"location\"", 15)==0) {
				ptemp+=15;
				while ((*pPostData=='\n')||(*pPostData=='\r')) {
					pPostData++;
					i++;
				}
				j=0;
				while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<sid->dat->in_ContentLength)&&(j<sizeof(location)-1)) {
					location[j]=*pPostData;
					pPostData++;
					i++;
					j++;
				}
				if (strncmp(location, "/files/", 7)==0) {
					directory=location+6;
				}
			}
		}
		if (strncmp(line, "Content-Type: ", 14)==0) {
			snprintf(file, sizeof(file)-1, "/files%s", directory);
			if ((rc=fileperm(sid, A_INSERT, file, filename))<0) {
				if (rc==-2) {
					prints(sid, "<CENTER>File does not exist</CENTER>\r\n");
				} else if (rc==-3) {
					prints(sid, "<CENTER>File already exists</CENTER>\r\n");
				} else {
					prints(sid, "<CENTER>%s</CENTER>\r\n", lang.err_noaccess);
				}
				DEBUG_OUT(sid, "filerecv()");
				return -1;
			}
			ptemp=line+14;
			snprintf(file, sizeof(file)-1, "%s/%04d/files%s%s", config->dir_var_domains, sid->dat->did, directory, filename);
			fixslashes(file);
			fp=fopen(file, "wb");
			if (fp==NULL) {
				log_error(proc->N, "mod_files", __FILE__, __LINE__, 1, "ERROR: Cannot create file [%s].", file);
				DEBUG_OUT(sid, "filerecv()");
				return -1;
			}
			while ((strncmp(pPostData, boundary3, strlen(boundary3))!=0)&&(i<sid->dat->in_ContentLength)) {
				fprintf(fp, "%c", *pPostData);
				pPostData++;
				i++;
			}
			fclose(fp);
		}
	}
	if ((ptemp=getmimeenv(sid, "DESCRIPTION", &mimesize))!=NULL) {
		strncpy(description, ptemp, sizeof(description)-1);
		if (mimesize<strlen(description)) description[mimesize]='\0';
	}
	if (sql_queryf(&sqr, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filename), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, location))<0) return -1;
	if (sql_numtuples(&sqr)>0) {
		sql_updatef("DELETE FROM gw_files WHERE fileid = %d AND obj_did = %d", atoi(sql_getvalue(&sqr, 0, 0)), sid->dat->did);
	}
	sql_freeresult(&sqr);
	if (sql_query(&sqr, "SELECT max(fileid) FROM gw_files")<0) { DEBUG_OUT(sid, "filerecv()"); return -1; }
	fileid=atoi(sql_getvalue(&sqr, 0, 0))+1;
	if (fileid<1) fileid=1;
	sql_freeresult(&sqr);
	strftime(datebuf, sizeof(datebuf)-1, "%Y-%m-%d %H:%M:%S", localtime(&t));
	strcpy(query, "INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filename, filepath, filetype, uldate, lastdldate, numdownloads, description) VALUES (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", fileid, datebuf, datebuf, filerec.obj_uid, filerec.obj_gid, filerec.obj_did, filerec.obj_gperm, filerec.obj_operm);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filename));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, location));
	strncatf(query, sizeof(query)-strlen(query)-1, "'file', '%s', '%s', '0', ", datebuf, datebuf);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, description));
	if (sql_update(query)<0) { DEBUG_OUT(sid, "filerecv()"); return -1; }
	db_log_activity(sid, "files", fileid, "insert", "%s - %s uploaded file %d %s", sid->dat->in_RemoteAddr, sid->dat->username, fileid, filename);
	prints(sid, "<EMBED SRC=/nullgs/sounds/reminder.wav AUTOSTART=TRUE HIDDEN=true VOLUME=100>\r\n");
	prints(sid, "<CENTER>\r\nFile '%s%s' has been received.<BR />\r\n", location, filename);
	prints(sid, "[<A HREF=javascript:window.close()>Close Window</A>]</CENTER>\r\n");
	htpage_footer(sid);
	sid->dat->out_bodydone=1;
	flushbuffer(sid);
	DEBUG_OUT(sid, "filerecv()");
*/	return 0;
}

void fileedit(CONN *sid)
{
/*
	REC_FILE file;
	char filename[255];
	char *ptemp;
	char *directory;
	int fileid;
	int rc;
	FILE *fp;
	int ich;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "FILEID"))==NULL) return;
	fileid=atoi(ptemp);
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((rc=fileperm(sid, A_MODIFY, file.filepath, file.filename))<0) {
		if (rc==-2) {
			prints(sid, "<BR /><CENTER>File does not exist</CENTER><BR />\r\n");
		} else if (rc==-3) {
			prints(sid, "<BR /><CENTER>File already exists</CENTER><BR />\r\n");
		} else {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		}
		return;
	}
	prints(sid, "<CENTER>\r\n<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/filesave NAME=fileedit ENCTYPE=multipart/form-data>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=fileid VALUE='%d'>\r\n", file.fileid);
	prints(sid, "<TR><TH><A HREF=%s/fileinfoedit?fileid=%d&location=", sid->dat->in_ScriptName, fileid);
	printhex(sid, "%s", file.filepath);
	prints(sid, ">File %d - %s%s", fileid, str2html(sid, file.filepath), str2html(sid, file.filename));
	prints(sid, "</TH></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER><TEXTAREA WRAP=OFF NAME=filebody ROWS=23 COLS=79>");
	directory=file.filepath+7;
	snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s%s", config->dir_var_domains, sid->dat->did, directory, file.filename);
	fixslashes(filename);
	fp=fopen(filename, "rb");
	if (fp!=NULL) {
		while ((ich=getc(fp))!=EOF) {
			if (ich=='&') {
				prints(sid, "&amp;");
			} else if  (ich=='<') {
				prints(sid, "&lt;");
			} else {
				prints(sid, "%c", (unsigned char)ich);
			}
		}
		fclose(fp);
	}
	prints(sid, "</TEXTAREA></TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'><INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'></TD></TR>");
	prints(sid, "</FORM>\r\n</TABLE>\r\n</CENTER>\r\n");
*/
	return;
}

void filesave(CONN *sid)
{
/*
	REC_FILE file;
	char filename[255];
	char *ptemp;
	char *directory;
	char *filebody;
	unsigned int filesize=0;
	unsigned int mimesize;
	unsigned int i;
	int fileid;
	int rc;
	FILE *fp;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getmimeenv(sid, "FILEID", &mimesize))==NULL) return;
	fileid=atoi(ptemp);
	if ((filebody=getmimeenv(sid, "FILEBODY", &filesize))==NULL) return;
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return;
	}
	if ((rc=fileperm(sid, A_MODIFY, file.filepath, file.filename))<0) {
		if (rc==-2) {
			prints(sid, "<BR /><CENTER>File does not exist</CENTER><BR />\r\n");
		} else if (rc==-3) {
			prints(sid, "<BR /><CENTER>File already exists</CENTER><BR />\r\n");
		} else {
			prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		}
		return;
	}
	directory=file.filepath+7;
	snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s%s", config->dir_var_domains, sid->dat->did, directory, file.filename);
	fixslashes(filename);
	fp=fopen(filename, "wb");
	if (fp==NULL) {
		log_error(proc->N, "mod_files", __FILE__, __LINE__, 1, "ERROR: Cannot write to file [%s].", filename);
		prints(sid, "<BR /><CENTER>Error writing to file %s</CENTER><BR />\r\n", filename);
		return;
	}
	for (i=0;i<filesize;i++) {
		fprintf(fp, "%c", *filebody);
		filebody++;
	}
	fclose(fp);
	prints(sid, "<CENTER>file %d edited successfully</CENTER><BR />\r\n", file.fileid);
	db_log_activity(sid, "files", file.fileid, "modify", "%s - %s edited file %d %s", sid->dat->in_RemoteAddr, sid->dat->username, file.fileid, file.filename);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/fileinfoedit?fileid=%d&location=", sid->dat->in_ScriptName, file.fileid);
	printhex(sid, "%s", file.filepath);
	prints(sid, "\">\r\n");
*/
	return;
}
