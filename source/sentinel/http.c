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

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

void SwapChar(char *string, char oldchar, char newchar)
{
 	while (*string) {
 		if (*string==oldchar) *string=newchar;
		string++;
	}
}

void FixSlashes(char *pOriginal)
{
#ifdef WIN32
	SwapChar(pOriginal, '/', '\\');
#else
	SwapChar(pOriginal, '\\', '/');
#endif
}

int IntFromHex(char *pChars)
{
	int Hi;
	int Lo;
	int Result;

	Hi=pChars[0];
	if ('0'<=Hi&&Hi<='9') {
		Hi-='0';
	} else if ('a'<=Hi&&Hi<='f') {
		Hi-=('a'-10);
	} else if ('A'<=Hi&&Hi<='F') {
		Hi-=('A'-10);
	}
	Lo = pChars[1];
	if ('0'<=Lo&&Lo<='9') {
		Lo-='0';
	} else if ('a'<=Lo&&Lo<='f') {
		Lo-=('a'-10);
	} else if ('A'<=Lo&&Lo<='F') {
		Lo-=('A'-10);
	}
	Result=Lo+(16*Hi);
	return (Result);
}

void URLDecode(unsigned char *pEncoded)
{
	char *pDecoded;

	pDecoded=pEncoded;
	while (*pDecoded) {
		if (*pDecoded=='+') *pDecoded=' ';
		pDecoded++;
	};
	pDecoded=pEncoded;
	while (*pEncoded) {
		if (*pEncoded=='%') {
			pEncoded++;
			if (isxdigit(pEncoded[0])&&isxdigit(pEncoded[1])) {
				*pDecoded++=(char)IntFromHex(pEncoded);
				pEncoded+=2;
			}
		} else {
			*pDecoded++=*pEncoded++;
		}
	}
	*pDecoded='\0';
}

char *str2html(char *instring)
{
	int sid=getsid();
	unsigned char *buffer=conn[sid].dat->envbuf;
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	while ((instring[i])&&(i<sizeof(conn[sid].dat->envbuf)-1)) {
		ch=instring[i];
		if (ch==0) break;
		if ((ch<32)||(ch>255)) { i++; continue; }
		if (ch=='\'') {
			buffer[bufferlength]='%';
			buffer[bufferlength+1]='2';
			buffer[bufferlength+2]='7';
			bufferlength+=3;
			i++;
			continue;
		}
		buffer[bufferlength]=ch;
		bufferlength++;
		i++;
	}
	return buffer;
}

int verifyimage(char *image)
{
	struct stat sb;
	char file[256];

	if (RunAsCGI) return 1;
	snprintf(file, sizeof(file)-1, "%s/sentinel/images/%s", config.server_http_dir, image);
	FixSlashes(file);
	if (stat(file, &sb)!=0) return 0;
	return 1;
}

char *get_mime_type(char *name)
{
	char *mime_types[40][2]={
		{ ".html", "text/html" },
		{ ".htm",  "text/html" },
		{ ".shtml","text/html" },
		{ ".css",  "text/css" },
		{ ".txt",  "text/plain" },
		{ ".mdb",  "application/msaccess" },
		{ ".xls",  "application/msexcel" },
		{ ".doc",  "application/msword" },
		{ ".exe",  "application/octet-stream" },
		{ ".pdf",  "application/pdf" },
		{ ".rtf",  "application/rtf" },
		{ ".tgz",  "application/x-compressed" },
		{ ".gz",   "application/x-compressed" },
		{ ".z",    "application/x-compress" },
		{ ".swf",  "application/x-shockwave-flash" },
		{ ".tar",  "application/x-tar" },
		{ ".rar",  "application/x-rar-compressed" },
		{ ".zip",  "application/x-zip-compressed" },
		{ ".ra",   "audio/x-pn-realaudio" },
		{ ".ram",  "audio/x-pn-realaudio" },
		{ ".wav",  "audio/x-wav" },
		{ ".gif",  "image/gif" },
		{ ".jpeg", "image/jpeg" },
		{ ".jpe",  "image/jpeg" },
		{ ".jpg",  "image/jpeg" },
		{ ".png",  "image/png" },
		{ ".avi",  "video/avi" },
		{ ".mp3",  "video/mpeg" },
		{ ".mpeg", "video/mpeg" },
		{ ".mpg",  "video/mpeg" },
		{ ".qt",   "video/quicktime" },
		{ ".mov",  "video/quicktime" },
		{ "",      "" }
	};
	char *extension;
	int i;

	extension=strrchr(name, '.');
	if (extension==NULL) {
		return "text/plain";
	}
	i=0;
	while (strlen(mime_types[i][0])>0) {
		if (strcmp(extension, mime_types[i][0])==0) {
			return mime_types[i][1];
		}
		i++;
	}
	return "application/octet-stream";
}

