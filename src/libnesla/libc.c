/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2008 Dan Cahill

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
#include "nesla/libnesla.h"
#include "opcodes.h"
#include <math.h>
#include <stdarg.h>
#if defined(_MSC_VER)||defined(__TURBOC__)
#include <time.h>
#include <sys/timeb.h>
#endif

#define __USE_ISOC99
#include <limits.h>

/* *printf() */
static const char nchars[]="0123456789ABCDEF";

int nc_vsnprintf(nes_state *N, char *dest, int max, const char *format, va_list ap)
{
#define __FUNCTION__ __FILE__ ":nc_vsnprintf()"
	char tmp[80];
	char *d, *s, *p;
	char esc=0;
	int len=0;

	settrace();
	for (s=(char *)format,d=dest;*s!='\0';s++) {
		if (!esc) {
			if (*s=='%') esc=1; else { *d++=*s; len++; }
			continue;
		}
		/* flags */
		if (*s=='-') s++;
		/* width */
		if (nc_isdigit(*s)) { while (nc_isdigit(*s)) s++; }
		/* precision */
		if (*s=='.') {
			s++;
			if (nc_isdigit(*s)) { while (nc_isdigit(*s)) s++; }
		}
		switch (*s) {
		case 'c': tmp[0]=(char)va_arg(ap, int); tmp[1]=0; goto end;
		case 'd': p=n_ntoa(N, tmp, va_arg(ap, int),          -10, 0); break;
		case 'i': p=n_ntoa(N, tmp, va_arg(ap, int),          -10, 0); break;
		case 'o': p=n_ntoa(N, tmp, va_arg(ap, int),            8, 0); break;
		case 'u': p=n_ntoa(N, tmp, va_arg(ap, unsigned int),  10, 0); break;
		case 'x': p=n_ntoa(N, tmp, va_arg(ap, unsigned int),  16, 0); break;
		case 'f': p=n_ntoa(N, tmp, va_arg(ap, double),       -10, 6); break;
		case 's': p=va_arg(ap, char *); if (p==NULL) p="(null)"; break;
		default : p="orphan %"; break;
		}
		nc_strncpy(d, p, max-len); len+=nc_strlen(p); d=dest+len;
end:
		esc=0;
	}
	*d='\0';
	return len;
#undef __FUNCTION__
}

int nc_snprintf(nes_state *N, char *str, int size, const char *format, ...)
{
#define __FUNCTION__ __FILE__ ":nc_snprintf()"
	va_list ap;
	int len;

	settrace();
	va_start(ap, format);
	len=nc_vsnprintf(N, str, size, format, ap);
	va_end(ap);
	return len;
#undef __FUNCTION__
}

int nc_printf(nes_state *N, const char *format, ...)
{
#define __FUNCTION__ __FILE__ ":nc_printf()"
	va_list ap;
	int len;

	settrace();
	if (N->outbuflen>OUTBUFLOWAT) nl_flush(N);
	va_start(ap, format);
	len=nc_vsnprintf(N, N->outbuf+N->outbuflen, MAX_OUTBUFLEN-N->outbuflen, format, ap);
	N->outbuflen+=len;
	va_end(ap);
	return len;
#undef __FUNCTION__
}

/* time stuff */
int nc_gettimeofday(struct timeval *tv, void *tz)
{
#if defined(_MSC_VER)||defined(__BORLANDC__)||defined(__TURBOC__)
	struct timeb tb;

	if (tv==NULL) return -1;
	ftime(&tb);
	tv->tv_sec=tb.time;
	tv->tv_usec=tb.millitm*1000;
#else
	gettimeofday(tv, tz);
#endif
	return 0;
}

char *nc_memcpy(char *dst, const char *src, int n)
{
	uchar *s=(uchar *)src, *d=(uchar *)dst;

	if (src==NULL) return dst;
	while (n-->0) *d++=*s++;
	return dst;
}

int nc_strlen(const char *s)
{
	char *p=(char *)s;

	while (*p) p++;
	return p-(char *)s;
}

char *nc_strchr(const char *s, int c)
{
	uchar *a=(uchar *)s;

	do {
		if (*a==c) return (char *)a;
	} while (*a++);
	return NULL;
}

char *nc_strncpy(char *dst, const char *src, int n)
{
	uchar *s=(uchar *)src, *d=(uchar *)dst;

	if (n>0) {
		if (src!=NULL) {
			while (*s!=0&&--n>0) *d++=*s++;
		}
		*d=0;
	}
	return dst;
}

int nc_strcmp(const char *s1, const char *s2)
{
	uchar *a, *b;

	if (s1==s2) return 0;
	else if (!s1) return -(uchar)*s2;
	else if (!s2) return +(uchar)*s1;
	a=(uchar *)s1;
	b=(uchar *)s2;
	while (*a==*b&&*a!='\0') {
		a++; b++;
	}
	return *a-*b;
}

int nc_strncmp(const char *s1, const char *s2, int n)
{
	uchar *a=(uchar *)s1, *b=(uchar *)s2;

	if (a==b) return 0;
	do {
		if (*a!=*b) return *a-*b;
		if (*a==0) return 0;
		a++; b++;
	} while (--n>0);
	return 0;
}

void *nc_memset(void *s, int c, int n)
{
	uchar *a=s;

	while (n) a[--n]=(uchar)c;
	return s;
}

