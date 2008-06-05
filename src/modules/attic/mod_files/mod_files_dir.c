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
#ifndef WIN32
#include <dirent.h>
#endif

int dirlist(CONN *sid)
{
/*
	obj_t *htobj=nes_settable(sid->N, &sid->N->g, "_SERVER");
	char *RequestURI=nes_getstr(sid->N, htobj, "REQUEST_URI");
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
	short int found;
	int i;
	int isvalid;
	int rc;
	SQLRES sqr1;
	SQLRES sqr2;
	int def_uid;
	int def_gid;
	int def_gperm;
	int def_operm;

	if (!(auth_priv(sid, "files")&A_READ)) {
		return -1;
	}
	if (strncmp(RequestURI, "/files/", 7)!=0) return -1;
	memset(uri, 0, sizeof(uri));
	snprintf(uri, sizeof(uri)-1, "%s", RequestURI);
	decodeurl(uri);
	if (strcmp("/files/~/", uri)==0) {
		snprintf(uri, sizeof(uri)-1, "/files/users/%s/", sid->dat->username);
		snprintf(RequestURI, sizeof(RequestURI)-1, "/files/users/%s/", sid->dat->username);
		snprintf(filename, sizeof(filename)-1, "%s/%04d%s", config->dir_var_domains, sid->dat->did, uri);
		while (filename[strlen(filename)-1]=='/') filename[strlen(filename)-1]='\0';
		if (stat(filename, &sb)!=0) {
			snprintf(uri, sizeof(uri)-1, "/files/users/");
			snprintf(RequestURI, sizeof(RequestURI)-1, "/files/users/");
			snprintf(filename, sizeof(filename)-1, "%s/%04d%s", config->dir_var_domains, sid->dat->did, uri);
			while (filename[strlen(filename)-1]=='/') filename[strlen(filename)-1]='\0';
		}
		if (stat(filename, &sb)!=0) {
			snprintf(uri, sizeof(uri)-1, "/files/");
			snprintf(RequestURI, sizeof(RequestURI)-1, "/files/");
		}
//	} else {
//		snprintf(uri, sizeof(uri)-1, "%s", RequestURI);
	}
	decodeurl(uri);
	directory=uri+7;
	snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s", config->dir_var_domains, sid->dat->did, directory);
	decodeurl(filename);
	fixslashes(filename);
	while ((filename[strlen(filename)-1]=='\\')||(filename[strlen(filename)-1]=='/')) { filename[strlen(filename)-1]='\0'; };
	if (strstr(filename, "..")!=NULL) return -1;
	snprintf(index, sizeof(index)-1, "%s/%s/index.html", config->dir_var_htdocs, directory);
	decodeurl(index);
	fixslashes(index);
	if (stat(index, &sb)==0) {
		filesend(sid, index);
		return 0;
	}
	if (stat(filename, &sb)!=0) {
		send_header(sid, 0, 200, "1", "text/html", -1, -1);
		htpage_topmenu(sid, MENU_FILES);
		prints(sid, "<BR /><CENTER>File or folder does not exist</CENTER><BR />\r\n");
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
			send_header(sid, 0, 200, "1", "text/html", -1, -1);
			htpage_topmenu(sid, MENU_FILES);
			if (rc==-2) {
				prints(sid, "<BR /><CENTER>File does not exist</CENTER><BR />\r\n");
			} else if (rc==-3) {
				prints(sid, "<BR /><CENTER>File already exists</CENTER><BR />\r\n");
			} else {
				prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
			}
			return 0;
		}
	}
	if (!(sb.st_mode & S_IFDIR)) {
		return filesend(sid, filename);
	}
	t=time(NULL);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime(&t));
*/
	send_header(sid, 0, 200, "1", "text/html", -1, -1);
	htpage_topmenu(sid, MENU_FILES);
/*
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
	strftime(timebuf, sizeof(timebuf)-1, "%Y-%m-%d %H:%M:%S", localtime(&t));
	if (sql_queryf(&sqr2, "SELECT obj_uid, obj_gid, obj_gperm, obj_operm FROM gw_files WHERE obj_did = %d AND filepath = '%s/' AND filename = '%s' AND filetype = 'dir'", sid->dat->did, uri2, ptemp)>-1) {
		if (sql_numtuples(&sqr2)==1) {
			def_uid=atoi(sql_getvalue(&sqr2, 0, 0));
			def_gid=atoi(sql_getvalue(&sqr2, 0, 1));
			def_gperm=atoi(sql_getvalue(&sqr2, 0, 2));
			def_operm=atoi(sql_getvalue(&sqr2, 0, 3));
		}
		sql_freeresult(&sqr2);
	}

	if (sql_queryf(&sqr1, "SELECT fileid, filename, filetype, lastdldate, numdownloads, description FROM gw_files WHERE obj_did = %d AND filepath = '%s' ORDER BY filetype, filename ASC", sid->dat->did, uri)<0) return -1;
	handle=opendir(filename);
	while ((dentry=readdir(handle))!=NULL) {
		if (strcmp(".", dentry->d_name)==0) continue;
		if (strcmp("..", dentry->d_name)==0) continue;
		snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s%s", config->dir_var_domains, sid->dat->did, directory, dentry->d_name);
		fixslashes(filename);
		stat(filename, &sb);
		found=0;
		for (i=0;i<sql_numtuples(&sqr1);i++) {
			if (strcmp(dentry->d_name, sql_getvalue(&sqr1, i, 1))==0) {
				found=1;
				break;
			}
		}
		if (!found) {
			strcpy(query, "INSERT INTO gw_files (obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filename, filepath, filetype, uldate, lastdldate, numdownloads, description) VALUES (");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '%d', '%d', '%d', '%d', '%d', ", timebuf, timebuf, def_uid, def_gid, sid->dat->did, def_gperm, def_operm);
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, dentry->d_name));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, uri));
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", sb.st_mode&S_IFDIR?"dir":"file");
			strncatf(query, sizeof(query)-strlen(query)-1, "'%s', '%s', '0', '')", timebuf, timebuf);
			if (sql_update(query)<0) {
				log_error(proc->N, "mod_files", __FILE__, __LINE__, 1, "%s - ERROR: Cannot upload file [%s].", sid->dat->in_RemoteAddr, dentry->d_name);
			} else {
//				db_log_activity(sid, "files", fileid, "insert", "%s - system uploaded file %d %s", sid->dat->in_RemoteAddr, fileid, dentry->d_name);
			}
			isvalid=0;
		}
	}
	closedir(handle);
	if (!isvalid) {
		sql_freeresult(&sqr1);
		if (sql_queryf(&sqr1, "SELECT fileid, filename, filetype, lastdldate, numdownloads, description FROM gw_files WHERE obj_did = %d AND filepath = '%s' ORDER BY filetype, filename ASC", sid->dat->did, uri)<0) return -1;
	}
	prints(sid, "<BR />\r\n<CENTER>\r\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=95%% STYLE='border-style:solid'>\r\n");
	i=4;
	if (auth_priv(sid, "files")&A_MODIFY) i++;
	if (strlen(mod_config.filter_program)) i++;
	prints(sid, "<TR><TH COLSPAN=%d STYLE='border-style:solid'>", i);
	prints(sid, "&nbsp;Index of ");
	memset(uri2, 0, sizeof(uri2));
	i=0;
	if (uri[i]=='/') {
		uri2[i]=uri[i];
		prints(sid, "/");
		i++;
		while (uri[i]) {
			if (uri[i]=='/') {
				prints(sid, "<A HREF=\"%s%s/\">%s</A>/", sid->dat->in_ScriptName, uri2, strrchr(uri2, '/')+1);
			}
			uri2[i]=uri[i];
			i++;
		}
	} else {
		prints(sid, "%s", uri);
	}
	prints(sid, "</TH></TR>\r\n");
	prints(sid, "<TR CLASS=\"FIELDNAME\">");
	if (auth_priv(sid, "files")&A_MODIFY) {
		prints(sid, "<TD STYLE='border-style:solid'>&nbsp;</TD>");
	}
	if (strlen(mod_config.filter_program)) {
		prints(sid, "<TD STYLE='border-style:solid'>&nbsp;</TD>");
	}
	prints(sid, "<TD width=20%% STYLE='border-style:solid'><B>&nbsp;Filename&nbsp;</B></TD><TD width=10%% STYLE='border-style:solid'><B>&nbsp;Date&nbsp;</B></TD>");
	prints(sid, "<TD width=10%% STYLE='border-style:solid'><B>&nbsp;Size&nbsp;</B></TD><TD width=60%% STYLE='border-style:solid'><B>&nbsp;Description&nbsp;</B></TD></TR>\r\n");
	if ((strncmp(uri, "/files/", 7)==0)&&(strlen(file.filepath)>1)) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		if (auth_priv(sid, "files")&A_MODIFY) {
			prints(sid, "<TD STYLE='border-style:solid'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>");
		}
		if (strlen(mod_config.filter_program)) {
			prints(sid, "<TD STYLE='border-style:solid'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</TD>");
		}
		prints(sid, "<TD COLSPAN=4 STYLE='border-style:solid'><A HREF=%s", sid->dat->in_ScriptName);
		printhex(sid, "%s", file.filepath);
		prints(sid, "><IMG ALIGN=TOP BORDER=0 SRC=/nullgs/images/file-foldero.png HEIGHT=16 WIDTH=16> Parent Directory</A></TD>\r\n");
	}
	for (i=0;i<sql_numtuples(&sqr1);i++) {
		snprintf(filename, sizeof(filename)-1, "%s/%04d/files/%s%s", config->dir_var_domains, sid->dat->did, directory, sql_getvalue(&sqr1, i, 1));
		fixslashes(filename);
		isvalid=1;
		if (stat(filename, &sb)!=0) isvalid=0;
		// we might wanna clean up, purge dead file records, but...
		if (!isvalid) continue;
		memset(timebuf, 0, sizeof(timebuf));
		if (isvalid) strftime(timebuf, sizeof(timebuf)-1, "%b %d %Y %H:%M", localtime(&sb.st_mtime));
		memset(showfile, 0, sizeof(showfile));
		snprintf(showfile, sizeof(showfile)-1, "%s", sql_getvalue(&sqr1, i, 1));
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		if (auth_priv(sid, "files")&A_MODIFY) {
			prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP STYLE='border-style:solid'><A HREF=%s/fileinfoedit?fileid=%d&location=", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
			printhex(sid, "%s", RequestURI);
			prints(sid, ">edit</A></TD>");
		}
		if (strlen(mod_config.filter_program)) {
			prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP STYLE='border-style:solid'>");
			if (!(sb.st_mode & S_IFDIR)) {
				prints(sid, "<A HREF=%s/filescan?fileid=%d&location=", sid->dat->in_ScriptName, atoi(sql_getvalue(&sqr1, i, 0)));
				printhex(sid, "%s", RequestURI);
				prints(sid, ">scan</A></TD>\r\n");
			} else {
				prints(sid, "&nbsp;</TD>\r\n");
			}
		}
		prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP style='cursor:hand; border-style:solid' onClick=window.location.href=\"");
		printhex(sid, "%s%s", RequestURI, showfile);
//		printhex(sid, "%s", showfile);
		prints(sid, "%s\" TITLE=\"%s\">", (strcmp(sql_getvalue(&sqr1, i, 2), "dir")==0)?"/":"", showfile);
		prints(sid, "<A HREF=");
		printhex(sid, "%s%s", RequestURI, showfile);
		if (strcmp(sql_getvalue(&sqr1, i, 2), "dir")==0) {
			prints(sid, "/ TITLE=\"%s\"><IMG ALIGN=TOP BORDER=0 SRC=/nullgs/images/file-folder.png HEIGHT=16 WIDTH=16>", showfile);
		} else {
			prints(sid, " TITLE=\"%s\"><IMG ALIGN=TOP BORDER=0 SRC=/nullgs/images/file-default.png HEIGHT=16 WIDTH=16>", showfile);
		}
		prints(sid, " %.25s%s</A></TD>", sql_getvalue(&sqr1, i, 1), strlen(sql_getvalue(&sqr1, i, 1))>25?"..":"");
		if (isvalid) {
			prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP STYLE='border-style:solid'>%s</TD>", timebuf);
			if (sb.st_size>1048576) {
				prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP STYLE='border-style:solid'>%10.1f M</TD>", (float)sb.st_size/1048576.0);
			} else {
				prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP STYLE='border-style:solid'>%10.1f K</TD>", (float)sb.st_size/1024.0);
			}
		} else {
			prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
			prints(sid, "<TD ALIGN=right VALIGN=top NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		}
		if (i>-1) {
			prints(sid, "<TD ALIGN=left VALIGN=top STYLE='border-style:solid'><PRE>%s&nbsp;</TD>", str2html(sid, sql_getvalue(&sqr1, i, 5)));
		} else {
			prints(sid, "<TD ALIGN=left VALIGN=top NOWRAP STYLE='border-style:solid'>&nbsp;</TD>");
		}
		prints(sid, "</TR>\r\n");
	}
	sql_freeresult(&sqr1);
	prints(sid, "</TABLE>\r\n</CENTER>\r\n</BODY></HTML>\r\n");
*/
	return 0;
}

