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
#include "mod_files.h"
#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

int fileperm(CONN *sid, int perm, char *dir, char *file)
{
	struct stat sb;
	char filename[512];
	char subdir[512];
	char subfile[512];
	char *ptemp;
	int sqr;
	int tuples;
	int x;

	DEBUG_IN(sid, "fileperm()");
//	logerror(sid, __FILE__, __LINE__, "dir[%s]file[%s]", dir, file);
	if (strncmp(dir, "/files/", 7)!=0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	memset(filename, 0, sizeof(filename));
	ptemp=dir;
	if (strlen(ptemp)>=7) ptemp+=7;
	snprintf(filename, sizeof(filename)-1, "%s/%s%s", config->server_dir_var_files, ptemp, file);
	fixslashes(filename);
//	logerror(sid, __FILE__, __LINE__, "[%s]", filename);
	x=stat(filename, &sb);
	if ((perm==A_READ)&&(x!=0)) { DEBUG_OUT(sid, "fileperm()"); return -2; }
	if ((perm==A_INSERT)&&(x==0)) { DEBUG_OUT(sid, "fileperm()"); return -3; }
//	if (sb.st_mode&S_IFDIR)
	/* if A_ADMIN is true and the file exists, then permission is granted */
	if (auth_priv(sid, "files")&A_ADMIN) { DEBUG_OUT(sid, "fileperm()"); return 0; }
	if ((perm==A_READ)&&(!auth_priv(sid, "files")&A_READ)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((perm==A_INSERT)&&(!auth_priv(sid, "files")&A_INSERT)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((perm==A_MODIFY)&&(!auth_priv(sid, "files")&A_MODIFY)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((perm==A_DELETE)&&(!auth_priv(sid, "files")&A_DELETE)) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if ((strcmp(dir, "/files/")==0)&&((perm==A_INSERT)||(perm==A_DELETE))) { DEBUG_OUT(sid, "fileperm()"); return -1; }
	if (perm==A_READ) {
		if ((sqr=sql_queryf(sid, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s' AND (obj_uid = %d OR (obj_gid = %d AND obj_gperm>=1) OR obj_operm>=1)", file, dir, sid->dat->user_uid, sid->dat->user_gid))<0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
		tuples=sql_numtuples(sqr);
		sql_freeresult(sqr);
	} else if (perm==A_MODIFY) {
		if ((sqr=sql_queryf(sid, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s' AND (obj_uid = %d OR (obj_gid = %d AND obj_gperm>=2) OR obj_operm>=2)", file, dir, sid->dat->user_uid, sid->dat->user_gid))<0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
		tuples=sql_numtuples(sqr);
		sql_freeresult(sqr);
	} else if ((perm==A_INSERT)||(perm==A_DELETE)) {
		memset(subdir, 0, sizeof(subdir));
		memset(subfile, 0, sizeof(subfile));
		snprintf(subdir, sizeof(subdir)-1, "%s", dir);
		ptemp=subdir;
 		while (*ptemp) { if (*ptemp=='\\') *ptemp='/'; ptemp++; }
		while (subdir[strlen(subdir)-1]=='/') subdir[strlen(subdir)-1]='\0';
		if ((ptemp=strrchr(subdir, '/'))!=NULL) {
			ptemp++;
			snprintf(subfile, sizeof(subfile)-1, "%s", ptemp);
			*ptemp='\0';
		}
		if ((sqr=sql_queryf(sid, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s' AND (obj_uid = %d OR (obj_gid = %d AND obj_gperm>=2) OR obj_operm>=2)", subfile, subdir, sid->dat->user_uid, sid->dat->user_gid))<0) { DEBUG_OUT(sid, "fileperm()"); return -1; }
		tuples=sql_numtuples(sqr);
		sql_freeresult(sqr);
	} else {
		tuples=0;
	}
	DEBUG_OUT(sid, "fileperm()");
	if (tuples!=1) return -1;
	return 0;
}

int dirlist(CONN *sid)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	struct stat sb;
	DIR  *handle;
	REC_FILE file;
	char filename[1024];
	char index[1024];
	char query[2048];
	char showfile[1024];
	char timebuf[40];
	char uri[512];
	char uri2[512];
	char *directory;
	char *ptemp;
	time_t t;
	int fileid;
	int i;
	int isvalid;
	int rc;
	int sqr;
	int sqr2;
	int def_uid;
	int def_gid;
	int def_gperm;
	int def_operm;

	if (!(auth_priv(sid, "files")&A_READ)) {
		return -1;
	}
	memset(uri, 0, sizeof(uri));
	snprintf(uri, sizeof(uri)-1, "%s", sid->dat->in_RequestURI);
	decodeurl(uri);
	if (strncmp(uri, "/files/", 7)!=0) {
		return -1;
	}
	directory=uri+7;
	snprintf(filename, sizeof(filename)-1, "%s/%s", config->server_dir_var_files, directory);
	decodeurl(filename);
	fixslashes(filename);
	while ((filename[strlen(filename)-1]=='\\')||(filename[strlen(filename)-1]=='/')) { filename[strlen(filename)-1]='\0'; };
	if (strstr(filename, "..")!=NULL) return -1;
	snprintf(index, sizeof(index)-1, "%s/%s/index.html", config->server_dir_var_htdocs, directory);
	decodeurl(index);
	fixslashes(index);
	if (stat(index, &sb)==0) {
		filesend(sid, index);
		return 0;
	}
	if (stat(filename, &sb)!=0) {
		send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
		htpage_topmenu(sid, MENU_FILES);
		prints(sid, "<BR><CENTER>File or folder does not exist</CENTER><BR>\n");
		return 0;
	}
	memset((char *)&file, 0, sizeof(file));
	snprintf(file.filepath, sizeof(file.filepath)-1, "%s", uri);
	decodeurl(file.filepath);
	ptemp=file.filepath;
 	while (*ptemp) { if (*ptemp=='\\') *ptemp='/'; ptemp++; }
	while (file.filepath[strlen(file.filepath)-1]=='/') file.filepath[strlen(file.filepath)-1]='\0';
	if ((ptemp=strrchr(file.filepath, '/'))!=NULL) {
		ptemp++;
		snprintf(file.filename, sizeof(file.filename)-1, "%s", ptemp);
		*ptemp='\0';
	}
	if ((strlen(file.filepath)>1)&&(!(auth_priv(sid, "files")&A_ADMIN))) {
		if ((rc=fileperm(sid, A_READ, file.filepath, file.filename))<0) {
			send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
			htpage_topmenu(sid, MENU_FILES);
			if (rc==-2) {
				prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\n");
			} else if (rc==-3) {
				prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\n");
			} else {
				prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			}
			return 0;
		}
	}
	if (!(sb.st_mode & S_IFDIR)) {
		return filesend(sid, filename);
	}
	t=time(NULL);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime(&t));
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_FILES);
	isvalid=1;

	memset(uri2, 0, sizeof(uri2));
	snprintf(uri2, sizeof(uri2)-1, "%s", uri);
	while (uri2[strlen(uri2)-1]=='/') uri2[strlen(uri2)-1]='\0';
	if ((ptemp=strrchr(uri2, '/'))!=NULL) {
		*ptemp++='\0';
	} else {
		ptemp=uri2;
	}
	def_uid=1;
	def_gid=1;
	def_gperm=1;
	def_operm=1;
	if ((sqr2=sql_queryf(sid, "SELECT obj_uid, obj_gid, obj_gperm, obj_operm FROM gw_files WHERE filepath = '%s/' AND filename = '%s' AND filetype = 'dir'", uri2, ptemp))>-1) {
		if (sql_numtuples(sqr2)==1) {
			def_uid=atoi(sql_getvalue(sqr2, 0, 0));
			def_gid=atoi(sql_getvalue(sqr2, 0, 1));
			def_gperm=atoi(sql_getvalue(sqr2, 0, 2));
			def_operm=atoi(sql_getvalue(sqr2, 0, 3));
		}
		sql_freeresult(sqr2);
	}

	if ((sqr=sql_queryf(sid, "SELECT fileid, filename, lastdldate, numdownloads, description, filetype FROM gw_files WHERE filepath = '%s' ORDER BY filetype, filename ASC", uri))<0) return -1;
//	if (!RunAsCGI) pthread_mutex_lock(&Lock.FileList);
	handle=opendir(filename);
	while ((dentry=readdir(handle))!=NULL) {
		snprintf(filename, sizeof(filename)-1, "%s/%s%s", config->server_dir_var_files, directory, dentry->d_name);
		fixslashes(filename);
		stat(filename, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if (strcmp("..", dentry->d_name)==0) continue;
		for (i=0;i<sql_numtuples(sqr);i++) {
			if (strcmp(dentry->d_name, sql_getvalue(sqr, i, 1))==0) break;
		}
		if (i==sql_numtuples(sqr)) {
			if ((sqr2=sql_query(sid, "SELECT max(fileid) FROM gw_files"))<0) {
				sql_freeresult(sqr);
				closedir(handle);
//				if (!RunAsCGI) pthread_mutex_unlock(&Lock.FileList);
				return -1;
			}
			fileid=atoi(sql_getvalue(sqr2, 0, 0))+1;
			if (fileid<1) fileid=1;
			sql_freeresult(sqr2);
			strftime(timebuf, sizeof(timebuf)-1, "%Y-%m-%d %H:%M:%S", localtime(&t));
			strcpy(query, "INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, filename, filepath, filetype, uldate, lastdldate, numdownloads, description) VALUES (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", fileid, timebuf, timebuf, def_uid, def_gid, def_gperm, def_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, dentry->d_name));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, uri));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", sb.st_mode&S_IFDIR?"dir":"file");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '0', '')", timebuf, timebuf);
			if (sql_update(sid, query)<0) {
				logerror(sid, __FILE__, __LINE__, "%s - ERROR: Cannot upload file [%s].", sid->dat->in_RemoteAddr, dentry->d_name);
			} else {
//				db_log_activity(sid, 1, "files", fileid, "insert", "%s - system uploaded file %d %s", sid->dat->in_RemoteAddr, fileid, dentry->d_name);
			}
			isvalid=0;
		}
	}
	closedir(handle);
//	if (!RunAsCGI) pthread_mutex_unlock(&Lock.FileList);
	if (!isvalid) {
		sql_freeresult(sqr);
		if ((sqr=sql_queryf(sid, "SELECT fileid, filename, lastdldate, numdownloads, description, filetype FROM gw_files WHERE filepath = '%s' ORDER BY filetype, filename ASC", uri))<0) return -1;
	}
	prints(sid, "<BR>\r\n<CENTER>\n<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=2 CELLSPACING=1 WIDTH=95%%>\r\n", proc->config.colour_tabletrim);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=%d>", config->colour_th, (auth_priv(sid, "files")&A_MODIFY)?5:4);
	prints(sid, "<FONT COLOR=%s>&nbsp;Index of ", config->colour_thtext);
	memset(uri2, 0, sizeof(uri2));
	i=0;
	if (uri[i]=='/') {
		uri2[i]=uri[i];
		prints(sid, "/");
		i++;
		while (uri[i]) {
			if (uri[i]=='/') {
				prints(sid, "<A HREF=\"%s/\" STYLE='color: %s'>%s</A>/", uri2, config->colour_thtext, strrchr(uri2, '/')+1);
			}
			uri2[i]=uri[i];
			i++;
		}
	} else {
		prints(sid, "%s", uri);
	}
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldname);
	if (auth_priv(sid, "files")&A_MODIFY) {
		prints(sid, "<TD>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>");
	}
	prints(sid, "<TD width=20%%><B>&nbsp;Filename&nbsp;</B></TD><TD width=10%%><B>&nbsp;Date&nbsp;</B></TD>");
	prints(sid, "<TD width=10%%><B>&nbsp;Size&nbsp;</B></TD><TD width=60%%><B>&nbsp;Description&nbsp;</B></TD></TR>\n");
	if ((strncmp(uri, "/files/", 7)==0)&&(strlen(file.filepath)>1)) {
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		if (auth_priv(sid, "files")&A_MODIFY) {
			prints(sid, "<TD>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>");
		}
		prints(sid, "<TD COLSPAN=4><A HREF=%s", sid->dat->in_ScriptName);
		printhex(sid, "%s", file.filepath);
		prints(sid, "><IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-foldero.gif HEIGHT=16 WIDTH=16> Parent Directory</A></TD>\n");
	}
	for (i=0;i<sql_numtuples(sqr);i++) {
		snprintf(filename, sizeof(filename)-1, "%s/%s%s", config->server_dir_var_files, directory, sql_getvalue(sqr, i, 1));
		fixslashes(filename);
		isvalid=1;
		if (stat(filename, &sb)!=0) isvalid=0;
		// we might wanna clean up, purge dead file records, but...
		if (!isvalid) continue;
		memset(timebuf, 0, sizeof(timebuf));
		if (isvalid) strftime(timebuf, sizeof(timebuf)-1, "%b %d %Y %H:%M", localtime(&sb.st_mtime));
		memset(showfile, 0, sizeof(showfile));
		snprintf(showfile, sizeof(showfile)-1, "%s", sql_getvalue(sqr, i, 1));
		prints(sid, "<TR BGCOLOR=%s>", config->colour_fieldval);
		if (auth_priv(sid, "files")&A_MODIFY) {
			prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP><A HREF=%s/fileinfoedit?fileid=%d&location=", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
			printhex(sid, "%s", sid->dat->in_RequestURI);
			prints(sid, ">edit</A></TD>");
		}
		prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP style=\"cursor:hand\" onClick=window.location.href=\"");
		printhex(sid, "%s", showfile);
		prints(sid, "%s\" TITLE=\"%s\">", (strcmp(sql_getvalue(sqr, i, 5), "dir")==0)?"/":"", showfile);
		prints(sid, "<A HREF=");
		printhex(sid, "%s", showfile);
		if (strcmp(sql_getvalue(sqr, i, 5), "dir")==0) {
			prints(sid, "/ TITLE=\"%s\"><IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-folder.gif HEIGHT=16 WIDTH=16>", showfile);
		} else {
			prints(sid, " TITLE=\"%s\"><IMG ALIGN=TOP BORDER=0 SRC=/groupware/images/file-default.gif HEIGHT=16 WIDTH=16>", showfile);
		}
		prints(sid, " %.25s%s</A></TD>", sql_getvalue(sqr, i, 1), strlen(sql_getvalue(sqr, i, 1))>25?"..":"");
		if (isvalid) {
			prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP>%s</TD>", timebuf);
			if (sb.st_size>1048576) {
				prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP>%10.1f M</TD>", (float)sb.st_size/1048576.0);
			} else {
				prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP>%10.1f K</TD>", (float)sb.st_size/1024.0);
			}
		} else {
			prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP>&nbsp;</TD>");
			prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP>&nbsp;</TD>");
		}
		if (i>-1) {
			prints(sid, "<TD ALIGN=left VALIGN=top><PRE>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, 4)));
		} else {
			prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP>&nbsp;</TD>");
		}
		prints(sid, "</TR>\n");
	}
	sql_freeresult(sqr);
	prints(sid, "</TABLE>\n</CENTER>\n</BODY></HTML>\n");
	return 0;
}

int fileul(CONN *sid)
{
	REC_FILE file;
	char directory[255];
	char filename[255];

	htpage_header(sid, "File Upload");
	if (!(auth_priv(sid, "files")&A_INSERT)) {
		return -1;
	}
	if (dbread_file(sid, 2, 0, &file)!=0) {
		return -1;
	}
	if (getgetenv(sid, "LOCATION")!=NULL) {
		snprintf(directory, sizeof(directory)-1, "%s", getgetenv(sid, "LOCATION"));
	}
	snprintf(filename, sizeof(filename)-1, "%s/%s", config->server_dir_var_files, directory+7);
	decodeurl(filename);
	fixslashes(filename);
	while ((filename[strlen(filename)-1]=='\\')||(filename[strlen(filename)-1]=='/')) { filename[strlen(filename)-1]='\0'; };
	if (strstr(filename, "..")!=NULL) return -1;
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "var x=0;\n");
	prints(sid, "function UploadStatus() {\n");
	prints(sid, "	var output=\"\\nPlease wait while the file is uploaded \";\n");
	prints(sid, "	if (x==0) {\n");
	prints(sid, "		output+=\"/\"\n");
	prints(sid, "		x=1;\n");
	prints(sid, "	} else if (x==1) {\n");
	prints(sid, "		output+=\"-\"\n");
	prints(sid, "		x=2;\n");
	prints(sid, "	} else if (x==2) {\n");
	prints(sid, "		output+=\"\\\\\"\n");
	prints(sid, "		x=3;\n");
	prints(sid, "	} else if (x==3) {\n");
	prints(sid, "		output+=\"|\"\n");
	prints(sid, "		x=0;\n");
	prints(sid, "	}\n");
	prints(sid, "	document.getElementById('Text').innerHTML=output;\n");
	prints(sid, "	window.setTimeout('UploadStatus()', 500);\n");
	prints(sid, "}\n");
	prints(sid, "function waitForCompletion() {\n");
	prints(sid, "	UploadStatus();\n");
	prints(sid, "	document.filesend.submit();\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<BR>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/filerecv NAME=filesend ENCTYPE=multipart/form-data onSubmit=waitForCompletion();>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=location value='%s'>\n", getgetenv(sid, "LOCATION"));
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>Uploading new file to '%s'</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, directory);
	prints(sid, "<TR BGCOLOR=%s><TD><B>&nbsp;File&nbsp;</B></TD><TD><INPUT TYPE=file NAME=userfile SIZE=35></TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2>&nbsp;<B>Description</B>&nbsp;</TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=description ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, file.description));
	if ((file.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "files")&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, file.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, file.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD NOWRAP STYLE='padding:0px'>&nbsp;<B>Group Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", file.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", file.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", file.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'>&nbsp;<B>Other Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", file.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", file.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", file.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>\n");
	prints(sid, "<FONT FACE=\"Courier\">\n");
	prints(sid, "<span id=\"Text\">\n");
	prints(sid, "<INPUT TYPE=submit VALUE='Upload'>\n");
	prints(sid, "</span>\n");
	prints(sid, "</FONT>\n");
	prints(sid, "</TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "</BODY></HTML>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.filesend.userfile.focus();\n// -->\n</SCRIPT>\n");
	return 0;
}

int filemkdir(CONN *sid)
{
	REC_FILE file;
	char directory[255];
	char filename[255];

	if (!(auth_priv(sid, "files")&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return -1;
	}
	if (dbread_file(sid, 2, 0, &file)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return -1;
	}
	snprintf(file.filename, sizeof(file.filename)-1, "New Folder");
	if (getgetenv(sid, "LOCATION")!=NULL) {
		snprintf(directory, sizeof(directory)-1, "%s", getgetenv(sid, "LOCATION"));
	}
	snprintf(filename, sizeof(filename)-1, "%s/%s", config->server_dir_var_files, directory+7);
	decodeurl(filename);
	fixslashes(filename);
	while ((filename[strlen(filename)-1]=='\\')||(filename[strlen(filename)-1]=='/')) { filename[strlen(filename)-1]='\0'; };
	if (strstr(filename, "..")!=NULL) return -1;
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/filedirsave?location=%s NAME=filedirsave>\n", sid->dat->in_ScriptName, directory);
//	prints(sid, "<FORM METHOD=POST ACTION=%s/filedirsave NAME=filedirsave>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=filepath value='%s'>\n", getgetenv(sid, "LOCATION"));
	prints(sid, "<TR><TH BGCOLOR=%s COLSPAN=2><FONT COLOR=%s>Adding folder to '%s'</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, directory);
	prints(sid, "<TR BGCOLOR=%s><TD>&nbsp;<B>Folder Name</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=filename SIZE=40 VALUE='%s'></TD></TR>\n", config->colour_editform, file.filename);
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2>&nbsp;<B>Description</B>&nbsp;</TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=description ROWS=5 COLS=50></TEXTAREA></TD></TR>\n", config->colour_editform);
	if ((file.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "files")&A_ADMIN)) {
		prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, file.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, file.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'>&nbsp;<B>Group Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", file.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", file.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", file.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'>&nbsp;<B>Other Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", file.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", file.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", file.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=submit VALUE='Create Folder'></TD></TR>\n");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	prints(sid, "</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.filedirsave.filename.focus();\n// -->\n</SCRIPT>\n");
	prints(sid, "</BODY></HTML>\n");
	return 0;
}

void fileinfoedit(CONN *sid)
{
	REC_FILE file;
	int editperms=0;
	int fileid;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (getgetenv(sid, "FILEID")==NULL) return;
	fileid=atoi(getgetenv(sid, "FILEID"));
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", fileid);
		return;
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this file?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/fileinfosave NAME=fileedit>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=fileid VALUE='%d'>\n", file.fileid);
	prints(sid, "<TR BGCOLOR=%s><TH COLSPAN=2><FONT COLOR=%s>File %d</FONT></TH></TR>\n", config->colour_th, config->colour_thtext, fileid);
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>File Name    </B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=filename     value=\"%s\" SIZE=40 DISABLED></TD></TR>\n", config->colour_editform, str2html(sid, file.filename));
	prints(sid, "<TR BGCOLOR=%s><TD NOWRAP>&nbsp;<B>File Location</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=filepath     value=\"%s\" SIZE=40 DISABLED></TD></TR>\n", config->colour_editform, str2html(sid, file.filepath));
	prints(sid, "<TR BGCOLOR=%s><TD COLSPAN=2>&nbsp;<B>Description</B>&nbsp;</TD></TR>\n", config->colour_editform);
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=HARD NAME=description ROWS=5 COLS=50>%s</TEXTAREA></TD></TR>\n", config->colour_editform, str2html(sid, file.description));
	prints(sid, "<TR BGCOLOR=%s><TH ALIGN=center COLSPAN=2><FONT COLOR=%s>Permissions</FONT></TH></TR>\n", config->colour_th, config->colour_thtext);
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
	htselect_user(sid, file.obj_uid);
	prints(sid, "</SELECT></TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>", config->colour_editform);
	prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "files")&A_ADMIN)?"":" DISABLED");
	htselect_group(sid, file.obj_gid);
	prints(sid, "</SELECT></TD></TR>\n");
	if ((file.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "files")&A_ADMIN)) editperms=1;
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'>&nbsp;<B>Group Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s%s>None\n", file.obj_gperm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s%s>Read\n", file.obj_gperm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s%s>Write\n", file.obj_gperm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD STYLE='padding:0px'>&nbsp;<B>Other Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n", config->colour_editform);
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s%s>None\n", file.obj_operm==0?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s%s>Read\n", file.obj_operm==1?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s%s>Write\n", file.obj_operm==2?" CHECKED":"", editperms?"":" DISABLED");
	prints(sid, "</TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2>");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'>\n");
	if ((auth_priv(sid, "files")&A_DELETE)&&(fileid!=0)) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "<INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'>\n");
	prints(sid, "</TD></TR>");
	prints(sid, "</FORM>\n");
	prints(sid, "</TABLE>\n");
	if ((strcmp(file.filetype, "file")==0)&&(strncmp(get_mime_type(file.filename), "text/", 5)==0)) {
		prints(sid, "[<A HREF=%s/fileedit?fileid=%d&location=", sid->dat->in_ScriptName, file.fileid);
		printhex(sid, "%s", file.filepath);
		prints(sid, ">Edit File</A>]\n");
	}
	prints(sid, "</CENTER>\n");
//	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.fileedit.filename.focus();\n// -->\n</SCRIPT>\n");
	return;
}

int filerecv(CONN *sid)
{
	REC_FILE filerec;
	char query[2048];
	char datebuf[40];
	char description[255];
	int sqr;
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
	prints(sid, "<CENTER>\n");
	prints(sid, "<TABLE BGCOLOR=%s BORDER=0 CELLPADDING=0 CELLSPACING=1 WIDTH=100%%><TR><TD>\r\n", proc->config.colour_tabletrim);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=left>&nbsp;</TD><TD ALIGN=right>&nbsp;</TD></TR>\n", config->colour_topmenu);
	prints(sid, "</TABLE>\n</TD></TR></TABLE>\n<BR>\n");
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
	if ((auth_priv(sid, "files")&A_ADMIN)||(filerec.obj_uid==sid->dat->user_uid)) {
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
	/* get the mime boundary */
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<sid->dat->in_ContentLength)&&(j<sizeof(boundary)-1)) {
		boundary[j]=*pPostData;
		pPostData++;
		i++;
		j++;
	}
	/* eat newline garbage */
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
		/* eat newline garbage */
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
					prints(sid, "<CENTER>File does not exist</CENTER>\n");
				} else if (rc==-3) {
					prints(sid, "<CENTER>File already exists</CENTER>\n");
				} else {
					prints(sid, "<CENTER>%s</CENTER>\n", ERR_NOACCESS);
				}
				DEBUG_OUT(sid, "filerecv()");
				return -1;
			}
			ptemp=line+14;
			snprintf(file, sizeof(file)-1, "%s%s%s", config->server_dir_var_files, directory, filename);
			fixslashes(file);
			fp=fopen(file, "wb");
			if (fp==NULL) {
				logerror(sid, __FILE__, __LINE__, "ERROR: Cannot create file [%s].", file);
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
	strftime(datebuf, sizeof(datebuf)-1, "%Y-%m-%d %H:%M:%S", localtime(&t));
	if ((sqr=sql_query(sid, "SELECT max(fileid) FROM gw_files"))<0) { DEBUG_OUT(sid, "filerecv()"); return -1; }
	fileid=atoi(sql_getvalue(sqr, 0, 0))+1;
	if (fileid<1) fileid=1;
	sql_freeresult(sqr);
	strcpy(query, "INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, filename, filepath, filetype, uldate, lastdldate, numdownloads, description) VALUES (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", fileid, datebuf, datebuf, filerec.obj_uid, filerec.obj_gid, filerec.obj_gperm, filerec.obj_operm);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, filename));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, location));
	strncatf(query, sizeof(query)-strlen(query)-1, "'file', '%s', '%s', '0', ", datebuf, datebuf);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, description));
	if (sql_update(sid, query)<0) { DEBUG_OUT(sid, "filerecv()"); return -1; }
	db_log_activity(sid, 1, "files", fileid, "insert", "%s - %s uploaded file %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, fileid, filename);
	prints(sid, "<BGSOUND SRC=/groupware/sounds/reminder.wav LOOP=1>\n");
	prints(sid, "<CENTER>\nFile '%s%s' has been received.<BR>\n", location, filename);
	prints(sid, "[<A HREF=javascript:window.close()>Close Window</A>]</CENTER>\n", sid->dat->in_ScriptName);
	htpage_footer(sid);
	sid->dat->out_bodydone=1;
	flushbuffer(sid);
	DEBUG_OUT(sid, "filerecv()");
	return 0;
}