void ReadPOSTData(int sid) {
	char *pPostData;
	int i=0;
	int rc=0;
	int x=0;

	if (conn[sid].PostData!=NULL) {
		free(conn[sid].PostData);
		conn[sid].PostData=NULL;
	}
	conn[sid].PostData=calloc(conn[sid].dat->in_ContentLength+1024, sizeof(char));
	if (conn[sid].PostData==NULL) {
		logerror("Memory allocation error while reading POST data.");
		closeconnect(sid, 1);
	}
	if (RunAsCGI) {
		while (i<conn[sid].dat->in_ContentLength) {
			x=fgetc(stdin);
			if (x==EOF) break;
			conn[sid].PostData[i++]=x;
		}
	} else {
		pPostData=conn[sid].PostData;
		/* reading beyond PostContentLength is required for IE5.5 and NS6 (HTTP 1.1) */
		do {
			rc=recv(conn[sid].socket, pPostData, 1024, 0);
			if (rc==-1) {
				closeconnect(sid, 1);
				return;
			}
			pPostData+=rc;
			x+=rc;
		} while ((rc==1024)||(x<conn[sid].dat->in_ContentLength));
		conn[sid].PostData[conn[sid].dat->in_ContentLength]='\0';
	}
}

char *getgetenv(int sid, char *query)
{
	char *buffer=conn[sid].dat->envbuf;
	char *pToken;
	char *pEquals;
	char pQuery[64];
	int loop;

	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	if (strlen(conn[sid].dat->in_QueryString)==0) return NULL;
	strncpy(conn[sid].dat->envbuf, conn[sid].dat->in_QueryString, sizeof(conn[sid].dat->envbuf)-1);
	strncpy(pQuery, query, sizeof(pQuery)-1);
	loop=0;
	while (pQuery[loop]) {
		pQuery[loop] = toupper(pQuery[loop]);
		loop++;
	}
	pToken = strtok(buffer, "&");
	while (pToken != NULL) {
		pEquals = strchr(pToken, '=');
		if (pEquals != NULL) {
			*pEquals++ = '\0';
			URLDecode(pToken);
			loop = 0;
			while (pToken[loop]) {
				pToken[loop] = toupper(pToken[loop]);
				loop++;
			}
			URLDecode(pEquals);
			if ((strcmp(pQuery, pToken)==0))
				return pEquals;
		}
		pToken = strtok(NULL,"&");
	}
	return NULL;
}

