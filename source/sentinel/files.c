/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

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

int dirlist(int sid)
{
#ifdef WIN32
	struct	direct *dentry;
#else
	struct	dirent *dentry;
#endif
	DIR	*handle;
	char	file[1024];
	char	index[1024];
	char	showfile[1024];
	struct	stat sb;
	char	*directory;
	char timebuf[100];
	time_t t;

	if (auth(sid, "files")<1) {
		return -1;
	}
	if (strncmp(conn[sid].dat->in_RequestURI, "/files/", 7)!=0) {
		return -1;
	}
	directory=conn[sid].dat->in_RequestURI+7;
	snprintf(file, sizeof(file)-1, "%s/%s", config.server_file_dir, directory);
	URLDecode(file);
	FixSlashes(file);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	if (strstr(file, "..")!=NULL) return -1;
	snprintf(index, sizeof(index)-1, "%s/%s/index.html", config.server_http_dir, directory);
	URLDecode(index);
	FixSlashes(index);
	if (stat(index, &sb)==0) {
		sendfile(sid, index);
		return 0;
	}
	if (stat(file, &sb)!=0) return -1;
	if (!(sb.st_mode & S_IFDIR)) return sendfile(sid, file);
	t=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), "%b %d %H:%M:%S", localtime(&t));
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Files");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch05-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("function FileUpload()\n");
	prints("{\n");
	prints("	window.open('%s/fileul?location=%s','fileulwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=20,left=20,status=no,menubar=no,scrollbars=yes,resizable=no,width=400,height=300');\n", conn[sid].dat->in_ScriptName, conn[sid].dat->in_RequestURI);
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<A CLASS='TBAR' HREF=%s>REFRESH</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_RequestURI);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("<BR>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4>Index of %s</TH></TR>\n", COLOR_TRIM, conn[sid].dat->in_RequestURI);
	prints("<TR BGCOLOR=%s>", COLOR_FNAME);
	prints("<TH width=20%%>Filename</TH><TH width=10%%>Size</TH>");
	prints("<TH width=10%%>Date</TH><TH width=60%%>Description</TH></TR>\n");
	handle=opendir(file);
	while ((dentry=readdir(handle))!=NULL) {
		snprintf(file, sizeof(file)-1, "%s/%s%s", config.server_file_dir, directory, dentry->d_name);
		FixSlashes(file);
		stat(file, &sb);
		if (strcmp(".", dentry->d_name)==0) continue;
		if ((strcmp("..", dentry->d_name)==0)&&(strcmp("/files/", conn[sid].dat->in_RequestURI)==0)) continue;
		if (strcmp("..", dentry->d_name)==0) {
			prints("<TR BGCOLOR=%s><TD COLSPAN=4><IMG SRC=/sentinel/images/folder-o.gif>", COLOR_FTEXT);
			prints("<A HREF=%s/> Parent Directory</A></TD>\n", dentry->d_name);
			continue;
		}
		strftime(timebuf, sizeof(timebuf), "%b %d %Y %H:%M", localtime(&sb.st_mtime));
		memset(showfile, 0, sizeof(showfile));
		snprintf(showfile, sizeof(showfile)-1, "%s", dentry->d_name);
		prints("<TR BGCOLOR=%s><TD ALIGN=left NOWRAP>", COLOR_FTEXT);
		if (sb.st_mode & S_IFDIR) {
			prints("<IMG SRC=/sentinel/images/folder.gif>&nbsp;<A HREF=");
			hexprints("%s", showfile);
			prints("/>%s/</A></TD>", dentry->d_name);
		} else {
			prints("<IMG SRC=/sentinel/images/file.gif>&nbsp;<A HREF=");
			hexprints("%s", showfile);
			prints(">%s</A></TD>", dentry->d_name);
		}
		if (sb.st_size>1048576) {
			prints("<TD ALIGN=right NOWRAP>%10.1f M</TD>\n", (float)sb.st_size/1048576.0);
		} else {
			prints("<TD ALIGN=right NOWRAP>%10.1f K</TD>\n", (float)sb.st_size/1024.0);
		}
		prints("<TD ALIGN=right NOWRAP>%s</TD>\n", timebuf);
		prints("<TD ALIGN=left NOWRAP>&nbsp;</TD></TR>\n");
	}
	closedir(handle);
	prints("</TABLE>\n");
	if (auth(sid, "files")>1) {
		prints("<A HREF=javascript:FileUpload()>Upload File</A>\n");
	}
	prints("</CENTER>\n");
	prints("</BODY></HTML>\n");
	return 0;
}

