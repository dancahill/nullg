/*
    Null Groupware - Copyright (C) 2000-2003 Dan Cahill

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

char *getbuffer(CONNECTION *sid)
{
	sid->dat->lastbuf++;
	if (sid->dat->lastbuf>3) sid->dat->lastbuf=0;
	memset(sid->dat->smallbuf[sid->dat->lastbuf], 0, sizeof(sid->dat->smallbuf[sid->dat->lastbuf]));
	return sid->dat->smallbuf[sid->dat->lastbuf];
}

void decodeurl(unsigned char *pEncoded)
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
				*pDecoded++=(char)hex2int(pEncoded);
				pEncoded+=2;
			}
		} else {
			*pDecoded++=*pEncoded++;
		}
	}
	*pDecoded='\0';
}

void fixslashes(char *string)
{
 	while (*string) {
#ifdef WIN32
 		if (*string=='/') *string='\\';
#else
 		if (*string=='\\') *string='/';
#endif
		string++;
	}
}

int hex2int(char *pChars)
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

void striprn(char *string)
{
	int i;

	while (1) {
		i=strlen(string);
		if (i<1) break;
		if (string[i-1]=='\r') { string[i-1]='\0'; continue; }
		if (string[i-1]=='\n') { string[i-1]='\0'; continue; }
		break;
	};
}

void swapchar(char *string, char oldchar, char newchar)
{
	while (*string) {
		if (*string==oldchar) *string=newchar;
		string++;
	}
}

char *strcasestr(char *src, char *query)
{
	char *pToken;
	char Buffer[8192];
	char Query[64];
	int loop;

	if (strlen(src)==0) return NULL;
	memset(Buffer, 0, sizeof(Buffer));
	strncpy(Buffer, src, sizeof(Buffer)-1);
	strncpy(Query, query, sizeof(Query)-1);
	loop=0;
	while (Buffer[loop]) {
		Buffer[loop]=toupper(Buffer[loop]);
		loop++;
	}
	loop=0;
	while (Query[loop]) {
		Query[loop]=toupper(Query[loop]);
		loop++;
	}
	pToken=strstr(Buffer, Query);
	if (pToken!=NULL) {
		return src+(pToken-(char *)&Buffer);
	}
	return NULL;
}

char *strcatf(char *dest, const char *format, ...)
{
	char catbuffer[1024];
	va_list ap;

	memset(catbuffer, 0, sizeof(catbuffer));
	va_start(ap, format);
	vsnprintf(catbuffer, sizeof(catbuffer)-1, format, ap);
	va_end(ap);
	strcat(dest, catbuffer);
	return dest;
}

int printhex(CONNECTION *sid, const char *format, ...)
{
	char *hex="0123456789ABCDEF";
	unsigned char buffer[1024];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	va_end(ap);
	while (buffer[offset]) {
		if ((buffer[offset]>32)&&(buffer[offset]<128)&&(buffer[offset]!='<')&&(buffer[offset]!='>')) {
			prints(sid, "%c", buffer[offset]);
		} else {
			prints(sid, "%%%c%c", hex[(unsigned int)buffer[offset]/16], hex[(unsigned int)buffer[offset]&15]);
		}
		offset++;
	}
	return 0;
}

int printht(CONNECTION *sid, const char *format, ...)
{
	unsigned char buffer1[1024];
	unsigned char buffer2[1024];
	char *ptemp1;
	char *ptemp2;
	int destleft=sizeof(buffer2)-6;
	va_list ap;

	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));
	ptemp1=buffer1;
	ptemp2=buffer2;
	va_start(ap, format);
	vsnprintf(buffer1, sizeof(buffer1)-1, format, ap);
	va_end(ap);
	while ((*ptemp1)&&(destleft>0)) {
		if (*ptemp1=='<') {
			strcat(ptemp2, "&lt;");
			ptemp2+=4;
			destleft-=4;
		} else if (*ptemp1=='>') {
			strcat(ptemp2, "&gt;");
			ptemp2+=4;
			destleft-=4;
		} else if (*ptemp1=='&') {
			strcat(ptemp2, "&amp;");
			ptemp2+=5;
			destleft-=5;
		} else if (*ptemp1=='"') {
			strcat(ptemp2, "&quot;");
			ptemp2+=6;
			destleft-=6;
		} else {
			*ptemp2=*ptemp1;
			ptemp2++;
			destleft--;
		}
		ptemp1++;
	}
	prints(sid, "%s", buffer2);
	return 0;
}

void printline(CONNECTION *sid, short int reply, char *msgtext)
{
	char *pmsgbody;
	char *ptemp;
	char line[120];
	unsigned int width=78;

	pmsgbody=msgtext;
	if (reply) width=76;
	while (strlen(pmsgbody)>width) {
		memset(line, 0, sizeof(line));
		snprintf(line, width+1, "%s", pmsgbody);
		line[width]='\0';
		if (reply) prints(sid, "> ");
		if (strchr(line, '\r')||strchr(line, '\n')) {
			if ((ptemp=strchr(line, '\r'))!=NULL) *ptemp='\0';
			if ((ptemp=strchr(line, '\n'))!=NULL) *ptemp='\0';
			printht(sid, "%s\r\n", line);
			pmsgbody+=strlen(line);
			if (*pmsgbody=='\r') pmsgbody++;
			if (*pmsgbody=='\n') pmsgbody++;
		} else if ((ptemp=strrchr(line, ' '))!=NULL) {
			*ptemp='\0';
			printht(sid, "%s\r\n", line);
			pmsgbody+=strlen(line)+1;
		} else {
			printht(sid, "%s", line);
			pmsgbody+=strlen(line);
		}
	}
	if (reply) prints(sid, "> ");
	printht(sid, "%s\r\n", pmsgbody);
}

void printline2(CONNECTION *sid, int dowrap, char *msgtext)
{
	unsigned char buffer[1024];
	char lastchar='\0';
	char *ptemp1;
	char *ptemp2;
	int destleft;

	ptemp1=msgtext;
	while (strlen(ptemp1)>0) {
		memset(buffer, 0, sizeof(buffer));
		destleft=sizeof(buffer)-6;
		ptemp2=buffer;
		while ((*ptemp1)&&(destleft>0)) {
			if (*ptemp1=='\n') {
				if (dowrap) {
					strcat(ptemp2, "\n<BR>");
					ptemp2+=5;
					destleft-=5;
				} else {
					strcat(ptemp2, "\n");
					ptemp2+=1;
					destleft-=1;
				}
			} else if (*ptemp1=='<') {
				strcat(ptemp2, "&lt;");
				ptemp2+=4;
				destleft-=4;
			} else if (*ptemp1=='>') {
				strcat(ptemp2, "&gt;");
				ptemp2+=4;
				destleft-=4;
			} else if (*ptemp1=='&') {
				strcat(ptemp2, "&amp;");
				ptemp2+=5;
				destleft-=5;
			} else if (*ptemp1=='"') {
				strcat(ptemp2, "&quot;");
				ptemp2+=6;
				destleft-=6;
			} else if (*ptemp1==' ') {
				if (dowrap) {
					if (!isalnum(ptemp1[1])||!isalnum(lastchar)) {
						strcat(ptemp2, "&nbsp;");
						ptemp2+=6;
						destleft-=6;
					} else {
						*ptemp2=*ptemp1;
						ptemp2++;
						destleft--;
					}
				} else {
					strcat(ptemp2, " ");
					ptemp2+=1;
					destleft-=1;
				}
			} else {
				*ptemp2=*ptemp1;
				ptemp2++;
				destleft--;
			}
			lastchar=*ptemp1;
			ptemp1++;
		}
		prints(sid, "%s", buffer);
	}
	return;
}

char *str2html(CONNECTION *sid, char *instring)
{
	unsigned char *buffer=getbuffer(sid);
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	while ((instring[i])&&(i<sizeof(sid->dat->smallbuf[0])-1)) {
		ch=instring[i];
		if (ch==0) break;
		if ((ch!=10)&&(ch!=13)) {
			if ((ch<32)||(ch>255)) { i++; continue; }
		}
		if (ch=='\"') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='q';
			buffer[bufferlength+2]='u';
			buffer[bufferlength+3]='o';
			buffer[bufferlength+4]='t';
			buffer[bufferlength+5]=';';
			bufferlength+=6;
			i++;
			continue;
		}
		if (ch=='&') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='a';
			buffer[bufferlength+2]='m';
			buffer[bufferlength+3]='p';
			buffer[bufferlength+4]=';';
			bufferlength+=5;
			i++;
			continue;
		}
		if (ch=='<') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='l';
			buffer[bufferlength+2]='t';
			buffer[bufferlength+3]=';';
			bufferlength+=4;
			i++;
			continue;
		}
		if (ch=='>') {
			buffer[bufferlength]='&';
			buffer[bufferlength+1]='g';
			buffer[bufferlength+2]='t';
			buffer[bufferlength+3]=';';
			bufferlength+=4;
			i++;
			continue;
		}
		buffer[bufferlength]=ch;
		bufferlength++;
		i++;
	}
	return buffer;
}

char *str2sql(CONNECTION *sid, char *instring)
{
	char *buffer=getbuffer(sid);
	unsigned char ch;
	int bufferlength=0;
	int i=0;

	while ((instring[i])&&(i<sizeof(sid->dat->smallbuf[0])-1)) {
		ch=instring[i];
		if (ch==0) break;
		if (ch=='\'') {
			buffer[bufferlength]='\'';
			buffer[bufferlength+1]=ch;
			bufferlength+=2;
			i++;
			continue;
		}
		buffer[bufferlength]=ch;
		bufferlength++;
		i++;
	}
	return buffer;
}

char *str2sqlbuf(CONNECTION *sid, char *instring, char *outstring, int outsize)
{
	int srcindex=0;
	int dstindex=0;

	while ((dstindex<outsize)&&(instring[srcindex])) {
		if (instring[srcindex]=='\'') {
			outstring[dstindex]='\'';
			outstring[dstindex+1]=instring[srcindex];
			dstindex+=2;
		} else {
			outstring[dstindex]=instring[srcindex];
			dstindex++;
		}
		srcindex++;
	}
	if (dstindex<outsize) {
		outstring[dstindex]='\0';
	}
	return outstring;
}