char *getmimeenv(int sid, char *query, unsigned int *buffersize)
{
	char *buffer=NULL;
	char boundary1[100];
	char boundary2[100];
	char pQuery[64];
	char *pPostData;
	int i;

	if (conn[sid].PostData==NULL) return NULL;
	strncpy(pQuery, query, sizeof(pQuery)-1);
	pPostData=conn[sid].PostData;
	*buffersize=0;
	memset(boundary1, 0, sizeof(boundary1));
	memset(boundary2, 0, sizeof(boundary2));
	i=0;
	while (pQuery[i]) {
//	FIXME: not fatal yet, but case sensitivity needs to be removed properly
		pQuery[i]=tolower(pQuery[i]);
		i++;
	}
	i=0;
	/* get the mime boundary */
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(i<conn[sid].dat->in_ContentLength)&&(strlen(boundary1)<sizeof(boundary1)-1)) {
		boundary1[i]=*pPostData;
		pPostData++;
		i++;
	}
	snprintf(boundary2, sizeof(boundary2)-1, "%s--", boundary1);
	while ((strncmp(pPostData, boundary2, sizeof(boundary2))!=0)&&(i<conn[sid].dat->in_ContentLength)) {
		/* eat newline garbage */
		while ((*pPostData=='\r')||(*pPostData=='\n')) {
			pPostData++;
			i++;
		}
		if (strncmp(pPostData, "Content-Disposition: form-data; name=\"", 38)==0) {
			pPostData+=38;
			if (strncmp(pPostData, pQuery, strlen(pQuery)-1)==0) {
				pPostData+=strlen(pQuery)+1;
				while ((*pPostData=='\r')||(*pPostData=='\n')) {
					pPostData++;
					i++;
				}
				if (strncmp(pPostData, boundary1, strlen(boundary1))==0) {
					*buffersize=0;
					return NULL;
				}
				buffer=pPostData;
				while ((strncmp(pPostData, boundary1, strlen(boundary1))!=0)&&(i<conn[sid].dat->in_ContentLength)) {
					*buffersize+=1;
					pPostData++;
					i++;
				}
				if (buffer[*buffersize-1]=='\n') *buffersize-=1;
				if (buffer[*buffersize-1]=='\r') *buffersize-=1;
				if (*buffersize<=0) {
					*buffersize=0;
					return NULL;
				}
				return buffer;
			}
		} else {
			pPostData++;
			i++;
		}
	}
	return NULL;
}

char *getpostenv(int sid, char *query)
{
	char *buffer=conn[sid].dat->envbuf;
	char *pToken;

	if (conn[sid].PostData==NULL) return NULL;
	memset(conn[sid].dat->envbuf, 0, sizeof(conn[sid].dat->envbuf));
	pToken=conn[sid].PostData;
	while (*pToken) {
		if ((strncasecmp(pToken, query, strlen(query))!=0)) {
			while ((*pToken)&&(*pToken!='&')) pToken++;
			if (*pToken=='&') pToken++;
			continue;
		}
		pToken=strchr(pToken, '=');
		if (pToken==NULL) break;
		pToken++;
		while ((*pToken)&&(*pToken!='&')&&(strlen(buffer)<sizeof(conn[sid].dat->envbuf)-1)) {
			buffer[strlen(buffer)]=*pToken;
			pToken++;
		}
		URLDecode(buffer);
		return buffer;
	}
	return NULL;
}

