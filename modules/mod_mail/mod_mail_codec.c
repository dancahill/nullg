/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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
#include "http_mod.h"
#include "mod_mail.h"

static const char Base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *DecodeRFC2047(CONN *sid, char *src)
{
	char *dest=getbuffer(sid);
	char *pos;
	int destidx;
	int state;
	int szdest;
	int ch=0;

	szdest=sizeof(sid->dat->smallbuf[0])-1;
	destidx=0;
	while (*src) {
		if (strncmp(src, "=?", 2)!=0) {
			dest[destidx++]=*src++;
			continue;
		}
		while ((*src)&&(*src!='?')) src++;
		if (*src=='\0') goto done;
		src++;
		// we don't try to make sense of the charset
		while ((*src)&&(*src!='?')) src++;
		if (*src=='\0') goto done;
		src++;
		if (tolower(*src)=='q') {
			while ((*src)&&(*src!='?')) src++;
			if (*src=='\0') goto done;
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
				if (destidx>=szdest) goto done;
			}
		} else if (tolower(*src)=='b') {
			while ((*src)&&(*src!='?')) src++;
			if (*src=='\0') goto done;
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
						if (destidx>=szdest) goto done;
						dest[destidx]=(pos-Base64)<<2;
					}
					state=1;
					break;
				case 1:
					if (dest) {
						if (destidx+1>=szdest) goto done;
						dest[destidx]|=(pos-Base64)>>4;
						dest[destidx+1]=((pos-Base64)&0x0f)<<4;
					}
					destidx++;
					state=2;
					break;
				case 2:
					if (dest) {
						if (destidx+1>=szdest) goto done;
						dest[destidx]|=(pos-Base64)>>2;
						dest[destidx+1]=((pos-Base64)&0x03)<<6;
					}
					destidx++;
					state=3;
					break;
				case 3:
					if (dest) {
						if (destidx>=szdest) goto done;
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
		if (*src=='\0') goto done;
		src++;
		if (*src=='=') src++;
		ch=0;
	}
done:
	dest[destidx]='\0';
	dest[sizeof(sid->dat->smallbuf[0])-1]='\0';
	return dest;
}

int DecodeHTML(CONN *sid, char *dest, unsigned int szdest, char *src, short int reply)
{
	char *destidx;

	destidx=dest;
	memset(dest, 0, szdest);
	while ((*src)&&(strlen(dest)<szdest)) {
		if (sid->dat->wm->codecstate==2) {
			while ((*src)&&(*src!='-')) src++;
			if ((*src)&&(strncasecmp(src, "-->", 3)==0)) {
				sid->dat->wm->codecstate=0;
				src+=3;
			} else {
				sid->dat->wm->codecstate=2;
				src++;
				continue;
			}
		} else if (sid->dat->wm->codecstate==1) {
			while ((*src)&&(*src!='>')) src++;
			if (*src) {
				sid->dat->wm->codecstate=0;
				src++;
			} else {
				sid->dat->wm->codecstate=1;
			}
		}
		if (strncasecmp(src, "<!--", 4)==0) {
			sid->dat->wm->codecstate=2;
		// KILL THESE EVERYWHERE
		} else if (strncasecmp(src, "<SCRIPT", 7)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<EMBED", 6)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<HEAD", 5)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<HTML", 5)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<IFRAME", 7)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<LINK", 5)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<OBJECT", 7)==0) {
			sid->dat->wm->codecstate=1;
		} else if (strncasecmp(src, "<PARAM", 6)==0) {
			sid->dat->wm->codecstate=1;
		// SPECIAL CASES FOR HTML REPLY FORMATTING
		} else if ((strncasecmp(src, "<BR>", 4)==0)||(strncasecmp(src, "<P>", 3)==0)||(strncasecmp(src, "<TR", 3)==0)) {
			if (reply) {
				sid->dat->wm->codecstate=1;
				strncat(dest, "\r\n", sizeof(dest)-strlen(dest)-1);
				destidx+=2;
			} else {
				*destidx++=*src++;
			}
		} else if (strncasecmp(src, "<LI>", 4)==0) {
			if (reply) {
				sid->dat->wm->codecstate=1;
//				strncat(dest, "\r\n> * ", sizeof(dest)-strlen(dest)-1);
				strncat(dest, "\r\n* ", sizeof(dest)-strlen(dest)-1);
				destidx+=4;
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
				(strncasecmp(src, "<STYLE", 6)==0)||
				(strncasecmp(src, "<CENTER>", 8)==0)) {
			if (reply) {
				sid->dat->wm->codecstate=1;
			} else {
				*destidx++=*src++;
			}
		// PRESERVE CLOSING TAGS IN READS
		} else if (strncasecmp(src, "</", 2)==0) {
			if (reply) {
				sid->dat->wm->codecstate=1;
			} else {
				*destidx++=*src++;
			}
		// KILL ANYTHING WE MISSED
		} else if (strncasecmp(src, "<", 1)==0) {
			sid->dat->wm->codecstate=1;
		} else {
			*destidx++=*src++;
		}
	}
	return (strlen(dest));
}