int filemkdir(CONN *sid)
{
/*
	REC_FILE file;
	char *ptemp;
	char directory[255];
	char filename[255];
	char location[255];

	if (!(auth_priv(sid, "files")&A_INSERT)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return -1;
	}
	if (dbread_file(sid, 2, 0, &file)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return -1;
	}
	snprintf(file.filename, sizeof(file.filename)-1, "New Folder");
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
	prints(sid, "<CENTER>\r\n");
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\r\n");
	prints(sid, "<FORM METHOD=POST ACTION=%s/filedirsave?location=\"%s\" NAME=filedirsave>\r\n", sid->dat->in_ScriptName, directory);
//	prints(sid, "<FORM METHOD=POST ACTION=%s/filedirsave NAME=filedirsave>\r\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=hidden NAME=filepath value='%s'>\r\n", location);
	prints(sid, "<TR><TH COLSPAN=2>Adding folder to '%s'</TH></TR>\r\n", directory);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD>&nbsp;<B>Folder Name</B>&nbsp;</TD><TD ALIGN=RIGHT><INPUT TYPE=TEXT NAME=filename SIZE=40 VALUE='%s'></TD></TR>\r\n", file.filename);
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2>&nbsp;<B>Description</B>&nbsp;</TD></TR>\r\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD ALIGN=CENTER COLSPAN=2><TEXTAREA WRAP=PHYSICAL NAME=description ROWS=5 COLS=50></TEXTAREA></TD></TR>\r\n");
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
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'>&nbsp;<B>Group Members</B>&nbsp;</TD><TD ALIGN=RIGHT STYLE='padding:0px'>\r\n");
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
	prints(sid, "<TR><TD ALIGN=CENTER COLSPAN=2><INPUT TYPE=submit VALUE='Create Folder'></TD></TR>\r\n");
	prints(sid, "</FORM>\r\n");
	prints(sid, "</TABLE>\r\n");
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\ndocument.filedirsave.filename.focus();\r\n// -->\r\n</SCRIPT>\r\n");
	prints(sid, "</BODY></HTML>\r\n");
*/
	return 0;
}