void read_cgienv(int sid)
{
	if (getenv("CONTENT_LENGTH")!=NULL)
		conn[sid].dat->in_ContentLength=atoi(getenv("CONTENT_LENGTH"));
	if (getenv("HTTP_COOKIE")!=NULL)
		strncpy(conn[sid].dat->in_Cookie, getenv("HTTP_COOKIE"), sizeof(conn[sid].dat->in_Cookie)-1);
	if (getenv("HTTP_HOST")!=NULL)
		strncpy(conn[sid].dat->in_Host, getenv("HTTP_HOST"), sizeof(conn[sid].dat->in_Host)-1);
	if (getenv("HTTP_USER_AGENT")!=NULL)
		strncpy(conn[sid].dat->in_UserAgent, getenv("HTTP_USER_AGENT"), sizeof(conn[sid].dat->in_UserAgent)-1);
	if (getenv("PATH_INFO")!=NULL)
		strncpy(conn[sid].dat->in_PathInfo, getenv("PATH_INFO"), sizeof(conn[sid].dat->in_PathInfo)-1);
	if (getenv("QUERY_STRING")!=NULL)
		strncat(conn[sid].dat->in_QueryString, getenv("QUERY_STRING"), sizeof(conn[sid].dat->in_QueryString)-1);
	if (getenv("REMOTE_ADDR")!=NULL)
		strncat(conn[sid].dat->in_ClientIP, getenv("REMOTE_ADDR"), sizeof(conn[sid].dat->in_ClientIP)-1);
	if (getenv("REQUEST_METHOD")!=NULL)
		strncpy(conn[sid].dat->in_RequestMethod, getenv("REQUEST_METHOD"), sizeof(conn[sid].dat->in_RequestMethod)-1);
	if (getenv("SCRIPT_NAME")!=NULL)
		strncat(conn[sid].dat->in_ScriptName, getenv("SCRIPT_NAME"), sizeof(conn[sid].dat->in_ScriptName)-1);
	strncpy(conn[sid].dat->in_RequestURI, conn[sid].dat->in_PathInfo, sizeof(conn[sid].dat->in_RequestURI)-1);
	if (strlen(conn[sid].dat->in_QueryString)>0) {
		strncat(conn[sid].dat->in_RequestURI, "?", sizeof(conn[sid].dat->in_RequestURI)-strlen(conn[sid].dat->in_RequestURI)-1);
		strncat(conn[sid].dat->in_RequestURI, conn[sid].dat->in_QueryString, sizeof(conn[sid].dat->in_RequestURI)-strlen(conn[sid].dat->in_RequestURI)-1);
	}
	if (strlen(conn[sid].dat->in_RequestURI)==0)
		strncpy(conn[sid].dat->in_RequestURI, "/", sizeof(conn[sid].dat->in_RequestURI)-1);
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if (conn[sid].dat->in_ContentLength<MAX_POSTSIZE) {
			ReadPOSTData(sid);
		} else {
			// try to print an error : note the inbuffer being full may block us
			printerror(sid, 413, "Bad Request", "Request entity too large.");
			closeconnect(sid, 1);
		}
	}
	return;
}

int read_header(int sid)
{
	char line[2048];
	char *pTemp;
	time_t x;

	strncpy(conn[sid].dat->in_ClientIP, inet_ntoa(conn[sid].ClientAddr.sin_addr), sizeof(conn[sid].dat->in_ClientIP)-1);
	x=time((time_t*)0);
	do {
		memset(line, 0, sizeof(line));
		sgets(line, sizeof(line), conn[sid].socket);
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) line[strlen(line)-1]='\0';
	} while ((strlen(line)==0)&&((time((time_t)0)-x)<30));
	if ((strlen(line)==0)&&((time((time_t)0)-x)>=30)) {
#ifdef DEBUG
logdata("\n[[[ KILLING IDLE KEEPALIVE ]]]\n");
#endif
		closeconnect(sid, 1);
	}
