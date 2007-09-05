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
#include "mod_email.h"

int wmfolder_testcreate(CONN *sid, int accountid, int folderid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	char *varpath=nes_getstr(proc->N, confobj, "var_path");
	int umask=(int)nes_getnum(proc->N, confobj, "umask");
	char dirname[512];
	struct stat sb;

	if (accountid<1) return -1;
	if (folderid<0) folderid=0;
	memset(dirname, 0, sizeof(dirname));
	snprintf(dirname, sizeof(dirname)-1, "%s/domains/%04d", varpath, sid->dat->did);
	fixslashes(dirname);
	if ((stat(dirname, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(dirname)!=0) {
#else
		if (mkdir(dirname, ~umask&0777)!=0) {
#endif
			log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ERROR: Maildir '%s' is not accessible!", dirname);
			return -1;
		}
	}
	snprintf(dirname, sizeof(dirname)-1, "%s/domains/%04d/mail", varpath, sid->dat->did);
	fixslashes(dirname);
	if ((stat(dirname, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(dirname)!=0) {
#else
		if (mkdir(dirname, ~umask&0777)!=0) {
#endif
			log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ERROR: Maildir '%s' is not accessible!", dirname);
			return -1;
		}
	}
	snprintf(dirname, sizeof(dirname)-1, "%s/domains/%04d/mail/%04d", varpath, sid->dat->did, accountid);
	fixslashes(dirname);
	if ((stat(dirname, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(dirname)!=0) {
#else
		if (mkdir(dirname, ~umask&0777)!=0) {
#endif
			log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ERROR: Maildir '%s' is not accessible!", dirname);
			return -1;
		}
	}
	snprintf(dirname, sizeof(dirname)-1, "%s/domains/%04d/mail/%04d/%04d", varpath, sid->dat->did, accountid, folderid);
	fixslashes(dirname);
	if ((stat(dirname, &sb)!=0)||(!(sb.st_mode&S_IFDIR))) {
#ifdef WIN32
		if (mkdir(dirname)!=0) {
#else
		if (mkdir(dirname, ~umask&0777)!=0) {
#endif
			log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ERROR: Maildir '%s' is not accessible!", dirname);
			return -1;
		}
	}
	return 0;
}

int wmfolder_msgmove(CONN *sid, int accountid, int messageid, int srcfolderid, int dstfolderid)
{
	obj_t *confobj=nes_getobj(proc->N, &proc->N->g, "CONFIG");
	char *varpath=nes_getstr(proc->N, confobj, "var_path");
	struct stat sb;
	char srcfilename[512];
	char dstfilename[512];

	wmfolder_testcreate(sid, accountid, dstfolderid);
	memset(srcfilename, 0, sizeof(srcfilename));
	memset(dstfilename, 0, sizeof(dstfilename));
	if (srcfolderid==0) {
		snprintf(srcfilename, sizeof(srcfilename)-1, "%s/domains/%04d/mail/%04d/%04d/%06d.tmp", varpath, sid->dat->did, accountid, srcfolderid, messageid);
	} else {
		snprintf(srcfilename, sizeof(srcfilename)-1, "%s/domains/%04d/mail/%04d/%04d/%06d.msg", varpath, sid->dat->did, accountid, srcfolderid, messageid);
	}
	snprintf(dstfilename, sizeof(dstfilename)-1, "%s/domains/%04d/mail/%04d/%04d/%06d.msg", varpath, sid->dat->did, accountid, dstfolderid, messageid);
	fixslashes(srcfilename);
	fixslashes(dstfilename);
	if (stat(srcfilename, &sb)==0) {
		if (rename(srcfilename, dstfilename)==0) {
			return 0;
		}
	}
	return -1;
}

int wmfolder_makedefaults(CONN *sid, int accountid)
{
	char curdate[40];
	char *p;

	memset(curdate, 0, sizeof(curdate));
	time_unix2sql(curdate, sizeof(curdate)-1, time(NULL));
	if (wmfolder_testcreate(sid, accountid, 1)<0) return -1;
	if (wmfolder_testcreate(sid, accountid, 2)<0) return -1;
	if (wmfolder_testcreate(sid, accountid, 3)<0) return -1;
	if (wmfolder_testcreate(sid, accountid, 4)<0) return -1;
	if (wmfolder_testcreate(sid, accountid, 5)<0) return -1;
	if (wmfolder_testcreate(sid, accountid, 6)<0) return -1;
	if (wmfolder_testcreate(sid, accountid, 7)<0) return -1;
	p=lang_gets(sid, "mod_email", "folder1_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	p=lang_gets(sid, "mod_email", "folder2_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	p=lang_gets(sid, "mod_email", "folder3_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	p=lang_gets(sid, "mod_email", "folder4_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	p=lang_gets(sid, "mod_email", "folder5_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	p=lang_gets(sid, "mod_email", "folder6_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	p=lang_gets(sid, "mod_email", "folder7_name");
	sql_updatef(proc->N, "INSERT INTO nullgs_entries (pid, did, ctime, mtime, class, name, data) VALUES (%d, %d, '%s', '%s', 'emailfolder', '%s', '{ foldername=''%s'' }');", accountid, sid->dat->did, curdate, curdate, p, p);
	return 0;
}

void webmailsave(CONN *sid)
{
/*
	struct stat sb;
	FILE *fp;
	FILE *fp2;
	wmheader header;
	char *filebody=NULL;
	char *msgbody=NULL;
	char *pmsgbody;
	char *ptemp;
	char query[8192];
	char boundary[128];
	char filename[512];
	char cfilesize[10];
	char msgctype[16];
	char datebuf[40];
	char date_tz[10];
	char line[1024];
	char msgfilename[512];
	int filesize=0;
	struct timeval ttime;
	struct timezone tzone;
	unsigned int mimesize;
	SQLRES sqr;
	int forward;
	int fwdacct;
	int headerid;
	int folderid;

	if (strcasecmp(sid->dat->in_RequestMethod, "POST")!=0) return;
	prints(sid, "Saving File");
	memset(datebuf, 0, sizeof(datebuf));
	if ((ptemp=getmimeenv(sid, "ACCOUNTID", &mimesize))!=NULL) {
		strncpy(datebuf, ptemp, sizeof(datebuf)-1);
		if (mimesize<strlen(datebuf)) datebuf[mimesize]='\0';
		sid->dat->mailcurrent=atoi(datebuf);
	}
	if (dbread_mailcurrent(sid, sid->dat->mailcurrent)<0) return;
	prints(sid, ".");
	memset((char *)&header, 0, sizeof(header));
	memset(boundary, 0, sizeof(boundary));
	memset(msgctype, 0, sizeof(msgctype));
	memset(filename, 0, sizeof(filename));
	memset(cfilesize, 0, sizeof(cfilesize));
	if (msgbody!=NULL) {
		free(msgbody);
		msgbody=NULL;
	}
	gettimeofday(&ttime, &tzone);
	snprintf(boundary, sizeof(boundary)-1, "------------NGW%d", (int)ttime.tv_sec);
	msgbody=calloc(sid->dat->in_ContentLength+1024, sizeof(char));
	if ((ptemp=getmimeenv(sid, "MSGTO", &mimesize))!=NULL) {
		strncpy(header.To, ptemp, sizeof(header.To)-1);
		if (mimesize<strlen(header.To)) header.To[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "MSGCC", &mimesize))!=NULL) {
		strncpy(header.CC, ptemp, sizeof(header.CC)-1);
		if (mimesize<strlen(header.CC)) header.CC[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "MSGBCC", &mimesize))!=NULL) {
		strncpy(header.BCC, ptemp, sizeof(header.BCC)-1);
		if (mimesize<strlen(header.BCC)) header.BCC[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "MSGSUBJECT", &mimesize))!=NULL) {
		strncpy(header.Subject, ptemp, sizeof(header.Subject)-1);
		if (mimesize<strlen(header.Subject)) header.Subject[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "INREPLYTO", &mimesize))!=NULL) {
		strncpy(header.InReplyTo, ptemp, sizeof(header.InReplyTo)-1);
		if (mimesize<strlen(header.InReplyTo)) header.InReplyTo[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "CTYPE", &mimesize))!=NULL) {
		snprintf(msgctype, sizeof(msgctype)-1, "text/%s", ptemp);
		if (mimesize+5<strlen(msgctype)) msgctype[mimesize+5]='\0';
	} else {
		snprintf(msgctype, sizeof(msgctype)-1, "text/plain");
	}
	if ((ptemp=getmimeenv(sid, "MSGBODY", &mimesize))!=NULL) {
		strncpy(msgbody, ptemp, sid->dat->in_ContentLength+1023);
		if (mimesize<strlen(msgbody)) msgbody[mimesize]='\0';
	}
	if ((ptemp=getmimeenv(sid, "FATTACH", &mimesize))!=NULL) {
		filebody=webmailfileul(sid, filename, cfilesize);
		filesize=atoi(cfilesize);
		if (strlen(filename)==0) filesize=0;
	}
	forward=0;
	if ((ptemp=getmimeenv(sid, "FORWARD", &mimesize))!=NULL) {
		strncpy(line, ptemp, sizeof(line)-1);
		if (mimesize<strlen(line)) line[mimesize]='\0';
		forward=atoi(line);
	}
	fwdacct=0;
	if ((ptemp=getmimeenv(sid, "FWDACCT", &mimesize))!=NULL) {
		strncpy(line, ptemp, sizeof(line)-1);
		if (mimesize<strlen(line)) line[mimesize]='\0';
		fwdacct=atoi(line);
	}
	gettimeofday(&ttime, &tzone);
	snprintf(header.MessageID, sizeof(header.MessageID)-1, "<%d%03d.%s>", (int)ttime.tv_sec, (int)(ttime.tv_usec/1000), sid->dat->wm->address);
	strftime(datebuf, sizeof(datebuf)-1, "%Y-%m-%d %H:%M:%S", gmtime((time_t *)&ttime.tv_sec));
	ttime.tv_sec+=time_tzoffset(sid, ttime.tv_sec);
	strftime(header.Date, sizeof(header.Date), "%a, %d %b %Y %H:%M:%S", gmtime((time_t *)&ttime.tv_sec));
	snprintf(date_tz, sizeof(date_tz)-1, " %+.4d", +time_tzoffset(sid, ttime.tv_sec)/36);
	strncat(header.Date, date_tz, sizeof(header.Date)-strlen(header.Date)-1);
	if ((filesize>0)||(forward>0)) {
		snprintf(header.contenttype, sizeof(header.contenttype)-1, "multipart/mixed; boundary=\"%s\"", boundary);
	} else {
		snprintf(header.contenttype, sizeof(header.contenttype)-1, "%s", msgctype);
	}
	if (sql_queryf(&sqr, "SELECT max(mailheaderid) FROM gw_email_headers where accountid = %d", sid->dat->mailcurrent)<0) return;
	headerid=atoi(sql_getvalue(&sqr, 0, 0))+1;
	sql_freeresult(&sqr);
	if (headerid<1) headerid=1;
	memset(query, 0, sizeof(query));
	strcpy(query, "INSERT INTO gw_email_headers (mailheaderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountid, folder, status, size, uidl, hdr_from, hdr_replyto, hdr_to, hdr_cc, hdr_bcc, hdr_subject, hdr_date, hdr_messageid, hdr_inreplyto, hdr_contenttype, hdr_boundary, hdr_encoding) values (");
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', ", headerid, datebuf, datebuf, sid->dat->uid, 0, sid->dat->did, 0, 0);
	strncatf(query, sizeof(query)-strlen(query)-1, "'%d", sid->dat->mailcurrent);
	strncatf(query, sizeof(query)-strlen(query)-1, "', '2"); // folder
	strncatf(query, sizeof(query)-strlen(query)-1, "', 'r");
	strncatf(query, sizeof(query)-strlen(query)-1, "', '0"); // MSG SIZE !!!
	strncatf(query, sizeof(query)-strlen(query)-1, "', 'uidl");
	strncatf(query, sizeof(query)-strlen(query)-1, "', '\"%s\" <%s>", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, sid->dat->wm->realname), str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, sid->dat->wm->address));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, sid->dat->wm->replyto));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.To));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.CC));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.BCC));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.Subject));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, datebuf));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.MessageID));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.InReplyTo));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.contenttype));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '--%s", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, boundary));
	strncatf(query, sizeof(query)-strlen(query)-1, "', '%s')", str2sql(getbuffer(sid), sizeof(sid->dat->smallbuf[0])-1, header.encoding));
	if (sql_update(query)<0) return;
	prints(sid, ".");
	if (wmfolder_testcreate(sid, sid->dat->mailcurrent, 1)<0) {
		return;
	}
	memset(msgfilename, 0, sizeof(msgfilename));
	snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->did, sid->dat->mailcurrent, 2, headerid);
	fixslashes(msgfilename);
	fp=fopen(msgfilename, "wb");
	if (fp==NULL) {
		log_error(proc->N, "mod_email", __FILE__, __LINE__, 1, "ERROR: Could not open message '%s'!", msgfilename);
		return;
	}
	fprintf(fp, "From: \"%s\" <%s>\r\n", sid->dat->wm->realname, sid->dat->wm->address);
	if (strlen(sid->dat->wm->replyto)>0) {
		fprintf(fp, "Reply-To: <%s>\r\n", sid->dat->wm->replyto);
	}
	if (strlen(sid->dat->wm->organization)>0) {
		fprintf(fp, "Organization: %s\r\n", sid->dat->wm->organization);
	}
	fprintf(fp, "To: %s\r\n", header.To);
	if (strlen(header.CC)) {
		fprintf(fp, "Cc: %s\r\n", header.CC);
	}
	fprintf(fp, "Subject: %s\r\n", header.Subject);
	fprintf(fp, "Date: %s\r\n", header.Date);
	fprintf(fp, "Message-ID: %s\r\n", header.MessageID);
	if (strlen(header.InReplyTo)) {
		fprintf(fp, "In-Reply-To: %s\r\n", header.InReplyTo);
	}
	if ((filesize>0)||(forward>0)||(strcasecmp(header.contenttype, "text/html")==0)) {
		fprintf(fp, "MIME-Version: 1.0\r\n");
	}
	if ((filesize>0)||(forward>0)) {
		fprintf(fp, "Content-Type: multipart/mixed; boundary=\"%s\"\r\n", boundary);
	} else {
		fprintf(fp, "Content-Type: %s; charset=%s\r\n", header.contenttype, "iso-8859-1");
	}
	fprintf(fp, "X-Mailer: %s %s\r\n", SERVER_NAME, PACKAGE_VERSION);
	fprintf(fp, "\r\n");
	if ((filesize>0)||(forward>0)) {
		fprintf(fp, "This is a multi-part message in MIME format.\r\n\r\n");
		fprintf(fp, "--%s\r\n", boundary);
		fprintf(fp, "Content-Type: %s\r\n", msgctype);
		fprintf(fp, "Content-Transfer-Encoding: 8bit\r\n\r\n");
	}
	if (strcasecmp(header.contenttype, "text/html")==0) fprintf(fp, "<HTML>\r\n");
	pmsgbody=msgbody;
	while (strlen(pmsgbody)>78) {
		memset(line, 0, sizeof(line));
		snprintf(line, 79, "%s", pmsgbody);
		line[78]='\0';
		if (strchr(line, '\r')||strchr(line, '\n')) {
			if ((ptemp=strchr(line, '\r'))!=NULL) *ptemp='\0';
			if ((ptemp=strchr(line, '\n'))!=NULL) *ptemp='\0';
			fprintf(fp, "%s\r\n", line);
			pmsgbody+=strlen(line);
			if (*pmsgbody=='\r') pmsgbody++;
			if (*pmsgbody=='\n') pmsgbody++;
		} else if ((ptemp=strrchr(line, ' '))!=NULL) {
			*ptemp='\0';
			fprintf(fp, "%s\r\n", line);
			pmsgbody+=strlen(line)+1;
		} else {
			fprintf(fp, "%s", line);
			pmsgbody+=strlen(line);
		}
	}
	memset(line, 0, sizeof(line));
	snprintf(line, 78, "%s", pmsgbody);
	if (strlen(line)) fprintf(fp, "%s\r\n", line);
	free(msgbody);
	if (strlen(sid->dat->wm->signature)>0) {
		fprintf(fp, "\r\n");
		if (strcasecmp(header.contenttype, "text/html")==0) fprintf(fp, "<PRE>");
		fprintf(fp, "%s", sid->dat->wm->signature);
		if (strcasecmp(header.contenttype, "text/html")==0) fprintf(fp, "</PRE>");
		fprintf(fp, "\r\n");
	}
	if (strcasecmp(header.contenttype, "text/html")==0) fprintf(fp, "</HTML>\r\n");
	if (filesize>0) {
		fprintf(fp, "\r\n--%s\r\n", boundary);
		fprintf(fp, "Content-Type: application/octet-stream; name=\"%s\"\r\n", filename);
		fprintf(fp, "Content-Transfer-Encoding: base64\r\n");
		fprintf(fp, "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n", filename);
		EncodeBase64file(fp, filebody, filesize);
	}
	if (forward>0) {
		if (sql_queryf(&sqr, "SELECT * FROM gw_email_headers WHERE obj_uid = %d and accountid = %d and status != 'd' AND mailheaderid = %d", sid->dat->uid, fwdacct, forward)<0) goto stuff;
		if (sql_numtuples(&sqr)!=1) {
			sql_freeresult(&sqr);
			goto stuff;
		} else {
			folderid=atoi(sql_getvaluebyname(&sqr, 0, "folder"));
		}
		dbread_getheader(sid, &sqr, 0, &header);
		sql_freeresult(&sqr);
		memset(msgfilename, 0, sizeof(msgfilename));
		snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->did, fwdacct, folderid, forward);
		fixslashes(msgfilename);
		if ((fp2=fopen(msgfilename, "r"))!=NULL) {
			fprintf(fp, "\r\n--%s\r\n", boundary);
			fprintf(fp, "Content-Type: message/rfc822\r\n");
			fprintf(fp, "Content-Disposition: inline\r\n\r\n");
			while (fgets(line, sizeof(line)-1, fp2)!=NULL) {
				fprintf(fp, "%s", line);
			}
			fclose(fp2);
		}
	}
stuff:
	if ((filesize>0)||(forward>0)) {
		fprintf(fp, "\r\n--%s--\r\n\r\n", boundary);
	}
	fclose(fp);
	prints(sid, ".");
	if ((stat(msgfilename, &sb)!=0)||(sb.st_mode&S_IFDIR)) return;
	sql_updatef("UPDATE gw_email_headers SET size = '%d' WHERE mailheaderid = %d AND accountid = %d", (int)sb.st_size, headerid, sid->dat->mailcurrent);
	prints(sid, "OK<BR />\r\n");
	prints(sid, "[<A HREF=javascript:window.close()>Close Window</A>]<BR />\r\n", sid->dat->in_ScriptName);
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\nwindow.close();\r\n// -->\r\n</SCRIPT>\r\n");
*/
	return;
}

void webmailmove(CONN *sid)
{
/*
	char *ptemp1;
	char *ptemp2;
	char msgnum[8];
//	char uidl[100];
	int folderid=0;
	int oldfolderid=1;
	int nummessages;
	short int order=0;
	int offset=0;
//	int deleted=0;
	int i;
	SQLRES sqr;
	int move=0;
	int purge=0;

	if ((ptemp1=getgetenv(sid, "OFFSET"))!=NULL) {
		offset=atoi(ptemp1);
	} else if ((ptemp1=getpostenv(sid, "OFFSET"))!=NULL) {
		offset=atoi(ptemp1);
	}
	if ((ptemp1=getgetenv(sid, "ORDER"))!=NULL) {
		order=atoi(ptemp1);
	} else if ((ptemp1=getpostenv(sid, "ORDER"))!=NULL) {
		order=atoi(ptemp1);
	}
	if (order<0) order=0;
	if (order>7) order=7;
	if (((ptemp1=getpostenv(sid, "MOVE"))!=NULL)&&(strcasecmp(ptemp1, "Move to")==0)) {
		if ((ptemp1=getpostenv(sid, "DEST1"))!=NULL) {
			folderid=atoi(ptemp1);
		}
		if (folderid<1) {
			if ((ptemp1=getpostenv(sid, "DEST2"))!=NULL) {
				folderid=atoi(ptemp1);
			}
		}
		move=1;
	} else {
		folderid=4;
	}
	if (sql_queryf(&sqr, "SELECT mailheaderid, uidl, status, folder FROM gw_email_headers WHERE obj_uid = %d and accountid = %d and status != 'd'", sid->dat->uid, sid->dat->mailcurrent)<0) return;
	nummessages=sql_numtuples(&sqr);
	if (nummessages<1) {
		prints(sid, "<CENTER><B>You have no messages in this mailbox</B></CENTER><BR />\r\n");
	}
	for (i=0;i<nummessages;i++) {
		memset(msgnum, 0, sizeof(msgnum));
		snprintf(msgnum, sizeof(msgnum)-1, "%d", atoi(sql_getvalue(&sqr, i, 0)));
		ptemp1=getpostenv(sid, msgnum);
		ptemp2=getgetenv(sid, msgnum);
		if (ptemp1==NULL) ptemp1=ptemp2;
		if (ptemp1==NULL) continue;
		if (strcmp(ptemp1, sql_getvalue(&sqr, i, 1))!=0) continue;
		if (folderid>0) {
			prints(sid, "%s message %d...", move?"Moving":"Deleting", atoi(sql_getvalue(&sqr, i, 0)));
			oldfolderid=atoi(sql_getvalue(&sqr, i, 3));
			if ((oldfolderid==4)&&(folderid==4)) {
				sql_updatef("UPDATE gw_email_headers SET status = 'd' WHERE accountid = %d AND obj_uid = %d and mailheaderid = %d", sid->dat->mailcurrent, sid->dat->uid, atoi(sql_getvalue(&sqr, i, 0)));
				purge=1;
			} else {
				sql_updatef("UPDATE gw_email_headers SET folder = '%d' WHERE accountid = %d AND obj_uid = %d and mailheaderid = %d", folderid, sid->dat->mailcurrent, sid->dat->uid, atoi(sql_getvalue(&sqr, i, 0)));
			}
			wmfolder_msgmove(sid, sid->dat->mailcurrent, atoi(sql_getvalue(&sqr, i, 0)), atoi(sql_getvalue(&sqr, i, 3)), folderid);
			prints(sid, "success.<BR />\r\n");
		}
//		deleted=i;
		flushbuffer(sid);
	}
	if (purge) {
		wmserver_purge(sid);
		if (sid->dat->wm!=NULL) {
			if (sid->dat->wm->showdebug) {
				sql_freeresult(&sqr);
				return;
			}
		}
	}
//	if ((purge)&&(sid->dat->wm->remove>0)) wmserver_purge(sid);
//	snprintf(msgnum, sizeof(msgnum)-1, "%d", deleted);
//	if (deleted>=nummessages) deleted=nummessages-1;
	if (sid->dat->menustyle>0) {
		prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
		prints(sid, "parent.wmlist.location=\"%s/mail/list?folderid=%d&offset=%d&order=%d\";\r\n", sid->dat->in_ScriptName, oldfolderid, offset, order);
		prints(sid, "document.location=\"%s/mail/null\";\r\n", sid->dat->in_ScriptName);
		prints(sid, "// -->\r\n</SCRIPT>\r\n");
		prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/null\"></NOSCRIPT>\r\n", sid->dat->in_ScriptName);
	} else {
//		if ((getgetenv(sid, msgnum)==NULL)||(nummessages<2)) {
			prints(sid, "<SCRIPT LANGUAGE=JavaScript>\r\n<!--\r\n");
			prints(sid, "document.location=\"%s/mail/list?folderid=%d&offset=%d&order=%d\";\r\n", sid->dat->in_ScriptName, oldfolderid, offset, order);
			prints(sid, "// -->\r\n</SCRIPT>\r\n");
			prints(sid, "<NOSCRIPT><META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/list?folderid=%d&offset=%d&order=%d\"><NOSCRIPT>\r\n", sid->dat->in_ScriptName, oldfolderid, offset, order);
//		} else {
//			prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=%s/mail/read?msg=%d\">\r\n", sid->dat->in_ScriptName, deleted);
//		}
	}
	sql_freeresult(&sqr);
*/
	return;
}

void webmailpurge(CONN *sid)
{
/*
	struct stat sb;
	char msgfilename[512];
	char newfilename[512];
	char *ptemp;
	int accountid;
	int err;
	int i;
	int j;
	SQLRES sqr;

	if ((ptemp=getgetenv(sid, "ACCOUNTID"))==NULL) return;
	accountid=atoi(ptemp);
	prints(sid, "<BR /><B>Purging and re-indexing account %d ... </B>", accountid);
	flushbuffer(sid);
	if (sql_queryf(&sqr, "SELECT mailheaderid, folder FROM gw_email_headers WHERE obj_uid = %d AND obj_did = %d AND accountid = %d and status = 'd' ORDER BY mailheaderid ASC", sid->dat->uid, sid->dat->did, accountid)<0) return;
	err=0;
	memset(msgfilename, 0, sizeof(msgfilename));
	memset(newfilename, 0, sizeof(newfilename));
	for (i=0;i<sql_numtuples(&sqr);i++) {
		snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->did, accountid, atoi(sql_getvalue(&sqr, i, 1)), atoi(sql_getvalue(&sqr, i, 0)));
		if (stat(msgfilename, &sb)==0) {
			if (unlink(msgfilename)!=0) {
				prints(sid, "<BR /><B>'%s' could not be deleted.</B>\r\n", msgfilename);
				err++;
			}
		}
	}
	sql_freeresult(&sqr);
	if (err==0) {
		sql_updatef("DELETE FROM gw_email_headers WHERE obj_uid = %d AND obj_did = %d AND accountid = %d and status = 'd'", sid->dat->uid, sid->dat->did, accountid);
	}
	if (sql_queryf(&sqr, "SELECT mailheaderid, folder FROM gw_email_headers WHERE obj_uid = %d AND obj_did = %d AND accountid = %d ORDER BY mailheaderid ASC", sid->dat->uid, sid->dat->did, accountid)<0) return;
	for (i=0,j=1;i<sql_numtuples(&sqr);i++) {
		if (j==atoi(sql_getvalue(&sqr, i, 0))) { j++; continue; }
		snprintf(msgfilename, sizeof(msgfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->did, accountid, atoi(sql_getvalue(&sqr, i, 1)), atoi(sql_getvalue(&sqr, i, 0)));
		snprintf(newfilename, sizeof(newfilename)-1, "%s/%04d/mail/%04d/%04d/%06d.msg", config->dir_var_domains, sid->dat->did, accountid, atoi(sql_getvalue(&sqr, i, 1)), j);
		if (rename(msgfilename, newfilename)==0) {
			if (sql_updatef("UPDATE gw_email_headers SET mailheaderid = '%d' WHERE obj_uid = %d AND obj_did = %d AND accountid = %d AND mailheaderid = %d", j, sid->dat->uid, sid->dat->did, accountid, atoi(sql_getvalue(&sqr, i, 0)))<0) {
				prints(sid, "<BR /><B>'%s' could not be moved to '%s'</B>\r\n", msgfilename, newfilename);
				rename(newfilename, msgfilename);
				break;
			}
			j++;
		}
	}
	sql_freeresult(&sqr);
	prints(sid, "<B>done.</B><BR />\r\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript TYPE=text/javascript>\r\n<!--\r\n");
	prints(sid, "window.setTimeout('location.replace(\"%s/mail/accounts/list\")', 2000);\r\n", sid->dat->in_ScriptName);
	prints(sid, "// -->\r\n</SCRIPT>\r\n");
	prints(sid, "<NOSCRIPT>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=%s/mail/accounts/list\">\r\n", sid->dat->in_ScriptName);
	prints(sid, "</NOSCRIPT>\r\n");
*/
	return;
}
