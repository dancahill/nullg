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
#include "http_main.h"

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

char *get_mime_type(char *name)
{
	char *mime_types[43][2]={
		{ ".css",  "text/css" },
		{ ".htm",  "text/html" },
		{ ".html", "text/html" },
		{ ".shtml","text/html" },
		{ ".js",   "text/javascript" },
		{ ".asp",  "text/plain" },
		{ ".c",    "text/plain" },
		{ ".cc",   "text/plain" },
		{ ".cfg",  "text/plain" },
		{ ".cpp",  "text/plain" },
		{ ".h",    "text/plain" },
		{ ".log",  "text/plain" },
		{ ".php",  "text/plain" },
		{ ".pl",   "text/plain" },
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
		{ NULL,     NULL }
	};
	char *extension;
	int i;

	extension=strrchr(name, '.');
	if (extension==NULL) {
		return "text/plain";
	}
	i=0;
	while (mime_types[i][0]!=NULL) {
		if (strcmp(extension, mime_types[i][0])==0) {
			return mime_types[i][1];
		}
		i++;
	}
	return "application/octet-stream";
}

void ReadPOSTData(CONN *sid) {
	char *pPostData;
	int bytesleft;
	int rc;

	bytesleft=sid->dat->in_ContentLength;
	if (bytesleft<0) {
		logerror(sid, __FILE__, __LINE__, 1, "ERROR: negative Content-Length of %d provided by client.", sid->dat->in_ContentLength);
		sid->dat->in_ContentLength=0;
		closeconnect(sid, 1);
		return;
	} else if (bytesleft>proc.config.http_maxpostsize) {
		logerror(sid, __FILE__, __LINE__, 1, "ERROR: Content-Length of %d is too large.", sid->dat->in_ContentLength);
		sid->dat->in_ContentLength=0;
		closeconnect(sid, 1);
		return;
	}
	if (sid->PostData!=NULL) {
		free(sid->PostData);
		sid->PostData=NULL;
	}
	sid->PostData=calloc(sid->dat->in_ContentLength+4, sizeof(char));
	if (sid->PostData==NULL) {
		logerror(sid, __FILE__, __LINE__, 1, "Memory allocation error while reading POST data.");
		closeconnect(sid, 1);
	}
	pPostData=sid->PostData;
	if (proc.RunAsCGI) {
		while (bytesleft>0) {
			rc=read(fileno(stdin), pPostData, bytesleft);
			if (rc<0) {
				closeconnect(sid, 1);
				return;
			}
			sid->atime=time(NULL);
			bytesleft-=rc;
			pPostData+=rc;
		}
	} else {
		/*
		 * If we already snorted part of the POST data while reading
		 * the header, then copy it here.
		 */
		if (sid->dat->recvbufsize>bytesleft) {
			logerror(sid, __FILE__, __LINE__, 1, "ERROR: Content-Length is larger than advertised.");
			sid->dat->in_ContentLength=0;
			closeconnect(sid, 1);
			return;
		}
		memcpy(pPostData, sid->dat->recvbuf+sid->dat->recvbufoffset, sid->dat->recvbufsize);
		memset(sid->dat->recvbuf, 0, sizeof(sid->dat->recvbuf));
		pPostData+=sid->dat->recvbufsize;
		bytesleft-=sid->dat->recvbufsize;
		sid->dat->recvbufoffset=0;
		sid->dat->recvbufsize=0;
		while (bytesleft>0) {
			rc=recv(sid->socket, pPostData, bytesleft, 0);
			if (rc<0) {
				closeconnect(sid, 1);
				return;
			}
			sid->atime=time(NULL);
			bytesleft-=rc;
			pPostData+=rc;
		}
	}
//	logerror(sid, __FILE__, __LINE__, 4, "[%s]", sid->PostData);
	return;
}