int DecodeQP(CONN *sid, char *dest, unsigned int szdest, char *src)
{
	char *destidx;
	short int crlf=1;

	destidx=dest;
	memset(dest, 0, szdest);
	while ((*src)&&(strlen(dest)<szdest)) {
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
	return (strlen(dest));
}

int DecodeText(CONN *sid, char *dest, unsigned int szdest, char *src)
{
	char *destidx;

	memset(dest, 0, szdest);
	destidx=dest;
	while ((*src)&&(strlen(dest)<szdest-1)) {
		if (*src=='\r') {
			*destidx++=*src++;
			if (*src!='\n') *destidx++='\n';
		} else {
			*destidx++=*src++;
		}
	}
	*destidx++='\r';
	*destidx++='\n';
	return (strlen(dest));
}

static int decode_b64(CONN *sid, char *dest, int szdest, char *src)
{
	int destidx, state, ch;
	char *pos;

	memset(dest, 0, szdest);
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
	return (destidx);
}

int DecodeBase64file(CONN *sid, char *src)
{
	char *dest=getbuffer(sid);
	int destsize;

	destsize=decode_b64(sid, dest, sizeof(sid->dat->smallbuf[0])-1, src);
	flushbuffer(sid);
	if (http_proc->RunAsCGI) {
		fwrite(dest, sizeof(char), destsize, stdout);
	} else {
		if (tcp_send(&sid->socket, dest, destsize, 0)<0) return -1;
	}
	sid->dat->out_bytecount+=destsize;
	return destsize;
}

char *DecodeBase64string(CONN *sid, char *src)
{
	char *dest=getbuffer(sid);

	decode_b64(sid, dest, sizeof(sid->dat->smallbuf[0])-1, src);
	return dest;
}

int EncodeBase64(CONN *sid, char *src, int srclen)
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

char *EncodeBase64string(CONN *sid, char *src)
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
		if (dst>=1020) goto done;
	}
	if (remlen==1) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		dest[dst+0]=Base64[a];
		dest[dst+1]=Base64[b];
		dest[dst+2]='=';
		dest[dst+3]='=';
		dst+=4;
	} else if (remlen==2) {
		a=(cp[0]>>2);
		b=(cp[0]<<4)&0x30;
		b|=(cp[1]>>4);
		c=(cp[1]<<2)&0x3c;
		dest[dst+0]=Base64[a];
		dest[dst+1]=Base64[b];
		dest[dst+2]=Base64[c];
		dest[dst+3]='=';
		dst+=4;
	}
done:
	dest[dst]='\0';
	dest[sizeof(sid->dat->smallbuf[0])-1]='\0';
	return dest;
}
