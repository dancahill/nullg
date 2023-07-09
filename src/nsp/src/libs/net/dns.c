/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2023 Dan Cahill

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
#include "nsp/nsplib.h"
#include "net.h"

#ifdef HAVE_DNS

#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <windns.h>
#define snprintf _snprintf

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dnsapi.lib")

#else
#include <stdlib.h>
#include <string.h>
#include <resolv.h>
#include <arpa/nameser.h>
/*
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
*/
typedef union {
	HEADER hdr;
	uchar buf[8192];
} querybuf;
#endif
#define HOSTBUF 512


static int dns_lookup(nsp_state *N, obj_t *tobj, const char *domain)
{
#define __FN__ __FILE__ ":dns_lookup()"
#ifdef _WIN32
	DNS_FREE_TYPE freetype;
	DNS_STATUS status;
	PDNS_RECORD pDnsRecord;
	PDNS_RECORD pDnsCur;
	obj_t *stobj;
	IN_ADDR ipaddr;
	char namebuf[8];
	register int i;

	freetype = DnsFreeRecordListDeep;
	/* status=DnsQuery_A(domain, DNS_TYPE_ANY, DNS_QUERY_STANDARD, NULL, &pDnsRecord, NULL); */
	status = DnsQuery_A(domain, DNS_TYPE_ANY, DNS_QUERY_BYPASS_CACHE, NULL, &pDnsRecord, NULL);
	if (status) {
		if (N->debug) n_warn(N, __FN__, "res_search failed");
		return -1;
	}
	/* Loop through the answer buffer and extract records. */
	i = 0;
	pDnsCur = pDnsRecord;
	while (pDnsCur != NULL) {
		stobj = nsp_settable(N, tobj, n_ntoa(N, namebuf, i, 10, 0));
		switch (pDnsCur->wType) {
		case DNS_TYPE_A: {
			ipaddr.S_un.S_addr = (pDnsCur->Data.A.IpAddress);
			nsp_setstr(N, stobj, "type", "A", -1);
			nsp_setstr(N, stobj, "host", pDnsCur->pName, -1);
			nsp_setstr(N, stobj, "addr", inet_ntoa(ipaddr), -1);
			if (N->debug) n_warn(N, __FN__, "A  [%s]", inet_ntoa(ipaddr));
			break;
		}
		case DNS_TYPE_NS: {
			nsp_setstr(N, stobj, "type", "NS", -1);
			/* is PTR the right struct? */
			nsp_setstr(N, stobj, "host", pDnsCur->Data.PTR.pNameHost, -1);
			if (N->debug) n_warn(N, __FN__, "NS [%s]", pDnsCur->Data.PTR.pNameHost);
			break;
		}
		case DNS_TYPE_SOA: {
			nsp_setstr(N, stobj, "type", "SOA", -1);
			nsp_setstr(N, stobj, "host", pDnsCur->Data.SOA.pNamePrimaryServer, -1);
			nsp_setstr(N, stobj, "mail", pDnsCur->Data.SOA.pNameAdministrator, -1);
			nsp_setnum(N, stobj, "serial", pDnsCur->Data.SOA.dwSerialNo);
			nsp_setnum(N, stobj, "refresh", pDnsCur->Data.SOA.dwRefresh);
			nsp_setnum(N, stobj, "retry", pDnsCur->Data.SOA.dwRetry);
			nsp_setnum(N, stobj, "expire", pDnsCur->Data.SOA.dwExpire);
			nsp_setnum(N, stobj, "minttl", pDnsCur->Data.SOA.dwDefaultTtl);
			break;
		}
		case DNS_TYPE_PTR: {
			nsp_setstr(N, stobj, "type", "PTR", -1);
			nsp_setstr(N, stobj, "host", pDnsCur->Data.PTR.pNameHost, -1);
			if (N->debug) n_warn(N, __FN__, "PTR [%s]", pDnsCur->Data.PTR.pNameHost);
			break;
		}
		case DNS_TYPE_MX: {
			nsp_setstr(N, stobj, "type", "MX", -1);
			nsp_setnum(N, stobj, "pref", pDnsCur->Data.MX.wPreference);
			nsp_setstr(N, stobj, "host", pDnsCur->Data.MX.pNameExchange, -1);
			if (N->debug) n_warn(N, __FN__, "MX [%s]", pDnsCur->Data.MX.pNameExchange);
			break;
		}
		case DNS_TYPE_TEXT: {
			unsigned int i;
			obj_t *cobj;

			nsp_setstr(N, stobj, "type", "TEXT", -1);
			nsp_setstr(N, stobj, "name", pDnsCur->pName, -1);
			cobj = nsp_setstr(N, stobj, "text", NULL, 0);
			for (i = 0; i < pDnsCur->Data.TXT.dwStringCount; i++) {
				if (i > 0) nsp_strcat(N, cobj, "\r\n", -1);
				nsp_strcat(N, cobj, pDnsCur->Data.TXT.pStringArray[i], (unsigned long)strlen(pDnsCur->Data.TXT.pStringArray[i]));
			}
			break;
		}
		default: {
			/* skip data we're too dumb to parse */
			nsp_setnum(N, stobj, "type", pDnsCur->wType);
			/* if (N->debug) */ n_warn(N, __FN__, "xx[%d]", pDnsCur->wType);
			break;
		}
		}
		i++;
		pDnsCur = pDnsCur->pNext;
	}
	DnsRecordListFree(pDnsRecord, freetype);
	return 0;
#else
	char hostbuf[HOSTBUF + 1];
	char abuf[20];
	char namebuf[8];
	querybuf answer;
	HEADER *hp;
	int ancount, qdcount;
	uchar *msg, *eom, *cp;
	//int type, class, ttl, dlen;
	int type, dlen;
	unsigned short pref;
	register int i;
	register int n;
	uchar *e;
	obj_t *stobj;

	/* Query the nameserver to retrieve mx records for the given domain. */
	n = res_search(domain, C_ANY, T_ANY, (uchar *)&answer, sizeof(answer));
	if (n < 0) {
		if (N->debug) n_warn(N, __FN__, "res_search failed");
		return -1;
	}
	if (n < HFIXEDSZ) return -1;
	/* avoid problems after truncation in tcp packets */
	if (n > (int)sizeof(answer)) n = (int)sizeof(answer);
	/* Valid answer received. Skip the query record. */
	hp = (HEADER *)&answer;
	qdcount = ntohs((u_short)hp->qdcount);
	ancount = ntohs((u_short)hp->ancount);
	msg = (uchar *)&answer;
	eom = (uchar *)&answer + n;
	cp = (uchar *)&answer + HFIXEDSZ;
	while (qdcount-- > 0 && cp < eom) {
		n = dn_skipname(cp, eom);
		if (n < 0) return -1;
		cp += n;
		cp += QFIXEDSZ;
	}
	/* Loop through the answer buffer and extract records. */
	i = 0;
	memset(hostbuf, 0, sizeof(hostbuf));
	while (ancount-- > 0 && cp < eom) {
		stobj = nsp_settable(N, tobj, n_ntoa(N, namebuf, i, 10, 0));
		if ((n = dn_expand(msg, eom, cp, hostbuf, HOSTBUF)) < 0) break;
		if (N->debug) n_warn(N, __FN__, "?[%s]", hostbuf);
		cp += n;
		GETSHORT(type, cp);
		//		GETSHORT(class, cp);
		//		GETLONG(ttl, cp);
		GETSHORT(dlen, cp);
		e = cp + dlen;
		switch (type) {
		case T_A: {
			snprintf(abuf, sizeof(abuf), "%d.%d.%d.%d", (int)cp[0], (int)cp[1], (int)cp[2], (int)cp[3]);
			nsp_setstr(N, stobj, "type", "A", -1);
			nsp_setstr(N, stobj, "host", hostbuf, -1);
			nsp_setstr(N, stobj, "addr", abuf, -1);
			if (N->debug) n_warn(N, __FN__, "A  [%s]", abuf);
			cp += dlen;
			break;
		}
		case T_NS: {
			if ((n = dn_expand(msg, eom, cp, hostbuf, HOSTBUF)) < 0) return -1;
			cp += n;
			n = strlen(hostbuf);
			nsp_setstr(N, stobj, "type", "NS", -1);
			nsp_setstr(N, stobj, "host", hostbuf, n);
			if (N->debug) n_warn(N, __FN__, "NS [%s]", hostbuf);
			break;
		}
		case T_SOA: {
			unsigned int n;

			nsp_setstr(N, stobj, "type", "SOA", -1);
			if ((n = dn_expand(msg, eom, cp, hostbuf, HOSTBUF)) < 0) return -1;
			cp += n;
			nsp_setstr(N, stobj, "host", hostbuf, -1);
			if ((n = dn_expand(msg, eom, cp, hostbuf, HOSTBUF)) < 0) return -1;
			cp += n;
			nsp_setstr(N, stobj, "mail", hostbuf, -1);
			GETLONG(n, cp); nsp_setnum(N, stobj, "serial", n);
			GETLONG(n, cp); nsp_setnum(N, stobj, "refresh", n);
			GETLONG(n, cp); nsp_setnum(N, stobj, "retry", n);
			GETLONG(n, cp); nsp_setnum(N, stobj, "expire", n);
			GETLONG(n, cp); nsp_setnum(N, stobj, "minttl", n);
			break;
		}
		case T_PTR: {
			if ((n = dn_expand(msg, eom, cp, hostbuf, HOSTBUF)) < 0) return -1;
			cp += n;
			n = strlen(hostbuf);
			nsp_setstr(N, stobj, "type", "PTR", -1);
			nsp_setstr(N, stobj, "host", hostbuf, n);
			if (N->debug) n_warn(N, __FN__, "PTR [%s]", hostbuf);
			break;
		}
		case T_MX: {
			GETSHORT(pref, cp);
			if ((n = dn_expand(msg, eom, cp, hostbuf, HOSTBUF)) < 0) return -1;
			cp += n;
			n = strlen(hostbuf);
			nsp_setstr(N, stobj, "type", "MX", -1);
			nsp_setnum(N, stobj, "pref", pref);
			nsp_setstr(N, stobj, "host", hostbuf, n);
			if (N->debug) n_warn(N, __FN__, "MX [%s]", hostbuf);
			break;
		}
		default: {
			/* skip data we're too dumb to parse */
			nsp_setnum(N, stobj, "type", type);
			/* if (N->debug) */ n_warn(N, __FN__, "xx[%d]", type);
			cp += dlen;
			break;
		}
		}
		i++;
		if (cp != e) n_warn(N, __FN__, "SOA[%d %d]", cp, e);
		cp = e;
	}
#endif
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_dns_addr2name)
{
#define __FN__ __FILE__ ":libnsp_net_dns_addr2name()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t tobj;
	uchar ip[4];
	char ipname[32];
	char *p;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj1->val->d.str == NULL) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	p = cobj1->val->d.str;
	if (strstr(p, "in-addr.arpa") == 0) {
		nc_memset((char *)&ip, 0, sizeof(ip));
		ip[0] = atoi(p);
		while (nc_isdigit(*p)) p++;
		if (*p == '.') p++;
		ip[1] = atoi(p);
		while (nc_isdigit(*p)) p++;
		if (*p == '.') p++;
		ip[2] = atoi(p);
		while (nc_isdigit(*p)) p++;
		if (*p == '.') p++;
		ip[3] = atoi(p);
		snprintf(ipname, sizeof(ipname) - 1, "%d.%d.%d.%d.in-addr.arpa", ip[3], ip[2], ip[1], ip[0]);
		p = ipname;
	}
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;
	dns_lookup(N, &tobj, p);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

NSP_FUNCTION(libnsp_net_dns_name2addr)
{
#define __FN__ __FILE__ ":libnsp_net_dns_name2addr()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	obj_t tobj;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj1->val->d.str == NULL) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	nc_memset((void *)&tobj, 0, sizeof(obj_t));
	nsp_linkval(N, &tobj, NULL);
	tobj.val->type = NT_TABLE;
	tobj.val->attr |= NST_AUTOSORT;
	dns_lookup(N, &tobj, cobj1->val->d.str);
	nsp_linkval(N, &N->r, &tobj);
	nsp_unlinkval(N, &tobj);
	return 0;
#undef __FN__
}

#endif /* HAVE_DNS */