char *getgetenv(CONN *sid, char *query)
{
	char *buffer=getbuffer(sid);
	char *pToken;
	char *pEquals;
	char pQuery[64];
	int loop;

	if (strlen(sid->dat->in_QueryString)==0) return NULL;
	strncpy(buffer, sid->dat->in_QueryString, sizeof(sid->dat->smallbuf[0])-1);
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
			decodeurl(pToken);
			loop = 0;
			while (pToken[loop]) {
				pToken[loop] = toupper(pToken[loop]);
				loop++;
			}
			decodeurl(pEquals);
			if ((strcmp(pQuery, pToken)==0))
				return pEquals;
		}
		pToken = strtok(NULL,"&");
	}
	return NULL;
}

char *getmimeenv(CONN *sid, char *query, unsigned int *buffersize)
{
	char *buffer=NULL;
	char boundary1[100];
	char boundary2[100];
	char queryl[64];
	char queryt[64];
	char *pPostData;
	int index=0;
	unsigned int i;

	if (sid->PostData==NULL) return NULL;
	*buffersize=0;
	memset(boundary1, 0, sizeof(boundary1));
	memset(boundary2, 0, sizeof(boundary2));
	memset(queryl, 0, sizeof(queryl));
	memset(queryt, 0, sizeof(queryt));
	for (i=0;query[i]!='\0';i++) {
		queryl[i]=tolower(query[i]);
	}
	pPostData=sid->PostData;
	i=0;
	while ((*pPostData!='\r')&&(*pPostData!='\n')&&(index<sid->dat->in_ContentLength)&&(i<sizeof(boundary1)-1)) {
		boundary1[i]=*pPostData;
		pPostData++;
		index++;
		i++;
	}
	snprintf(boundary2, sizeof(boundary2)-1, "%s--", boundary1);
	while ((strncmp(pPostData, boundary2, sizeof(boundary2))!=0)&&(index<sid->dat->in_ContentLength)) {
		while ((*pPostData=='\r')||(*pPostData=='\n')) {
			pPostData++;
			index++;
		}
		if (strncasecmp(pPostData, "Content-Disposition: form-data; name=", 37)==0) {
			pPostData+=37;
			if (*pPostData=='\"') pPostData++;
			memset(queryt, 0, sizeof(queryt));
			i=0;
			while ((*pPostData)&&(*pPostData!='\"')&&(*pPostData!='\r')&&(*pPostData!='\n')) {
				queryt[i++]=tolower(*pPostData++);
				index++;
			}
			while ((*pPostData)&&(*pPostData!='\r')&&(*pPostData!='\n')) {
				pPostData++;
				index++;
			}
			if (*pPostData=='\r') { pPostData++; index++; }
			if (*pPostData=='\n') { pPostData++; index++; }
			while ((*pPostData)&&(*pPostData!='\r')&&(*pPostData!='\n')) {
				while ((*pPostData)&&(*pPostData!='\r')&&(*pPostData!='\n')) {
					pPostData++;
					index++;
				}
				if (*pPostData=='\r') { pPostData++; index++; }
				if (*pPostData=='\n') { pPostData++; index++; }
			}
			if (*pPostData=='\r') { pPostData++; index++; }
			if (*pPostData=='\n') { pPostData++; index++; }
			if (strcmp(queryl, queryt)==0) {
				buffer=pPostData;
				for (i=0;;i++) {
					if (strncmp(pPostData, boundary1, strlen(boundary1))==0) break;
					if (index>sid->dat->in_ContentLength) break;
					*buffersize+=1;
					pPostData++;
					index++;
				}
				if (buffer[*buffersize-1]=='\n') *buffersize-=1;
				if (buffer[*buffersize-1]=='\r') *buffersize-=1;
				if (*buffersize<0) {
					*buffersize=0;
					buffer=NULL;
				}
				return buffer;
			}
		} else {
			pPostData++;
			index++;
		}
	}
	return NULL;
}