#ifdef DEBUG
logdata("\n[[[ STARTING REQUEST ]]]\n");
#endif
	if (strlen(line)==0)
		printerror(sid, 400, "Bad Request", "No Request Found.");
	if (sscanf(line, "%[^ ] %[^ ] %[^ ]", conn[sid].dat->in_RequestMethod, conn[sid].dat->in_RequestURI, conn[sid].dat->in_Protocol)!=3)
		printerror(sid, 400, "Bad Request", "Can't Parse Request.");
	pTemp=conn[sid].dat->in_RequestMethod;
	while (*pTemp) { *pTemp=toupper(*pTemp); pTemp++; };
	while (strlen(line)>0) {
		sgets(line, sizeof(line)-1, conn[sid].socket);
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) line[strlen(line)-1]='\0';
		if (strncasecmp(line, "Connection: ", 12)==0)
			strncpy(conn[sid].dat->in_Connection, (char *)&line+12, sizeof(conn[sid].dat->in_Connection)-1);
		if (strncasecmp(line, "Content-Length: ", 16)==0) 
			conn[sid].dat->in_ContentLength=atoi((char *)&line+16);
		if (strncasecmp(line, "Cookie: ", 8)==0)
			strncpy(conn[sid].dat->in_Cookie, (char *)&line+8, sizeof(conn[sid].dat->in_Cookie)-1);
		if (strncasecmp(line, "Host: ", 6)==0)
			strncpy(conn[sid].dat->in_Host, (char *)&line+6, sizeof(conn[sid].dat->in_Host)-1);
		if (strncasecmp(line, "If-Modified-Since: ", 19)==0)
			strncpy(conn[sid].dat->in_IfModifiedSince, (char *)&line+19, sizeof(conn[sid].dat->in_IfModifiedSince)-1);
		if (strncasecmp(line, "User-Agent: ", 12)==0)
			strncpy(conn[sid].dat->in_UserAgent, (char *)&line+12, sizeof(conn[sid].dat->in_UserAgent)-1);
	}
	if ((strcmp(conn[sid].dat->in_RequestMethod, "GET")!=0)&&(strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0)) {
		printerror(sid, 501, "Not Implemented", "That method is not implemented.");
		closeconnect(sid, 1);
		return -1;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if (conn[sid].dat->in_ContentLength<MAX_POSTSIZE) {
			ReadPOSTData(sid);
		} else {
			// try to print an error : note the inbuffer being full may block us
			// FIXME: this is causing the children to segfault in win32
			printerror(sid, 413, "Bad Request", "Request entity too large.");
			logerror("%s - Large POST (>%d bytes) disallowed", conn[sid].dat->in_ClientIP, MAX_POSTSIZE);
			closeconnect(sid, 1);
			return -1;
		}
	}
	if (conn[sid].dat->in_RequestURI[0]!='/') {
		printerror(sid, 400, "Bad Request", "Bad filename.");
	}
	if (strchr(conn[sid].dat->in_RequestURI, '?')!=NULL) {
		strncpy(conn[sid].dat->in_QueryString, strchr(conn[sid].dat->in_RequestURI, '?')+1, sizeof(conn[sid].dat->in_QueryString)-1);
	}
	return 0;
}