int fileul(int sid)
{
	char directory[255];
	char file[255];
//	struct stat sb;

	if (auth(sid, "files")<2) {
		return -1;
	}
	if (getgetenv(sid, "LOCATION")!=NULL) {
		snprintf(directory, sizeof(directory)-1, "%s", getgetenv(sid, "LOCATION"));
	}
	snprintf(file, sizeof(file)-1, "%s/%s", config.server_file_dir, directory+7);
	URLDecode(file);
	FixSlashes(file);
	while ((file[strlen(file)-1]=='\\')||(file[strlen(file)-1]=='/')) { file[strlen(file)-1]='\0'; };
	if (strstr(file, "..")!=NULL) return -1;
//	if (stat(file, &sb)!=0) return -1;
//	if (!(sb.st_mode & S_IFDIR)) return sendfile(file);
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel File Upload");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left>&nbsp;\n", COLOR_TMENU);
	prints("</TD><TD ALIGN=right>\n");
	prints("<A CLASS='TBAR' HREF=/sentinel/help/index.html TARGET=main>HELP</A>\n");
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
	prints("<BR><BR>\n<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 WIDTH=90%%>\n");
	prints("<TR BGCOLOR=%s><TH COLSPAN=4>Index of %s</TH></TR>\n", COLOR_TRIM, directory);
	prints("</TABLE>\n");
	if (auth(sid, "files")>1) {
		prints("<FORM METHOD=POST ACTION=%s/recvfile NAME=filesend ENCTYPE=multipart/form-data>\n", conn[sid].dat->in_ScriptName);
		prints("<INPUT TYPE=hidden NAME=location value='%s'>\n", getgetenv(sid, "LOCATION"));
		prints("<INPUT TYPE=file NAME=userfile>\n");
		prints("<INPUT TYPE=submit VALUE='Send'>\n");
		prints("</FORM>\n");
	}
	prints("</CENTER>\n");
	prints("</BODY></HTML>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.filesend.userfile.focus();\n// -->\n</SCRIPT>\n");
	return 0;
}