char *getpostenv(CONN *sid, char *query)
{
	char *buffer=sid->dat->largebuf;
	char *pToken;

	if (sid->PostData==NULL) return NULL;
	memset(sid->dat->largebuf, 0, sizeof(sid->dat->largebuf));
	pToken=sid->PostData;
	while (*pToken) {
		if ((strncasecmp(pToken, query, strlen(query))!=0)) {
			while ((*pToken)&&(*pToken!='&')) pToken++;
			if (*pToken=='&') pToken++;
			continue;
		}
		if ((*pToken)&&(pToken[strlen(query)]!='=')) { pToken++; continue; }
		pToken=strchr(pToken, '=');
		if (pToken==NULL) break;
		pToken++;
		while ((*pToken)&&(*pToken!='&')&&(strlen(buffer)<sizeof(sid->dat->largebuf)-1)) {
			buffer[strlen(buffer)]=*pToken;
			pToken++;
		}
		decodeurl(buffer);
		return buffer;
	}
	return NULL;
}

char *getxmlenv(CONN *sid, char *query)
{
	char *buffer=getbuffer(sid);
	char *pToken;

	if (sid->PostData==NULL) return NULL;
	pToken=sid->PostData;
	while (*pToken) {
		if ((strncasecmp(pToken, query, strlen(query))!=0)) {
			while ((*pToken)&&(*pToken!='<')) pToken++;
			if (*pToken=='<') pToken++;
			continue;
		}
		if ((*pToken)&&(pToken[strlen(query)]!=' ')&&(pToken[strlen(query)]!='/')&&(pToken[strlen(query)]!='>')) { pToken++; continue; }
		while ((*pToken)&&(*pToken==' ')) {
			pToken++;
		}
		if (*pToken=='/') return "";
		pToken=strchr(pToken, '>');
		if (pToken==NULL) break;
		pToken++;
		while ((*pToken)&&(*pToken!='<')&&(strlen(buffer)<sizeof(sid->dat->smallbuf[0])-1)) {
			buffer[strlen(buffer)]=*pToken;
			pToken++;
		}
		decodeurl(buffer);
		return buffer;
	}
	return NULL;
}

