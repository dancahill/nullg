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
#include "mod_substub.h"
#include "mod_mail.h"

static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *DecodeRFC2047(CONNECTION *sid, char *src)
{
	char *dest=getbuffer(sid);
	char *pos;
	int destidx;
	int state;
	int szdest;
	int ch=0;

	szdest=2047;
	destidx=0;
	while (*src) {
		if (strncmp(src, "=?", 2)!=0) {
			dest[destidx++]=*src++;
			continue;
		}
		while ((*src)&&(*src!='?')) src++;
		if (*src=='\0') return dest;
		src++;
		// we don't try to make sense of the charset
		while ((*src)&&(*src!='?')) src++;
		if (*src=='\0') return dest;
		src++;
		if (tolower(*src)=='q') {
			while ((*src)&&(*src!='?')) src++;
			if (*src=='\0') return dest;
			src++;
			while ((ch=*src++)!='\0') {
				if (ch=='?') break;
				if (ch=='=') {
					if (isxdigit(src[0])&&isxdigit(src[1])) {
						dest[destidx++]=(char)hex2int(src);
						src+=2;
					}
				} else {
					dest[destidx++]=ch;
				}
				if (destidx>=szdest) return dest;
			}
		} else if (tolower(*src)=='b') {
			while ((*src)&&(*src!='?')) src++;
			if (*src=='\0') return dest;
			src++;
			state=0;
			while ((ch=*src++)!='\0') {
				if (isspace(ch)) continue;
				if (ch=='=') break;
				pos=strchr(Base64, ch);
				if (pos==0) break;
				switch (state) {
				case 0:
					if (dest) {
						if (destidx>=szdest) return dest;
						dest[destidx]=(pos-Base64)<<2;
					}
					state=1;
					break;
				case 1:
					if (dest) {
						if (destidx+1>=szdest) return dest;
						dest[destidx]|=(pos-Base64)>>4;
						dest[destidx+1]=((pos-Base64)&0x0f)<<4;
					}
					destidx++;
					state=2;
					break;
				case 2:
					if (dest) {
						if (destidx+1>=szdest) return dest;
						dest[destidx]|=(pos-Base64)>>2;
						dest[destidx+1]=((pos-Base64)&0x03)<<6;
					}
					destidx++;
					state=3;
					break;
				case 3:
					if (dest) {
						if (destidx>=szdest) return dest;
						dest[destidx]|=(pos-Base64);
					}
					destidx++;
					state=0;
					break;
				}
			}
		}
		if (ch=='=') {
			while ((*src)&&(*src!='?')) src++;
		}
		if (*src=='\0') return dest;
		src++;
		if (*src=='=') src++;
		ch=0;
	}
	return dest;
}

int DecodeHTML(CONNECTION *sid, short int reply, char *src, char *ctype, short int crlf)
{
	char dest[1024];
	char *destidx;

	if (strncasecmp(ctype, "text/html", 9)!=0) return 0;
	memset(dest, 0, sizeof(dest));
	destidx=dest;
	while ((*src)&&(strlen(dest)<sizeof(dest))) {
		if (sid->dat->user_wmcodecstate&2) {
			while ((*src)&&(*src!='-')) src++;
			if ((*src)&&(strncasecmp(src, "-->", 3)==0)) {
				sid->dat->user_wmcodecstate=0;
				src+=3;
			} else {
				sid->dat->user_wmcodecstate=2;
				src++;
				continue;
			}
		} else if (sid->dat->user_wmcodecstate&1) {
			while ((*src)&&(*src!='>')) src++;
			if (*src) {
				sid->dat->user_wmcodecstate=0;
				src++;
			} else {
				sid->dat->user_wmcodecstate=1;
			}
		}
		if (strncasecmp(src, "<!--", 4)==0) {
			sid->dat->user_wmcodecstate=2;
		// KILL THESE EVERYWHERE
		} else if (strncasecmp(src, "<SCRIPT", 7)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<EMBED", 6)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<HEAD", 5)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<HTML", 5)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<IFRAME", 7)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<LINK", 5)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<OBJECT", 7)==0) {
			sid->dat->user_wmcodecstate=1;
		} else if (strncasecmp(src, "<PARAM", 6)==0) {
			sid->dat->user_wmcodecstate=1;
		// SPECIAL CASES FOR HTML REPLY FORMATTING
		} else if ((strncasecmp(src, "<BR>", 4)==0)||(strncasecmp(src, "<P>", 3)==0)||(strncasecmp(src, "<TR", 3)==0)) {
			if (reply) {
				sid->dat->user_wmcodecstate=1;
				strncat(dest, "\r\n> ", sizeof(dest)-strlen(dest)-1);
				destidx+=4;
			} else {
				*destidx++=*src++;
			}
		} else if (strncasecmp(src, "<LI>", 4)==0) {
			if (reply) {
				sid->dat->user_wmcodecstate=1;
				strncat(dest, "\r\n> * ", sizeof(dest)-strlen(dest)-1);
				destidx+=6;
			} else {
				*destidx++=*src++;
			}
		// KILL THESE IN HTML REPLIES BUT PRESERVE IN READS
		} else if ((strncasecmp(src, "<A ", 3)==0)||
				(strncasecmp(src, "<TABLE ", 6)==0)||
				(strncasecmp(src, "<TD", 3)==0)||
				(strncasecmp(src, "<TH", 3)==0)||
				(strncasecmp(src, "<PRE", 4)==0)||
				(strncasecmp(src, "<IMG", 4)==0)||
				(strncasecmp(src, "<FONT", 5)==0)||
				(strncasecmp(src, "<STRONG>", 8)==0)||
				(strncasecmp(src, "<EM>", 4)==0)||
				(strncasecmp(src, "<B>", 3)==0)||
				(strncasecmp(src, "<I>", 3)==0)||
 				(strncasecmp(src, "<U>", 3)==0)||
				(strncasecmp(src, "<CENTER>", 8)==0)) {
			if (reply) {
				sid->dat->user_wmcodecstate=1;
			} else {
				*destidx++=*src++;
			}
		// PRESERVE CLOSING TAGS IN READS
		} else if (strncasecmp(src, "</", 2)==0) {
			if (reply) {
				sid->dat->user_wmcodecstate=1;
			} else {
				*destidx++=*src++;
			}
		// KILL ANYTHING WE MISSED
		} else if (strncasecmp(src, "<", 1)==0) {
			sid->dat->user_wmcodecstate=1;
		} else {
			*destidx++=*src++;
		}
	}
	if (strlen(dest)>0) {
		if (crlf) {
			prints(sid, "\r\n");
			if (reply) {
				prints(sid, "> ");
			}
		}
		prints(sid, "%s", dest);
	}
	return (strlen(dest));
}