int sendfile(int sid, unsigned char *file)
{
	struct stat sb;
	FILE *fp;
	char fileblock[2048];
	int blocksize;
	int ich;

	URLDecode(file);
	FixSlashes(file);
	if (strstr(file, "..")!=NULL) return -1;
	if (stat(file, &sb)!=0) return -1;
	if (sb.st_mode&S_IFDIR) return -1;
	if (strlen(conn[sid].dat->in_IfModifiedSince)) {
		send_fileheader(sid, 1, 304, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
		conn[sid].dat->out_headdone=1;
		conn[sid].dat->out_bodydone=1;
		conn[sid].dat->out_flushed=1;
		conn[sid].dat->out_ReplyData[0]='\0';
		flushbuffer(sid);
		return 0;
	}
	conn[sid].dat->out_ContentLength=sb.st_size;
	send_fileheader(sid, 1, 200, "OK", "1", get_mime_type(file), sb.st_size, sb.st_mtime);
	fp=fopen(file, "rb");
	if (fp==NULL) return -1;
	blocksize=0;
#ifdef WIN32
	if (RunAsCGI) _setmode(_fileno(stdout), _O_BINARY);
#endif
	while ((ich=getc(fp))!=EOF) {
		if (blocksize>sizeof(fileblock)-1) {
			if (RunAsCGI) {
				fwrite(fileblock, sizeof(char), blocksize, stdout);
			} else {
				send(conn[sid].socket, fileblock, blocksize, 0);
			}
			blocksize=0;
		}
		fileblock[blocksize]=ich;
		blocksize++;
	}
	if (blocksize) {
		if (RunAsCGI) {
			fwrite(fileblock, sizeof(char), blocksize, stdout);
		} else {
			send(conn[sid].socket, fileblock, blocksize, 0);
		}
		blocksize=0;
	}
	fclose(fp);
	conn[sid].dat->out_headdone=1;
	conn[sid].dat->out_bodydone=1;
	conn[sid].dat->out_flushed=1;
	conn[sid].dat->out_ReplyData[0]='\0';
	flushbuffer(sid);
#ifdef DEBUG
logdata(">> [ SENDING FILE DATA ]\n");
#endif
	return 0;
}

int recvfile(int sid)
{
	char *directory=NULL;
	char filename[1024];
	char file[1024];
	char line[1024];
	char location[1024];
	char boundary[1024];
	char boundary2[1024];
	char boundary3[1024];
	char *pPostData;
	char *pTemp;
	FILE *fp;
	int i;
	unsigned int j;

	if (auth(sid, "files")<2) {
		return -1;
	}
	pPostData=conn[sid].PostData;
	memset(boundary, 0, sizeof(boundary));
	memset(location, 0, sizeof(location));
	i=0;
	j=0;
	/* get the mime boundary */
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<conn[sid].dat->in_ContentLength)&&(j<sizeof(boundary)-1)) {
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
	pPostData=conn[sid].PostData;
	memset(filename, 0, sizeof(filename));
	while ((strcmp(line, boundary2)!=0)&&(i<conn[sid].dat->in_ContentLength)) {
		memset(line, 0, sizeof(line));
		j=0;
		while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<conn[sid].dat->in_ContentLength)&&(j<sizeof(line)-1)) {
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
		pTemp=line;
		if (strncmp(line, "Content-Disposition: form-data; ", 32)==0) {
			pTemp+=32;
			if (strncmp(pTemp, "name=\"userfile\"; ", 17)==0) {
				pTemp+=17;
				if (strncmp(pTemp, "filename=\"", 10)==0) {
					pTemp+=10;
					if (strrchr(pTemp, '\\')!=NULL) {
						pTemp=strrchr(pTemp, '\\')+1;
					}
					snprintf(filename, sizeof(filename)-1, "%s", pTemp);
					while (filename[strlen(filename)-1]=='\"') filename[strlen(filename)-1]='\0';
				}
			} else if (strncmp(pTemp, "name=\"location\"", 15)==0) {
				pTemp+=15;
				while ((*pPostData=='\n')||(*pPostData=='\r')) {
					pPostData++;
					i++;
				}
				j=0;
				while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<conn[sid].dat->in_ContentLength)&&(j<sizeof(location)-1)) {
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
			pTemp=line+14;
			snprintf(file, sizeof(file)-1, "%s%s%s", config.server_file_dir, directory, filename);
			FixSlashes(file);
			fp=fopen(file, "wb");
			if (fp==NULL) {
				logerror("ERROR: Cannot create file [%s].\n", file);
				return -1;
			}
			while ((strncmp(pPostData, boundary3, strlen(boundary3))!=0)&&(i<conn[sid].dat->in_ContentLength)) {
				fprintf(fp, "%c", *pPostData);
				pPostData++;
				i++;
			}
			fclose(fp);
		}
	}
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel File Upload");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left>&nbsp;\n", COLOR_TMENU);
	prints("</TD><TD ALIGN=right>\n");
	prints("<A CLASS='TBAR' HREF=/sentinel/help/index.html TARGET=main>HELP</A>\n");
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE><BR>\n");
	prints("<BGSOUND SRC=/sentinel/sounds/reminder.wav>\n");
	prints("<CENTER>\n");
	prints("File '%s%s' has been received.", location, filename);
	prints("</CENTER><BR>\n");
	printfooter(sid);
	logaccess(1, "%s - %s uploaded file %s", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, file);
	conn[sid].dat->out_bodydone=1;
	flushbuffer(sid);
	return 0;
}