char *getxmlparam(CONN *sid, int param, char *reqtype)
{
	char *buffer=getbuffer(sid);
	char *pToken;
	char type[20];
	int params=0;

	if (sid->PostData==NULL) return NULL;
	memset(type, 0, sizeof(type));
	snprintf(type, sizeof(type)-1, "<%s>", reqtype);
	pToken=sid->PostData;
	while (*pToken) {
		while ((strncasecmp(pToken, "<param>", strlen("<param>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		params++;
		pToken+=strlen("<param>");
		if (param!=params) continue;
		while ((strncasecmp(pToken, "<value>", strlen("<value>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("<value>");
		while ((strncasecmp(pToken, type, strlen(type))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen(type);
		if (param==params) {
			while ((*pToken)&&(*pToken!='<')&&(strlen(buffer)<sizeof(sid->dat->smallbuf[0])-1)) {
				buffer[strlen(buffer)]=*pToken;
				pToken++;
			}
			decodeurl(buffer);
			return buffer;
		}
	}
	return NULL;
}

char *getxmlstruct(CONN *sid, char *reqmember, char *reqtype)
{
	char *buffer=getbuffer(sid);
	char *pToken;
	char type[32];

	if (sid->PostData==NULL) return NULL;
	memset(type, 0, sizeof(type));
	pToken=sid->PostData;
	while ((strncasecmp(pToken, "<struct>", strlen("<struct>"))!=0)) {
		if (*pToken=='\0') return NULL;
		pToken++;
	}
	pToken+=strlen("<struct>");
	while (*pToken) {
		while ((strncasecmp(pToken, "<member>", strlen("<member>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("<member>");
		while ((strncasecmp(pToken, "<name>", strlen("<name>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("<name>");
		/* if the name matches, we never leave this block, btw */
		if ((strncasecmp(pToken, reqmember, strlen(reqmember))==0)) {
			pToken+=strlen(reqmember);
			if (*pToken!='<') continue; /* oops..  wrong member.. never mind what i just said */
			while ((strncasecmp(pToken, "</name>", strlen("</name>"))!=0)) {
				if (*pToken=='\0') return NULL;
				pToken++;
			}
			pToken+=strlen("</name>");
			while ((strncasecmp(pToken, "<value>", strlen("<value>"))!=0)) {
				if (*pToken=='\0') return NULL;
				pToken++;
			}
			pToken+=strlen("<value>");
			while ((strncasecmp(pToken, "<", strlen("<"))!=0)) {
				if (*pToken=='\0') return NULL;
				pToken++;
			}
			pToken+=strlen("<");
			while ((*pToken)&&(*pToken!='>')&&(strlen(type)<sizeof(type)-1)) {
				type[strlen(type)]=*pToken;
				pToken++;
			}
			if (*pToken=='>') pToken++;
			while ((*pToken)&&(*pToken!='<')&&(strlen(buffer)<sizeof(sid->dat->smallbuf[0])-1)) {
				buffer[strlen(buffer)]=*pToken;
				pToken++;
			}
			decodeurl(buffer);
			return buffer;
		}
		while ((strncasecmp(pToken, "</name>", strlen("</name>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("</name>");
		while ((strncasecmp(pToken, "</member>", strlen("</member>"))!=0)) {
			if (*pToken=='\0') return NULL;
			pToken++;
		}
		pToken+=strlen("</member>");
	}
	return NULL;
}

void read_cgienv(CONN *sid)
{
	char *ptemp;
	struct stat sb;
	char progname[255];
	char scriptname[255];
	int length;

	if ((ptemp=getenv("CONTENT_LENGTH"))!=NULL) {
		sid->dat->in_ContentLength=atoi(ptemp);
		if (sid->dat->in_ContentLength<0) {
			logerror(sid, __FILE__, __LINE__, 2, "ERROR: negative Content-Length of %d provided by client.", sid->dat->in_ContentLength);
			sid->dat->in_ContentLength=0;
		}
	}
	if ((ptemp=getenv("HTTP_COOKIE"))!=NULL)     strncpy(sid->dat->in_Cookie, ptemp, sizeof(sid->dat->in_Cookie)-1);
	if ((ptemp=getenv("HTTP_HOST"))!=NULL)       strncpy(sid->dat->in_Host, ptemp, sizeof(sid->dat->in_Host)-1);
	if ((ptemp=getenv("HTTP_USER_AGENT"))!=NULL) strncpy(sid->dat->in_UserAgent, ptemp, sizeof(sid->dat->in_UserAgent)-1);
	if ((ptemp=getenv("PATH_INFO"))!=NULL)       strncpy(sid->dat->in_PathInfo, ptemp, sizeof(sid->dat->in_PathInfo)-1);
	if ((ptemp=getenv("QUERY_STRING"))!=NULL)    strncat(sid->dat->in_QueryString, ptemp, sizeof(sid->dat->in_QueryString)-1);
	if ((ptemp=getenv("REMOTE_ADDR"))!=NULL)     strncat(sid->dat->in_RemoteAddr, ptemp, sizeof(sid->dat->in_RemoteAddr)-1);
	if ((ptemp=getenv("REQUEST_METHOD"))!=NULL)  strncpy(sid->dat->in_RequestMethod, ptemp, sizeof(sid->dat->in_RequestMethod)-1);
	if ((ptemp=getenv("SCRIPT_NAME"))!=NULL)     strncat(sid->dat->in_ScriptName, ptemp, sizeof(sid->dat->in_ScriptName)-1);
	strncpy(sid->dat->in_RequestURI, sid->dat->in_PathInfo, sizeof(sid->dat->in_RequestURI)-1);
	if (strlen(sid->dat->in_QueryString)>0) {
		strncat(sid->dat->in_RequestURI, "?", sizeof(sid->dat->in_RequestURI)-strlen(sid->dat->in_RequestURI)-1);
		strncat(sid->dat->in_RequestURI, sid->dat->in_QueryString, sizeof(sid->dat->in_RequestURI)-strlen(sid->dat->in_RequestURI)-1);
	}
	if (strlen(sid->dat->in_RequestURI)==0) {
		strncpy(sid->dat->in_RequestURI, "/", sizeof(sid->dat->in_RequestURI)-1);
	}
//////////////////
	strncpy(sid->dat->in_CGIScriptName, sid->dat->in_RequestURI, sizeof(sid->dat->in_CGIScriptName)-1);
	if ((ptemp=strchr(sid->dat->in_CGIScriptName, '?'))!=NULL) *ptemp='\0';
//	AHHH!!!
//	if (sid->dat->in_CGIScriptName[strlen(sid->dat->in_CGIScriptName)-1]=='/') sid->dat->in_CGIScriptName[strlen(sid->dat->in_CGIScriptName)-1]='\0';
	if (strncmp(sid->dat->in_CGIScriptName, "/cgi-bin/", 9)==0) {
		snprintf(progname, sizeof(progname)-1, "%s", sid->dat->in_CGIScriptName+9);
		snprintf(scriptname, sizeof(scriptname)-1, "%s/%s", proc.config.server_dir_cgi, progname);
	} else if (strncmp(sid->dat->in_CGIScriptName, "/", 1)==0) {
		snprintf(progname, sizeof(progname)-1, "%s", sid->dat->in_CGIScriptName+1);
		snprintf(scriptname, sizeof(scriptname)-1, "%s/%s", proc.config.server_dir_var_htdocs, progname);
	}
	ptemp=scriptname;
 	while (*ptemp) { if (*ptemp=='\\') *ptemp='/'; ptemp++; }
	length=strlen(scriptname);
	do {
		if ((stat(scriptname, &sb)==0)&&(!(sb.st_mode&S_IFDIR))) break;
		if ((ptemp=strrchr(scriptname, '/'))!=NULL) {
			*ptemp='\0';
		} else {
			break;
		}
	} while (strlen(scriptname)>0);
	fixslashes(scriptname);
	length=length-strlen(scriptname);
	if (length>0) {
		snprintf(sid->dat->in_CGIPathInfo, sizeof(sid->dat->in_CGIPathInfo)-1, "%s", progname+(strlen(progname)-length));
		sid->dat->in_CGIScriptName[strlen(sid->dat->in_CGIScriptName)-length]='\0';
	}
//////////////////
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		if (sid->dat->in_ContentLength<proc.config.http_maxpostsize) {
			ReadPOSTData(sid);
		} else {
			// try to print an error : note the inbuffer being full may block us
			send_error(sid, 413, "Bad Request", "Request entity too large.");
			closeconnect(sid, 1);
		}
	}
	return;
}

int read_header(CONN *sid)
{
	char alter_uri[1024];
	char line[2048];
	char *ptemp;
	char *ptemp2;
	time_t x;

	struct stat sb;
	char progname[255];
	char scriptname[255];
	int length;

	DEBUG_IN(sid, "read_header()");
	strncpy(sid->dat->in_RemoteAddr, inet_ntoa(sid->ClientAddr.sin_addr), sizeof(sid->dat->in_RemoteAddr)-1);
	x=time(NULL);
	do {
		memset(line, 0, sizeof(line));
		if (sgets(sid, line, sizeof(line)-1, sid->socket)<0) {
			DEBUG_OUT(sid, "read_header()");
			return -1;
		}
		striprn(line);
	} while ((strlen(line)==0)&&((time(NULL)-x)<15));
	if ((strlen(line)==0)&&((time(NULL)-x)>=15)) {
		DEBUG_OUT(sid, "read_header()");
		closeconnect(sid, 2);
		return -1;
	}
	sid->state=1;
	if (strlen(line)==0) {
		send_error(sid, 400, "Bad Request", "No Request Found.");
	}
	ptemp=line;
	snprintf(sid->dat->in_RequestMethod, sizeof(sid->dat->in_RequestMethod)-1, "%s", ptemp);
	while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
	if (*ptemp) ptemp++;
	ptemp2=sid->dat->in_RequestMethod;
	while ((*ptemp2)&&(*ptemp2!=' ')) ptemp2++;
	if (*ptemp2) *ptemp2='\0';
	snprintf(sid->dat->in_RequestURI, sizeof(sid->dat->in_RequestURI)-1, "%s", ptemp);
	while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
	if (*ptemp) ptemp++;
	ptemp2=sid->dat->in_RequestURI;
	while ((*ptemp2)&&(*ptemp2!=' ')) ptemp2++;
	if (*ptemp2) *ptemp2='\0';
	snprintf(sid->dat->in_Protocol, sizeof(sid->dat->in_Protocol)-1, "%s", ptemp);
	while ((*ptemp)&&(*ptemp!=' ')) ptemp++;
	if (*ptemp) ptemp++;
	ptemp2=sid->dat->in_Protocol;
	while ((*ptemp2)&&(*ptemp2!=' ')) ptemp2++;
	if (*ptemp2) *ptemp2='\0';
//	logerror(sid, __FILE__, __LINE__, 2, "[%s]", line);
//	logerror(sid, __FILE__, __LINE__, 2, "[%s][%s][%s]", sid->dat->in_RequestMethod, sid->dat->in_RequestURI, sid->dat->in_Protocol);
	if (strlen(sid->dat->in_RequestMethod)==0) send_error(sid, 400, "Bad Request", "Method not found in request.");
	if (strlen(sid->dat->in_RequestURI)==0) send_error(sid, 400, "Bad Request", "URI not found in request.");
	if (strlen(sid->dat->in_Protocol)==0) send_error(sid, 400, "Bad Request", "Protocol not found in request.");
//	if (sscanf(line, "%[^ ] %[^ ] %[^ ]", sid->dat->in_RequestMethod, sid->dat->in_RequestURI, sid->dat->in_Protocol)!=3)
//		send_error(sid, 400, "Bad Request", "Can't Parse Request.");
	ptemp=sid->dat->in_RequestMethod;
	while (*ptemp) { *ptemp=toupper(*ptemp); ptemp++; };
	memset(alter_uri, 0, sizeof(alter_uri));
	while (strlen(line)>0) {
		if (sgets(sid, line, sizeof(line)-1, sid->socket)<0) return -1;
		striprn(line);
		if (strncasecmp(line, "Connection: ", 12)==0) {
			strncpy(sid->dat->in_Connection, (char *)&line+12, sizeof(sid->dat->in_Connection)-1);
		} else if (strncasecmp(line, "Content-Length: ", 16)==0) {
			sid->dat->in_ContentLength=atoi((char *)&line+16);
			if (sid->dat->in_ContentLength<0) {
				// Negative Content-Length?  If so, the client is either broken or malicious.
				// Thanks to <ilja@idefense.be> for spotting this one (Null httpd).
				logerror(sid, __FILE__, __LINE__, 1, "ERROR: negative Content-Length of %d provided by client.", sid->dat->in_ContentLength);
				sid->dat->in_ContentLength=0;
			}
		} else if (strncasecmp(line, "Cookie: ", 8)==0) {
			if (strlen(sid->dat->in_Cookie)) {
				strncat(sid->dat->in_Cookie, ";", sizeof(sid->dat->in_Cookie)-strlen(sid->dat->in_Cookie)-1);
			}
			strncat(sid->dat->in_Cookie, (char *)&line+8, sizeof(sid->dat->in_Cookie)-strlen(sid->dat->in_Cookie)-1);
		} else if (strncasecmp(line, "Host: ", 6)==0) {
			strncpy(sid->dat->in_Host, (char *)&line+6, sizeof(sid->dat->in_Host)-1);
		} else if (strncasecmp(line, "If-Modified-Since: ", 19)==0) {
			strncpy(sid->dat->in_IfModifiedSince, (char *)&line+19, sizeof(sid->dat->in_IfModifiedSince)-1);
		} else if (strncasecmp(line, "User-Agent: ", 12)==0) {
			strncpy(sid->dat->in_UserAgent, (char *)&line+12, sizeof(sid->dat->in_UserAgent)-1);
		} else if (strncasecmp(line, "FCGI-URI: ", 10)==0) {
			strncpy(alter_uri, (char *)&line+10, sizeof(alter_uri)-1);
		}
	}
	if (strlen(alter_uri)) {
		sid->dat->cgi_lite=1;
		strncpy(sid->dat->in_ScriptName, alter_uri, sizeof(sid->dat->in_ScriptName)-1);
	}
	strncpy(sid->dat->in_PathInfo, sid->dat->in_RequestURI, sizeof(sid->dat->in_PathInfo)-1);
	if ((ptemp=strchr(sid->dat->in_PathInfo, '?'))!=NULL) *ptemp='\0';
	if ((strcmp(sid->dat->in_RequestMethod, "GET")!=0)&&(strcmp(sid->dat->in_RequestMethod, "POST")!=0)) {
		send_error(sid, 501, "Not Implemented", "That method is not implemented.");
		DEBUG_OUT(sid, "read_header()");
		closeconnect(sid, 1);
		return -1;
	}
	if (strcmp(sid->dat->in_RequestMethod, "POST")==0) {
		if (sid->dat->in_ContentLength<proc.config.http_maxpostsize) {
			ReadPOSTData(sid);
		} else {
			// try to print an error : note the inbuffer being full may block us
			send_error(sid, 413, "Bad Request", "Request entity too large.");
			logerror(sid, __FILE__, __LINE__, 1, "%s - Large POST (>%d bytes) disallowed", sid->dat->in_RemoteAddr, proc.config.http_maxpostsize);
			DEBUG_OUT(sid, "read_header()");
			closeconnect(sid, 1);
			return -1;
		}
	}
	if (sid->dat->in_RequestURI[0]!='/') {
		send_error(sid, 400, "Bad Request", "Bad filename.");
	}
	if (strchr(sid->dat->in_RequestURI, '?')!=NULL) {
		strncpy(sid->dat->in_QueryString, strchr(sid->dat->in_RequestURI, '?')+1, sizeof(sid->dat->in_QueryString)-1);
	}
//////////////////
	strncpy(sid->dat->in_CGIScriptName, sid->dat->in_RequestURI, sizeof(sid->dat->in_CGIScriptName)-1);
	if ((ptemp=strchr(sid->dat->in_CGIScriptName, '?'))!=NULL) *ptemp='\0';
//	AHHH!!!
//	if (sid->dat->in_CGIScriptName[strlen(sid->dat->in_CGIScriptName)-1]=='/') sid->dat->in_CGIScriptName[strlen(sid->dat->in_CGIScriptName)-1]='\0';
	if (strncmp(sid->dat->in_CGIScriptName, "/cgi-bin/", 9)==0) {
		snprintf(progname, sizeof(progname)-1, "%s", sid->dat->in_CGIScriptName+9);
		snprintf(scriptname, sizeof(scriptname)-1, "%s/%s", proc.config.server_dir_cgi, progname);
	} else if (strncmp(sid->dat->in_CGIScriptName, "/", 1)==0) {
		snprintf(progname, sizeof(progname)-1, "%s", sid->dat->in_CGIScriptName+1);
		snprintf(scriptname, sizeof(scriptname)-1, "%s/%s", proc.config.server_dir_var_htdocs, progname);
	}
	ptemp=scriptname;
 	while (*ptemp) { if (*ptemp=='\\') *ptemp='/'; ptemp++; }
	length=strlen(scriptname);
	do {
		if ((stat(scriptname, &sb)==0)&&(!(sb.st_mode&S_IFDIR))) break;
		if ((ptemp=strrchr(scriptname, '/'))!=NULL) {
			*ptemp='\0';
		} else {
			break;
		}
	} while (strlen(scriptname)>0);
	fixslashes(scriptname);
	length=length-strlen(scriptname);
	if (length>0) {
		snprintf(sid->dat->in_CGIPathInfo, sizeof(sid->dat->in_CGIPathInfo)-1, "%s", progname+(strlen(progname)-length));
		sid->dat->in_CGIScriptName[strlen(sid->dat->in_CGIScriptName)-length]='\0';
	}
//////////////////
	if (strcasecmp(sid->dat->in_Connection, "Keep-Alive")==0) {
		snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Keep-Alive");
	} else {
		if ((strcasecmp(sid->dat->out_Protocol, "HTTP/1.1")==0)&&(strlen(sid->dat->in_Connection)==0)) {
			snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Keep-Alive");
		} else {
			snprintf(sid->dat->out_Connection, sizeof(sid->dat->out_Connection)-1, "Close");
		}
	}
	DEBUG_OUT(sid, "read_header()");
	return 0;
}

void send_error(CONN *sid, int status, char* title, char* text)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	prints(sid, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
	prints(sid, "<HTML>\r\n<HEAD>\r\n");
	prints(sid, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; CHARSET=us-ascii\">\r\n");
	prints(sid, "<TITLE>%d %s</TITLE>\r\n</HEAD>\r\n", status, title);
	prints(sid, "<BODY BGCOLOR=\"#F0F0F0\" TEXT=\"#000000\" LINK=\"#0000FF\" ALINK=\"#0000FF\" VLINK=\"#0000FF\">\r\n");
	prints(sid, "<H1>%d %s</H1>\r\n", status, title);
	prints(sid, "%s\r\n", str2html(sid, text));
	prints(sid, "<HR>\r\n<ADDRESS>%s %s</ADDRESS>\r\n</BODY></HTML>\r\n", SERVER_NAME, SERVER_VERSION);
	sid->dat->out_bodydone=1;
	flushbuffer(sid);
	logaccess(sid, "%s \"%s %s %s\" %d %d \"%s\"", sid->dat->in_RemoteAddr, sid->dat->in_RequestMethod, sid->dat->in_RequestURI, sid->dat->in_Protocol, sid->dat->out_status, sid->dat->out_bytecount, sid->dat->in_UserAgent);
	closeconnect(sid, 1);
	return;
}

void send_header(CONN *sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod)
{
	char timebuf[100];
	time_t now;

	if (status) {
		sid->dat->out_status=status;
	} else {
		sid->dat->out_status=200;
	}
	if (length>=0) {
		sid->dat->out_ContentLength=length;
	}
	if (mod!=(time_t)-1) {
		strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&mod));
		snprintf(sid->dat->out_LastModified, sizeof(sid->dat->out_LastModified)-1, "%s", timebuf);
	}
	now=time(NULL);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	snprintf(sid->dat->out_Date, sizeof(sid->dat->out_Date)-1, "%s", timebuf);
	if (cacheable) {
		snprintf(sid->dat->out_CacheControl, sizeof(sid->dat->out_CacheControl)-1, "public");
		snprintf(sid->dat->out_Pragma, sizeof(sid->dat->out_Pragma)-1, "public");
	} else {
		snprintf(sid->dat->out_CacheControl, sizeof(sid->dat->out_CacheControl)-1, "no-cache; must-revalidate");
		snprintf(sid->dat->out_Expires, sizeof(sid->dat->out_Expires)-1, "%s", timebuf);
		snprintf(sid->dat->out_Pragma, sizeof(sid->dat->out_Pragma)-1, "no-cache");
	}
	if (extra_header!=(char*)0) {
		snprintf(sid->dat->out_ContentType, sizeof(sid->dat->out_ContentType)-1, "%s", mime_type);
	} else {
		snprintf(sid->dat->out_ContentType, sizeof(sid->dat->out_ContentType)-1, "text/html");
	}
}

void send_fileheader(CONN *sid, int cacheable, int status, char *title, char *extra_header, char *mime_type, int length, time_t mod)
{
	send_header(sid, cacheable, status, title, extra_header, mime_type, length, mod);
	flushheader(sid);
}