int filedirsave(CONN *sid)
{
/*
	REC_FILE filerec;
	char file[1024];
	char query[2048];
	char datebuf[40];
	char *ptemp;
	int rc;
	SQLRES sqr;
	int fileid;
	time_t t;

	if (!(auth_priv(sid, "files")&A_INSERT)) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return -1;
	}
	if (dbread_file(sid, 2, 0, &filerec)!=0) {
		prints(sid, "<BR /><CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
		return -1;
	}
	if (auth_priv(sid, "files")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) filerec.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) filerec.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "files")&A_ADMIN)||(filerec.obj_uid==sid->dat->uid)) {
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
			prints(sid, "<CENTER>Directory does not exist</CENTER><BR />\r\n");
		} else if (rc==-3) {
			prints(sid, "<CENTER>Directory already exists</CENTER><BR />\r\n");
		} else {
			prints(sid, "<CENTER>%s</CENTER><BR />\r\n", lang.err_noaccess);
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
	snprintf(file, sizeof(file)-1, "%s/%04d/files%s%s", config->dir_var_domains, sid->dat->did, filerec.filepath+6, filerec.filename);
	if (strstr(file, "..")!=NULL) return -1;
	fixslashes(file);
#ifdef WIN32
	if (mkdir(file)!=0) {
#else
	if (mkdir(file, ~config->umask&0777)!=0) {
#endif
		prints(sid, "<BR /><CENTER>Error creating folder.</CENTER><BR />\r\n");
		return -1;
	}
	if (sql_queryf(&sqr, "SELECT fileid FROM gw_files WHERE filename = '%s' AND filepath = '%s'", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filerec.filename), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filerec.filepath))<0) return -1;
	if (sql_numtuples(&sqr)>0) {
		sql_updatef("DELETE FROM gw_files WHERE fileid = %d AND obj_did = %d", atoi(sql_getvalue(&sqr, 0, 0)), sid->dat->did);
	}
	sql_freeresult(&sqr);
	if (sql_query(&sqr, "SELECT max(fileid) FROM gw_files")<0) return -1;
	fileid=atoi(sql_getvalue(&sqr, 0, 0))+1;
	if (fileid<1) fileid=1;
	sql_freeresult(&sqr);
	strcpy(query, "INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filename, filepath, filetype, uldate, lastdldate, numdownloads, description) VALUES (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", fileid, datebuf, datebuf, filerec.obj_uid, filerec.obj_gid, filerec.obj_did, filerec.obj_gperm, filerec.obj_operm);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filerec.filename));
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s', ", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filerec.filepath));
	strncatf(query, sizeof(query)-strlen(query)-1, "'dir', '%s', '%s', '0', ", datebuf, datebuf);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, filerec.description));
	if (sql_update(query)<0) return -1;
	db_log_activity(sid, "filefolders", fileid, "insert", "%s - %s created file folder %d %s", sid->dat->in_RemoteAddr, sid->dat->username, fileid, file);
	prints(sid, "<CENTER>\r\n");
	prints(sid, "File folder '%s%s' has been created.", filerec.filepath, filerec.filename);
	prints(sid, "</CENTER>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s%s%s/\">\r\n", sid->dat->in_ScriptName, encodeurl(sid, filerec.filepath), encodeurl(sid, filerec.filename));
*/
	return 0;
}