int DecodeQP(CONNECTION *sid, short int reply, char *src, char *ctype)
{
	char dest[1024];
	char *destidx;
	short int crlf=1;

	memset(dest, 0, sizeof(dest));
	destidx=dest;
	while ((*src)&&(strlen(dest)<sizeof(dest))) {
		if (*src=='=') {
			src++;
			if (*src==0) {
				crlf=0;
			} else if (isxdigit(src[0])&&isxdigit(src[1])) {
				*destidx++=(char)hex2int(src);
				src+=2;
			}
		} else if ((*src=='\r')||(*src=='\n')) {
			src++;
		} else {
			*destidx++=*src++;
		}
	}
	if (strncasecmp(ctype, "text/html", 9)==0) {
		DecodeHTML(sid, reply, dest, ctype, crlf);
	} else {
		if (reply) prints(sid, "> ");
		prints(sid, "%s", dest);
		if (src[strlen(src)-1]!='=') prints(sid, "\r\n");
	}
	return (strlen(dest));
}

int DecodeText(CONNECTION *sid, short int reply, char *src)
{
	char dest[1024];
	char *destidx;

	memset(dest, 0, sizeof(dest));
	destidx=dest;
	while ((*src)&&(strlen(dest)<sizeof(dest)-1)) {
		if (*src=='\r') {
			*destidx++=*src++;
			if (*src!='\n') *destidx++='\n';
		} else {
			*destidx++=*src++;
		}
	}
	if (reply) prints(sid, "> ");
	prints(sid, "%s\r\n", dest);
	return (strlen(dest));
}

/* IS THIS B64 CODE LICENCED?  SOURCE = http://www.jti.net/brad/base64.htm */
int EncodeBase64(CONNECTION *sid, char *src, int srclen)
{
	unsigned char a, b, c, d, *cp;
	int dst, i, enclen, remlen, linelen;

	cp=src;
	dst=0;
	linelen=0;
	enclen=srclen/3;
	remlen=srclen-3*enclen;
	for (i=0;i<enclen;i++) {
		a=(cp[0]>>2);
	        b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		c|=(cp[2]>>6);
		d=cp[2]&0x3f;
		cp+=3;
		wmprints(sid, "%c%c%c%c", Base64[a], Base64[b], Base64[c], Base64[d]);
		dst+=4;
		linelen+=4;
		if (linelen>=76) {
			wmprints(sid, "\r\n");
			linelen=0;
		}
	}
	if (remlen==0) {
		wmprints(sid, "\r\n");
	} else if (remlen==1) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		wmprints(sid, "%c%c==\r\n", Base64[a], Base64[b]);
		dst+=4;
	} else if (remlen==2) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		wmprints(sid, "%c%c%c=\r\n", Base64[a], Base64[b], Base64[c]);
		dst+=4;
	}
	return dst;
}