void n_error(nes_state *N, short int err, const char *fname, const char *format, ...)
{
#define __FUNCTION__ __FILE__ ":n_error()"
	va_list ap;
	int len;

	settrace();
	N->err=err;
	if (N->err) {
		len=nc_snprintf(N, N->errbuf, sizeof(N->errbuf)-1, "%-15s : ", fname);
		va_start(ap, format);
		len+=nc_vsnprintf(N, N->errbuf+len, sizeof(N->errbuf)-len-1, format, ap);
		va_end(ap);
	}
	nl_flush(N);
	if (N->savjmp!=NULL) {
		longjmp(*N->savjmp, 1);
	} else {
		n_warn(N, __FUNCTION__, "jmp ptr not set - errno=%d :: \r\n%s", N->err, N->errbuf);
	}
	return;
#undef __FUNCTION__
}

void n_expect(nes_state *N, const char *fname, uchar op)
{
#define __FUNCTION__ __FILE__ ":n_expect()"
	settrace();
	if (*N->readptr!=op) n_error(N, NE_SYNTAX, fname, "expected a '%s'", n_getsym(N, op));
#undef __FUNCTION__
}

void n_warn(nes_state *N, const char *fname, const char *format, ...)
{
#define __FUNCTION__ __FILE__ ":n_warn()"
	va_list ap;

	settrace();
	if (++N->warnings>10000) n_error(N, NE_SYNTAX, __FUNCTION__, "too many warnings (%d)\n", N->warnings);
	if (N->outbuflen>OUTBUFLOWAT) nl_flush(N);
#if defined(__GNUC__)
	nc_printf(N, "\r\n[01;33;40m%s : ", fname);
#else
	nc_printf(N, "\r\n%s : ", fname);
#endif
	va_start(ap, format);
	N->outbuflen+=nc_vsnprintf(N, N->outbuf+N->outbuflen, MAX_OUTBUFLEN-N->outbuflen, format, ap);
	va_end(ap);
#if defined(__GNUC__)
	nc_printf(N, "\r\n[00m");
#else
	nc_printf(N, "\r\n");
#endif
	nl_flush(N);
	if ((N->strict)&&(N->savjmp!=NULL)) {
		longjmp(*N->savjmp, 1);
	}
	return;
#undef __FUNCTION__
}


/*
 * the following functions are public API functions
 */
num_t n_aton(nes_state *N, const char *str)
{
#define __FUNCTION__ __FILE__ ":n_aton()"
	char *s=(char *)str;
	num_t rval=0;
	num_t rdot=0.1;

	settrace();
	while (nc_isdigit(*s)) {
		rval=10*rval+(*s++-'0');
	}
	if (*s!='.') return rval;
	s++;
	while (nc_isdigit(*s)) {
		rval+=(*s++-'0')*rdot;
		rdot*=0.1;
	}
	return rval;
#undef __FUNCTION__
}

char *n_ntoa(nes_state *N, char *str, num_t num, short base, unsigned short dec)
{
#define __FUNCTION__ __FILE__ ":n_ntoa()"
/*
#  define LLONG_MAX	9223372036854775807LL
#  define ULONG_MAX     0xffffffffUL  / * maximum unsigned long value * /

#  define LONG_MAX	2147483647L
#  define LONG_MIN	(-LONG_MAX - 1L)
*/

#if defined (LLONG_MAX)
	long long int n=(long long int)num;
	num_t f=(num_t)num-(long long int)num;
#elif defined (_MSC_VER)&&(_MSC_VER<=1400)
	__int64 n=(__int64)num;
	num_t f=(num_t)num-(__int64)num;
#else
	/* long long int causes fires - need working substitute */
	long int n=(long int)num;
	num_t f=(num_t)num-(long int)num;
#endif
	unsigned int i;
	char c, sign='+';
	char *p, *q;

	settrace();
#ifdef LLONG_MAX
#elif defined (_MSC_VER)&&(_MSC_VER<=1400)
#else
	if (num>ULONG_MAX) n_warn(N, __FUNCTION__, "broken number display...");
#endif
/*
	if (isinf(f)) {
		nc_strncpy(str, "inf", 4);
		return str;
	} else if (isnan(f)) {
		nc_strncpy(str, "nan", 4);
		return str;
	}
*/
	if (base<0) {
		if (num<0) {
			sign='-';
			n*=-1;
			f*=-1;
		}
		base=-base;
	}
	p=q=str;
	/* need a _real_ fix */
	if (n%base<0) {
		nc_strncpy(str, "nan", 4);
		return str;
	}
	do {
		*p++=nchars[n%base];
	} while ((n/=base)>0);
	if (sign=='-') *p++='-';
	*p='\0';
	/* and flip */
	while (q<--p) {
		c=*q;
		*q++=*p;
		*p=c;
	}
	if (dec) {
		while (*p) p++;
		*p++='.';
		for (i=0;i<dec;i++) {
			f*=base;
			*p++=nchars[(int)f%base];
		}
		*p='\0';
		/* insert rounding stuff around here.. */

		/* trim the zeros */
		i=nc_strlen(str)-1;
		while (str[i]=='0') str[i--]='\0';
		if (str[i]=='.') str[i--]='\0';
	}
	return str;
#undef __FUNCTION__
}