int filedirsave(CONN *sid)
{
	REC_FILE filerec;
	char file[1024];
	char query[2048];
	char datebuf[40];
	char *ptemp;
	int rc;
	int sqr;
	int fileid;
	time_t t;

	if (!(auth_priv(sid, "files")&A_INSERT)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return -1;
	}
	if (dbread_file(sid, 2, 0, &filerec)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return -1;
	}
	if (auth_priv(sid, "files")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) filerec.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) filerec.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "files")&A_ADMIN)||(filerec.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) filerec.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) filerec.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "FILENAME"))!=NULL) snprintf(filerec.filename, sizeof(filerec.filename)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "FILEPATH"))!=NULL) snprintf(filerec.filepath, sizeof(filerec.filepath)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "ULDATE"))!=NULL) filerec.uldate=time_sql2unix(ptemp);
	if ((ptemp=getpostenv(sid, "LASTDLDATE"))!=NULL) filerec.lastdldate=time_sql2unix(ptemp);
	if ((ptemp=getpostenv(sid, "NUMDOWNLOADS"))!=NULL) filerec.numdownloads=atoi(ptemp);
	if ((ptemp=getpostenv(sid, "DESCRIPTION"))!=NULL) snprintf(filerec.description, sizeof(filerec.description)-1, "%s", ptemp);
	if ((rc=fileperm(sid, A_INSERT, filerec.filepath, filerec.filename))<0) {
		if (rc==-2) {
			prints(sid, "<CENTER>Directory does not exist</CENTER><BR>\n");
		} else if (rc==-3) {
			prints(sid, "<CENTER>Directory already exists</CENTER><BR>\n");
		} else {
			prints(sid, "<CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		}
		return -1;
	}
	fixslashes(filerec.filename);
	fixslashes(filerec.filepath);
	ptemp=filerec.filepath;
 	while (*ptemp) { if (*ptemp=='\\') *ptemp='/'; ptemp++; }
	t=time(NULL);
	strftime(datebuf, sizeof(datebuf)-1, "%Y-%m-%d %H:%M:%S", localtime(&t));
	memset(file, 0, sizeof(file));
	snprintf(file, sizeof(file)-1, "%s%s%s", config->server_dir_var_files, filerec.filepath+6, filerec.filename);
	if (strstr(file, "..")!=NULL) return -1;
	fixslashes(file);
#ifdef WIN32
	if (mkdir(file)!=0) {
#else
	if (mkdir(file, 0755)!=0) {
#endif
		prints(sid, "<BR><CENTER>Error creating folder.</CENTER><BR>\n");
		return -1;
	}
	if ((sqr=sql_query(sid, "SELECT max(fileid) FROM gw_files"))<0) return -1;
	fileid=atoi(sql_getvalue(sqr, 0, 0))+1;
	if (fileid<1) fileid=1;
	sql_freeresult(sqr);
	strcpy(query, "INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_gperm, obj_operm, filename, filepath, filetype, uldate, lastdldate, numdownloads, description) VALUES (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', ", fileid, datebuf, datebuf, filerec.obj_uid, filerec.obj_gid, filerec.obj_gperm, filerec.obj_operm);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, filerec.filename));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(sid, filerec.filepath));
	strncatf(query, sizeof(query)-strlen(query)-1, "'dir', '%s', '%s', '0', ", datebuf, datebuf);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(sid, filerec.description));
	if (sql_update(sid, query)<0) return -1;
	db_log_activity(sid, 1, "filefolders", fileid, "insert", "%s - %s created file folder %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, fileid, file);
	prints(sid, "<CENTER>\n");
	prints(sid, "File folder '%s%s' has been created.", filerec.filepath, filerec.filename);
	prints(sid, "</CENTER>\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s%s%s/\">\n", sid->dat->in_ScriptName, filerec.filepath, filerec.filename);
	return 0;
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
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "FILEID"))==NULL) return;
	fileid=atoi(ptemp);
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
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
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((rc=fileperm(sid, A_DELETE, file.filepath, file.filename))<0) {
			if (rc==-2) {
				prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\n");
			} else if (rc==-3) {
				prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\n");
			} else {
				prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			}
			return;
		}
		memset(filename, 0, sizeof(filename));
		snprintf(filename, sizeof(filename)-1, "%s%s%s", config->server_dir_var_files, file.filepath+6, file.filename);
		fixslashes(filename);
		if (stat(filename, &sb)==0) {
			if (sb.st_mode & S_IFDIR) {
				if (rmdir(filename)!=0) {
					prints(sid, "<CENTER><B>Directory could not be deleted.</B></CENTER><BR>\n");
					return;
				}
			} else {
				if (unlink(filename)!=0) {
					prints(sid, "<CENTER><B>File could not be deleted.</B></CENTER><BR>\n");
					return;
				}
			}
		}
		if (sql_updatef(sid, "DELETE FROM gw_files WHERE fileid = %d", file.fileid)<0) return;
		prints(sid, "<CENTER>file %d deleted successfully</CENTER><BR>\n", file.fileid);
		db_log_activity(sid, 1, "files", file.fileid, "delete", "%s - %s deleted file %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, file.fileid, file.filename);
	} else {
		if (!(auth_priv(sid, "files")&A_MODIFY)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((rc=fileperm(sid, A_MODIFY, file.filepath, file.filename))<0) {
			if (rc==-2) {
				prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\n");
			} else if (rc==-3) {
				prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\n");
			} else {
				prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			}
			return;
		}
		snprintf(query, sizeof(query)-1, "UPDATE gw_files SET obj_mtime = '%s', obj_uid = '%d', obj_gid = '%d', obj_gperm = '%d', obj_operm = '%d', ", curdate, file.obj_uid, file.obj_gid, file.obj_gperm, file.obj_operm);
		strncatf(query, sizeof(query)-strlen(query)-1, "filename = '%s', ", str2sql(sid, file.filename));
		strncatf(query, sizeof(query)-strlen(query)-1, "filepath = '%s', ", str2sql(sid, file.filepath));
		strncatf(query, sizeof(query)-strlen(query)-1, "uldate = '%s', ", time_unix2sql(sid, file.uldate));
		strncatf(query, sizeof(query)-strlen(query)-1, "lastdldate = '%s', ", time_unix2sql(sid, file.lastdldate));
		strncatf(query, sizeof(query)-strlen(query)-1, "numdownloads = '%d', ", file.numdownloads);
		strncatf(query, sizeof(query)-strlen(query)-1, "description = '%s'", str2sql(sid, file.description));
		strncatf(query, sizeof(query)-strlen(query)-1, " WHERE fileid = %d", file.fileid);
		if (sql_update(sid, query)<0) return;
		prints(sid, "<CENTER>file %d modified successfully</CENTER><BR>\n", file.fileid);
		db_log_activity(sid, 1, "files", file.fileid, "modify", "%s - %s modified file %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, file.fileid, file.filename);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s", sid->dat->in_ScriptName);
	printhex(sid, "%s", file.filepath);
	prints(sid, "\">\n");
	return;
}

void fileedit(CONN *sid)
{
	REC_FILE file;
	char filename[255];
	char *ptemp;
	char *directory;
	int fileid;
	int rc;
	FILE *fp;
	int ich;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "FILEID"))==NULL) return;
	fileid=atoi(ptemp);
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((rc=fileperm(sid, A_MODIFY, file.filepath, file.filename))<0) {
		if (rc==-2) {
			prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\n");
		} else if (rc==-3) {
			prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\n");
		} else {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		}
		return;
	}
	prints(sid, "<CENTER>\n<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/filesave NAME=fileedit ENCTYPE=multipart/form-data>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=fileid VALUE='%d'>\n", file.fileid);
	prints(sid, "<TR BGCOLOR=%s><TH><FONT COLOR=%s>", config->colour_th, config->colour_thtext);
	prints(sid, "<A HREF=%s/fileinfoedit?fileid=%d&location=", sid->dat->in_ScriptName, fileid, config->colour_thtext, fileid);
	printhex(sid, "%s", file.filepath);
	prints(sid, " STYLE='color: %s'>File %d - ", config->colour_thtext, fileid);
	prints(sid, "%s%s", str2html(sid, file.filepath), str2html(sid, file.filename));
	prints(sid, "</FONT></TH></TR>\n");
	prints(sid, "<TR BGCOLOR=%s><TD ALIGN=CENTER><TEXTAREA WRAP=OFF NAME=filebody ROWS=23 COLS=79>", config->colour_editform);
	directory=file.filepath+7;
	snprintf(filename, sizeof(filename)-1, "%s/%s%s", config->server_dir_var_files, directory, file.filename);
	fixslashes(filename);
	fp=fopen(filename, "rb");
	if (fp!=NULL) {
		while ((ich=getc(fp))!=EOF) {
//			if (ich>127) logerror(sid, __FILE__, __LINE__, "high ascii [%d][%c].", ich, (unsigned char)ich);
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
	prints(sid, "</TEXTAREA></TD></TR>\n");
	prints(sid, "<TR><TD ALIGN=CENTER><INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Save'><INPUT TYPE=RESET CLASS=frmButton NAME=Reset VALUE='Reset'></TD></TR>");
	prints(sid, "</FORM>\n</TABLE>\n</CENTER>\n");
	return;
}

void filesave(CONN *sid)
{
	REC_FILE file;
	char filename[255];
	char *ptemp;
	char *directory;
	unsigned char *filebody;
	int filesize=0;
	int fileid;
	int rc;
	int i;
	int mimesize;
	FILE *fp;

	if (!(auth_priv(sid, "files")&A_MODIFY)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getmimeenv(sid, "FILEID", &mimesize))==NULL) return;
	fileid=atoi(ptemp);
	if ((filebody=getmimeenv(sid, "FILEBODY", &filesize))==NULL) return;
	if (dbread_file(sid, 2, fileid, &file)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((rc=fileperm(sid, A_MODIFY, file.filepath, file.filename))<0) {
		if (rc==-2) {
			prints(sid, "<BR><CENTER>File does not exist</CENTER><BR>\n");
		} else if (rc==-3) {
			prints(sid, "<BR><CENTER>File already exists</CENTER><BR>\n");
		} else {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		}
		return;
	}
	directory=file.filepath+7;
	snprintf(filename, sizeof(filename)-1, "%s/%s%s", config->server_dir_var_files, directory, file.filename);
	fixslashes(filename);
	fp=fopen(filename, "wb");
	if (fp==NULL) {
		logerror(sid, __FILE__, __LINE__, "ERROR: Cannot write to file [%s].", filename);
		prints(sid, "<BR><CENTER>Error writing to file %s</CENTER><BR>\n", filename);
		return;
	}
	for (i=0;i<filesize;i++) {
//		if (*filebody>127) logerror(sid, __FILE__, __LINE__, "high ascii [%d][%c].", *filebody, *filebody);
		fprintf(fp, "%c", *filebody);
		filebody++;
	}
	fclose(fp);
	prints(sid, "<CENTER>file %d edited successfully</CENTER><BR>\n", file.fileid);
	db_log_activity(sid, 1, "files", file.fileid, "modify", "%s - %s edited file %d %s", sid->dat->in_RemoteAddr, sid->dat->user_username, file.fileid, file.filename);
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/fileinfoedit?fileid=%d&location=", sid->dat->in_ScriptName, file.fileid);
	printhex(sid, "%s", file.filepath);
	prints(sid, "\">\n");
	return;
}

void mod_main(CONN *sid)
{
	if (dirlist(sid)==0) return;
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	if ((strncmp(sid->dat->in_RequestURI, "/fileul", 7)!=0)&&(strncmp(sid->dat->in_RequestURI, "/filerecv", 9)!=0)) {
		htpage_topmenu(sid, MENU_FILES);
		prints(sid, "<BR>\r\n");
	}
	if (strncmp(sid->dat->in_RequestURI, "/fileul", 7)==0) {
		fileul(sid);
	} else if (strncmp(sid->dat->in_RequestURI, "/filerecv", 9)==0) {
		filerecv(sid);
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
	return;
}

DllExport int mod_init(_PROC *_proc, FUNCTION *_functions)
{
	proc=_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main("mod_files", "FILES", "/files/", "mod_main", "/file", mod_main)!=0) return -1;
	return 0;
}