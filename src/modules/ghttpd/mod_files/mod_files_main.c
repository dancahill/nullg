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
#define SRVMOD_MAIN 1
#include "mod_files.h"

void fileinfoedit(CONN *sid)
{
	REC_FILE file;
	char *ptemp;
	int editperms=0;
	int fileid;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if ((ptemp=getgetenv(sid, "FILEID"))==NULL) return;
	fileid=atoi(ptemp);
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", fileid);
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
	prints(sid, "function ConfirmDelete() {\r\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this file?\");\r\n");
	prints(sid, "}\r\n");
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<CENTER>\r\n<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/fileinfosave NAME=fileedit>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=fileid VALUE='%d'>\r\n", file.fileid);
	prints(sid, "<TR><TH COLSPAN=2>File %d</TH></TR>\r\n", fileid);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>File Name    </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=filename     value=\"%s\" SIZE=40 DISABLED></TD></TR>\r\n", str2html(sid, file.filename));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD NOWRAP>&nbsp;<B>File Location</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=filepath     value=\"%s\" SIZE=40 DISABLED></TD></TR>\r\n", str2html(sid, file.filepath));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2>&nbsp;<B>Description</B>&nbsp;</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=description ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\r\n", str2html(sid, file.description));
	prints(sid, "<TR><TH ALIGN=center COLSPAN=2>Permissions</TH></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\r\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, file.obj_uid, sid->dat->user_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\r\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, auth_priv(sid, "files"), file.obj_gid, sid->dat->user_did);
	prints(sid, "</SELECT></TD></TR>\r\n");
	if ((file.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "files")&A_ADMIN)) editperms=1;
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'>&nbsp;<B>Group Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\r\n", file.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\r\n", file.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\r\n", file.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'>&nbsp;<B>Other Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\r\n", file.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\r\n", file.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\r\n", file.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\r\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\r\n");
	if ((auth_priv(sid, "files")&A_DELETE)&&(fileid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\r\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\r\n");
	prints(sid, "</TD></TR>");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	if (strcmp(file.filetype, "file")==0) {
		if (strncmp(get_mime_type(file.filename), "text/", 5)==0) {
			prints(sid, "[<A HREF=%s/fileedit?fileid=%d&location=", sid->dat->in_ScriptName, file.fileid);
			printhex(sid, "%s", file.filepath);
			prints(sid, ">Edit File</A>]\r\n");
		}
		if (strlen(mod_config.filter_program)) {
			prints(sid, "[<A HREF=%s/filescan?fileid=%d&location=", sid->dat->in_ScriptName, file.fileid);
			printhex(sid, "%s", file.filepath);
			prints(sid, ">Scan File</A>]\r\n");
		}
	}
	prints(sid, "</CENTER>\r\n");
//	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.fileedit.filename.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	return;
}

void fileinfosave(CONN *sid)
{
	struct stat sb;
	REC_FILE file;
	char query[2048];
	char curdate[40];
	char filename[255];
	char *ptemp;
	time_t t;
	int fileid;
	int rc;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FILEID"))==NULL) return;
	fileid=atoi(ptemp);
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (auth_priv(sid, "files")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) file.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) file.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "files")&A_ADMIN)||(file.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) file.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) file.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "FILENAME"))!=NULL) snprintf(file.filename, sizeof(file.filename)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "FILEPATH"))!=NULL) snprintf(file.filepath, sizeof(file.filepath)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ULDATE"))!=NULL) file.uldate=time_sql2unix(ptemp);
	if ((ptemp=getpostenv(sid, "LASTDLDATE"))!=NULL) file.lastdldate=time_sql2unix(ptemp);
	if ((ptemp=getpostenv(sid, "NUMDOWNLOADS"))!=NULL) file.numdownloads=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DESCRIPTION"))!=NULL) snprintf(file.description, sizeof(file.description)-1, "%s", ptemp);
	t=time(NULL);
	strftime(curdate, sizeof(curdate)-1, "%Y-%m-%d %H:%M:%S", localtime(&t));
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "files")&A_DELETE)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if ((rc=fileperm(sid, A_DELETE, file.filepath, file.filename))<0) {
			if (rc==-2) {
				prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\r\n");
			} else if (rc==-3) {
				prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\r\n");
			} else {
				prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			}
			return;
		}
		memset(filename, 0, sizeof(filename));
		snprintf(filename, sizeof(filename)-1, "%s/%04d/files%s%s", config->dir_var_domains, sid->dat->user_did, file.filepath+6, file.filename);
		fixslashes(filename);
		if (stat(filename, &sb)==0) {
			if (sb.st_mode & S_IFDIR) {
				if (rmdir(filename)!=0) {
					prints(sid, "<CENTER><B>Directory could not be deleted.</B></CENTER><BR>\r\n");
					return;
				}
			} else {
				if (unlink(filename)!=0) {
					prints(sid, "<CENTER><B>File could not be deleted.</B></CENTER><BR>\r\n");
					return;
				}
			}
		}
		if (sql_updatef("DELETE FROM gw_files WHERE fileid = %d AND obj_did = %d", file.fileid, sid->dat->user_did)<0) return;
		prints(sid, "<CENTER>file %d deleted successfully</CENTER><BR>\r\n", file.fileid);
		db_log_activity(sid, "files", file.fileid, "delete", "%s - %s deleted file %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, file.fileid, file.filename);
	} else {
		if (!(auth_priv(sid, "files")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			return;
		}
		if ((rc=fileperm(sid, A_MODIFY, file.filepath, file.filename))<0) {
			if (rc==-2) {
				prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\r\n");
			} else if (rc==-3) {
				prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\r\n");
			} else {
				prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
			}
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_files SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, file.obj_uid, file.obj_gid, file.obj_gperm, file.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "filename = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, file.filename));
		strncatf(query, sizeof(query)-strlen(query)-1, "filepath = '%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, file.filepath));
		strncatf(query, sizeof(query)-strlen(query)-1, "uldate = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, file.uldate));
		strncatf(query, sizeof(query)-strlen(query)-1, "lastdldate = '%s', ", time_unix2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, file.lastdldate));
		strncatf(query, sizeof(query)-strlen(query)-1, "numdownloads = '%d', ", file.numdownloads);
		strncatf(query, sizeof(query)-strlen(query)-1, "description = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, file.description));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE fileid = %d AND obj_did = %d", file.fileid, sid->dat->user_did);
		if (sql_update(query)<0) return;
		prints(sid, "<CENTER>file %d modified successfully</CENTER><BR>\r\n", file.fileid);
		db_log_activity(sid, "files", file.fileid, "modify", "%s - %s modified file %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, file.fileid, file.filename);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s", sid->dat->in_ScriptName);
	printhex(sid, "%s", file.filepath);
	prints(sid, "\">\r\n");
	return;
}

void filescan(CONN *sid)
{
	REC_FILE file;
	FILE *fp;
	struct stat sb;
	int fileid;
	char line[512];
	char filename[255];
	char tempname[255];
	char *directory;
	char *ptemp;
	short int err;

	if (!(auth_priv(sid, "files")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\r\n", lang.err_noaccess);
		return;
	}
	if (strlen(mod_config.filter_program)==0) return;
	if ((ptemp=getgetenv(sid, "FILEID"))==NULL) return;
	fileid=atoi(ptemp);
	if (dbread_file(sid, 1, fileid, &file)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\r\n", fileid);
		return;
	}
	directory=file.filepath+7;
	snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s%s", config->dir_var_domains, sid->dat->user_did, directory, file.filename);
	fixslashes(filename);
	snprintf(tempname, sizeof(tempname)-1, "%s/scan-%d.tmp", config->dir_var_tmp, (int)(time(NULL)%999));
	fixslashes(tempname);
	prints(sid, "Scanning '%s'<BR>\r\n", filename);
	if (stat(filename, &sb)!=0) {
		prints(sid, "File not found");
		return;
	}
	if (sb.st_mode&S_IFDIR) {
		prints(sid, "Not a file");
		return;
	}
#ifdef WIN32
	snprintf(line, sizeof(line)-1, "\"%s\" \"%s\" > \"%s\"", mod_config.filter_program, filename, tempname);
#else
	snprintf(line, sizeof(line)-1, "%s %s > %s 2>&1", mod_config.filter_program, filename, tempname);
#endif
	flushbuffer(sid);
	err=sys_system(line);
	if (err>255) err=err>>8;
	if ((err>0)&&(err<4)) {
		prints(sid, "<FONT COLOR=red><B>INFECTED AND/OR POTENTIALLY DANGEROUS</B></FONT><BR>\r\n");
	} else if (err==0) {
		prints(sid, "<FONT COLOR=green><B>LOOKS OK TO ME</B></FONT><BR>\r\n");
	} else {
		prints(sid, "<FONT COLOR=orange><B>PROGRAM ERROR?</B></FONT><BR>\r\n");
	}
	prints(sid, "Scanner returned %d\r\n", err);
	if (err!=0) {
		prints(sid, "<HR><PRE>");
		if ((fp=fopen(tempname, "r"))!=NULL) {
			while (fgets(line, sizeof(line)-1, fp)!=NULL) {
				prints(sid, "%s", line);
			}
			fclose(fp);
		}
		prints(sid, "</PRE>");
	}
	unlink(tempname);
	return;
}

DllExport int mod_main(CONN *sid)
{
	if (dirlist(sid)==0) return 0;
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	if ((strncmp(sid->dat->in_RequestURI, "/fileul", 7)!=0)&&(strncmp(sid->dat->in_RequestURI, "/filerecv", 9)!=0)) {
		htpage_topmenu(sid, MENU_FILES);
		prints(sid, "<BR>\r\n");
	}
	if (strncmp(sid->dat->in_RequestURI, "/fileul", 7)==0) {
		fileul(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/filerecv", 9)==0) {
		filerecv(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/filescan", 9)==0) {
		filescan(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/fileinfoedit", 13)==0) {
		fileinfoedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/fileinfosave", 13)==0) {
		fileinfosave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/filemkdir", 10)==0) {
		filemkdir(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/filedirsave", 12)==0) {
		filedirsave(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/fileedit", 9)==0) {
		fileedit(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/filesave", 9)==0) {
		filesave(sid);
	}
	htpage_footer(sid);
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_files",		// mod_name
		3,			// mod_submenu
		"FILES",		// mod_menuname
		"default.png",		// mod_menupic
		"/files/~/",		// mod_menuuri
		"files",		// mod_menuperm
		"mod_main",		// fn_name
		"/file",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	conf_read();
	lang_read();
	if (mod_export_main(&newmod)!=0) return -1;
	return 0;
}