void send_header(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod)
{
	char timebuf[100];
	time_t now;

	if (status) {
		conn[sid].dat->out_status=status;
	} else {
		conn[sid].dat->out_status=200;
	}
	if (length>=0) {
		conn[sid].dat->out_ContentLength=length;
	}
	if (mod!=(time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		snprintf(conn[sid].dat->out_LastModified, sizeof(conn[sid].dat->out_LastModified)-1, "%s", timebuf);
	}
	now=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	snprintf(conn[sid].dat->out_Date, sizeof(conn[sid].dat->out_Date)-1, "%s", timebuf);
	if (cacheable) {
		snprintf(conn[sid].dat->out_CacheControl, sizeof(conn[sid].dat->out_CacheControl)-1, "public");
		snprintf(conn[sid].dat->out_Pragma, sizeof(conn[sid].dat->out_Pragma)-1, "public");
	} else {
		snprintf(conn[sid].dat->out_CacheControl, sizeof(conn[sid].dat->out_CacheControl)-1, "no-store");
		snprintf(conn[sid].dat->out_Expires, sizeof(conn[sid].dat->out_Expires)-1, "%s", timebuf);
		snprintf(conn[sid].dat->out_Pragma, sizeof(conn[sid].dat->out_Pragma)-1, "no-cache");
	}
	if (extra_header!=(char*)0) {
		snprintf(conn[sid].dat->out_ContentType, sizeof(conn[sid].dat->out_ContentType)-1, "%s", mime_type);
	} else {
		snprintf(conn[sid].dat->out_ContentType, sizeof(conn[sid].dat->out_ContentType)-1, "text/html");
	}
}

void send_fileheader(int sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod)
{
	char timebuf[100];
	time_t now;

	if (status) {
		conn[sid].dat->out_status=status;
	} else {
		conn[sid].dat->out_status=200;
	}
	if (strcasestr(conn[sid].dat->in_Protocol, "HTTP/1.1")!=NULL) {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.1");
	} else {
		snprintf(conn[sid].dat->out_Protocol, sizeof(conn[sid].dat->out_Protocol)-1, "HTTP/1.0");
	}
	if (strcasecmp(conn[sid].dat->in_Connection, "Keep-Alive")==0) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Keep-Alive");
	} else {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	// Nutscrape and Mozilla don't know what a fucking keepalive is
	if ((strcasestr(conn[sid].dat->in_UserAgent, "MSIE")==NULL)) {
		snprintf(conn[sid].dat->out_Connection, sizeof(conn[sid].dat->out_Connection)-1, "Close");
	}
	if (!RunAsCGI) {
//		prints("HTTP/1.1 200 OK\r\n");
		prints("%s %d OK\r\n", conn[sid].dat->out_Protocol, conn[sid].dat->out_status);
		prints("Connection: %s\r\n", conn[sid].dat->out_Connection);
	}
	prints("Server: %s\r\n", SERVER_NAME);
	if ((length>=0)&&(status!=304)) {
		prints("Content-Length: %d\r\n", length);
	}
	now=time((time_t*)0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	prints("Date: %s\r\n", timebuf);
	if (mod!=(time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		prints("Last-Modified: %s\r\n", timebuf);
	}
	if (cacheable) {
		now=time((time_t*)0)+604800;
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
		prints("Expires: %s\r\n", timebuf);
		prints("Cache-Control: public\r\n");
		prints("Pragma: public\r\n");
	} else {
		now=time((time_t*)0);
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
		prints("Expires: %s\r\n", timebuf);
		prints("Cache-Control: no-store\r\n");
		prints("Pragma: no-cache\r\n");
	}
	if (extra_header!=(char*)0) {
		prints("Content-Type: %s\r\n\r\n", mime_type);
	} else {
		prints("Content-Type: text/html\r\n\r\n");
	}
	conn[sid].dat->out_headdone=1;
	flushbuffer(sid);
}

/* the rest is all ugly debug code */
/*
void PrintVars()
{
	int i = 0;
	char Buffer[8192];
	
	prints("<B>Environment Variables</B>\n");
	prints("<UL>");
	while(environ[i]) {
		strncpy(Buffer, environ[i], sizeof(Buffer)-1);
		URLDecode(Buffer);
		prints("<LI>%s\n",Buffer);
		i++;
	}
	prints("</UL>\n");
}

void PrintOut(char * VarVal)
{
	char *pEquals;
	int i;

	pEquals = strchr(VarVal, '=');
	if (pEquals != NULL) {
		*pEquals++ = '\0';
		URLDecode(VarVal);
		i = 0;
		while (VarVal[i]) {
			VarVal[i] = toupper(VarVal[i]);
			i++;
		}  
		URLDecode(pEquals);
		prints("<li>%s=%s\n",VarVal,pEquals);
	}
}

void PrintPOSTData()
{
	char * pToken;
	char Buffer[8192];

	strncpy(Buffer, conn[getsid()].in.PostData, sizeof(Buffer)-1);
	prints("<b>POST Data</b>\n");
	prints("<ul>\n");
	pToken = strtok(Buffer, "&");
	while (pToken != NULL) {
		PrintOut (pToken);
		pToken = strtok(NULL,"&");
	}
	prints("</ul>\n");
}

void PrintGETData()
{
	char * pToken;
	char Buffer[8192];

	if (getenv("QUERY_STRING")==NULL)
		return;
	strncpy(Buffer, getenv("QUERY_STRING"), sizeof(Buffer)-1);
	prints("<b>GET Data</b>\n");
	prints("<ul>\n");
	pToken = strtok(Buffer, "&");
	while (pToken != NULL) {
		PrintOut (pToken);
		pToken = strtok(NULL,"&");
	}
	prints("</ul>\n");
}

void printenv()
{
	char *pRequestMethod;

	pRequestMethod=conn[getsid()].in.RequestMethod;
	if (pRequestMethod==NULL)
		return;
	prints("<FONT COLOR=#000000>");
	PrintVars();
	PrintGETData();
	if (strcmp(pRequestMethod,"POST")==0)
		PrintPOSTData();
}
*/