int EncodeBase64file(FILE *fp, char *src, int srclen)
{
	unsigned char a, b, c, d, *cp;
	int dst, i, enclen, remlen, linelen;

	cp=src;
	dst=0;
	linelen=0;
	enclen=srclen/3;
	remlen=srclen-3*enclen;
	for (i=0;i<enclen;i++) {
		a=(cp[0]>>2);
	        b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		c|=(cp[2]>>6);
		d=cp[2]&0x3f;
		cp+=3;
		fprintf(fp, "%c%c%c%c", Base64[a], Base64[b], Base64[c], Base64[d]);
		dst+=4;
		linelen+=4;
		if (linelen>=76) {
			fprintf(fp, "\r\n");
			linelen=0;
		}
	}
	if (remlen==0) {
		fprintf(fp, "\r\n");
	} else if (remlen==1) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		fprintf(fp, "%c%c==\r\n", Base64[a], Base64[b]);
		dst+=4;
	} else if (remlen==2) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		fprintf(fp, "%c%c%c=\r\n", Base64[a], Base64[b], Base64[c]);
		dst+=4;
	}
	return dst;
}

int DecodeBase64(CONNECTION *sid, char *src, char *ctype)
{
	char dest[1024];
	int destidx, state, ch;
	int szdest;
	char *pos;

	memset(dest, 0, sizeof(dest));
	szdest=sizeof(dest)-1;
	state=0;
	destidx=0;
	while ((ch=*src++)!='\0') {
		if (isspace(ch)) continue;
		if (ch=='=') break;
		pos=strchr(Base64, ch);
		if (pos==0) return (-1);
		switch (state) {
			case 0:
				if (dest) {
					if (destidx>=szdest) return (-1);
					dest[destidx]=(pos-Base64)<<2;
				}
				state=1;
				break;
			case 1:
				if (dest) {
					if (destidx+1>=szdest) return (-1);
					dest[destidx]|=(pos-Base64)>>4;
					dest[destidx+1]=((pos-Base64)&0x0f)<<4;
				}
				destidx++;
				state=2;
				break;
			case 2:
				if (dest) {
					if (destidx+1>=szdest) return (-1);
					dest[destidx]|=(pos-Base64)>>2;
					dest[destidx+1]=((pos-Base64)&0x03)<<6;
				}
				destidx++;
				state=3;
				break;
			case 3:
				if (dest) {
					if (destidx>=szdest) return (-1);
					dest[destidx]|=(pos-Base64);
				}
				destidx++;
				state=0;
				break;
		}
	}
	if (strncasecmp(ctype, "text/html", 9)==0) {
		DecodeHTML(sid, 0, dest, ctype, 0);
	} else if (strncasecmp(ctype, "text/plain", 10)==0) {
		prints(sid, "%s", dest);
	} else {
		if (config->RunAsCGI) {
			fwrite(dest, sizeof(char), destidx, stdout);
		} else {
			send(sid->socket, dest, destidx, 0);
		}
	}
	return (destidx);
}

char *EncodeBase64string(CONNECTION *sid, char *src)
{
	unsigned char a, b, c, d, *cp;
	char *dest=getbuffer(sid);
	int dst, i, enclen, remlen;
	int srclen=strlen(src);

	cp=src;
	dst=0;
	enclen=srclen/3;
	remlen=srclen-3*enclen;
	for (i=0;i<enclen;i++) {
		a=(cp[0]>>2);
	        b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		c|=(cp[2]>>6);
		d=cp[2]&0x3f;
		cp+=3;
		dest[dst+0]=Base64[a];
		dest[dst+1]=Base64[b];
		dest[dst+2]=Base64[c];
		dest[dst+3]=Base64[d];
		dst+=4;
		if (dst>=1020) return dest;
	}
	if (remlen==1) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		dest[dst+0]=Base64[a];
		dest[dst+1]=Base64[b];
		dest[dst+2]='=';
		dest[dst+3]='=';
	} else if (remlen==2) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		dest[dst+0]=Base64[a];
		dest[dst+1]=Base64[b];
		dest[dst+2]=Base64[c];
		dest[dst+3]='=';
	}
	return dest;
}

char *DecodeBase64string(CONNECTION *sid, char *src)
{
	static char dest[1024];
	int destidx, state, ch;
	int szdest;
	char *pos;

	memset(dest, 0, sizeof(dest));
	szdest=sizeof(dest)-1;
	state=0;
	destidx=0;
	while ((ch=*src++)!='\0') {
		if (isspace(ch)) continue;
		if (ch=='=') break;
		pos=strchr(Base64, ch);
		if (pos==0) return "";
		switch (state) {
			case 0:
				if (dest) {
					if (destidx>=szdest) return dest;
					dest[destidx]=(pos-Base64)<<2;
				}
				state=1;
				break;
			case 1:
				if (dest) {
					if (destidx+1>=szdest) return dest;
					dest[destidx]|=(pos-Base64)>>4;
					dest[destidx+1]=((pos-Base64)&0x0f)<<4;
				}
				destidx++;
				state=2;
				break;
			case 2:
				if (dest) {
					if (destidx+1>=szdest) return dest;
					dest[destidx]|=(pos-Base64)>>2;
					dest[destidx+1]=((pos-Base64)&0x03)<<6;
				}
				destidx++;
				state=3;
				break;
			case 3:
				if (dest) {
					if (destidx>=szdest) return dest;
					dest[destidx]|=(pos-Base64);
				}
				destidx++;
				state=0;
				break;
		}
	}
	return dest;
}
