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
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <windns.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "dnsapi.lib")
#define BUFFER_LEN 255
#define MAXBUF 512
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <resolv.h>
#include <arpa/nameser.h>
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#include <arpa/nameser_compat.h>
#endif
#define MAXPACKET	8192	/* max size of packet */
#define MAXMXHOSTS	20	/* max num of mx records we want to see */
#define MAXBUF		512
enum { MAXMXBUFSIZ=(MAXMXHOSTS*(MAXBUF+1)) };
typedef union {
	HEADER hdr;
	u_char buf[MAXPACKET];
} querybuf;
#endif

char *dns_getmxbyname(char *dest, int destlen, char *domain)
{
#ifdef WIN32
	DNS_FREE_TYPE freetype;
	DNS_STATUS status;
	PDNS_RECORD pDnsRecord;

	freetype=DnsFreeRecordListDeep;
	status=DnsQuery_A(domain, DNS_TYPE_MX, DNS_QUERY_BYPASS_CACHE, NULL, &pDnsRecord, NULL);
	if (status) {
		dest[0]='\0';
	} else {
		_snprintf(dest, destlen, "%s", (pDnsRecord->Data.MX.pNameExchange));
		DnsRecordListFree(pDnsRecord, freetype);
	}
#else
	char hostbuf[MAXMXBUFSIZ];
	char *MxHosts[MAXMXHOSTS];
	querybuf answer;		/* answer buffer from nameserver */
	HEADER *hp;			/* answer buffer header */
	int ancount, qdcount;		/* answer count and query count */
	u_char *msg, *eom, *cp;		/* answer buffer positions */
	int type, class, ttl, dlen;	/* record type, class, ttl and length */
	u_short pref;			/* mx preference value */
	u_short prefer[MAXMXHOSTS];	/* saved preferences of mx records */
	u_char *bp;			/* hostbuf pointer */
	int nmx;			/* number of mx hosts found */
	register int i;
	register int j;
	register int n;

	dest[0]='\0';
	// Query the nameserver to retrieve mx records for the given domain.
	n=res_search(domain, C_IN, T_MX, (u_char *)&answer, sizeof(answer));
	if (n<0) {
		if (_res.options&RES_DEBUG) printf("sres_search failed\n");
		return(0);
	}
	if (n<HFIXEDSZ) return(0);
	/* avoid problems after truncation in tcp packets */
	if (n>(int)sizeof(answer)) n=(int)sizeof(answer);
	// Valid answer received. Skip the query record.
	hp=(HEADER *)&answer;
	qdcount=ntohs((u_short)hp->qdcount);
	ancount=ntohs((u_short)hp->ancount);
	msg=(u_char *)&answer;
	eom=(u_char *)&answer+n;
	cp=(u_char *)&answer+HFIXEDSZ;
	while (qdcount-- > 0 && cp < eom) {
		n = dn_skipname(cp, eom);
		if (n < 0) return(0);
		cp += n;
		cp += QFIXEDSZ;
	}
	// Loop through the answer buffer and extract mx records.
	nmx = 0;
	bp = hostbuf;
	while (ancount-- > 0 && cp < eom && nmx < MAXMXHOSTS) {
		n=dn_expand(msg, eom, cp, (u_char *)bp, MAXBUF);
		if (n < 0) break;
		cp += n;
		GETSHORT(type, cp);
		//cp += INT16SZ;
		GETSHORT(class, cp);
		//cp += INT16SZ;
		GETLONG(ttl, cp);
		//cp += INT32SZ;
		GETSHORT(dlen, cp);
		//cp += INT16SZ;
		if (type != T_MX || class != C_IN) {
			cp += dlen;
			continue;
		}
		GETSHORT(pref, cp);
		//cp += INT16SZ;
		n = dn_expand(msg, eom, cp, (u_char *)bp, MAXBUF);
		if (n < 0) break;
		cp += n;
		prefer[nmx] = pref;
		MxHosts[nmx] = bp;
		nmx++;
		n = strlen(bp) + 1;
		bp += n;
	}
	if (nmx<1) return NULL;
	// Sort all records by preference.
	for (i=0;i<nmx;i++) {
		for (j=i+1;j<nmx;j++) {
			if (prefer[i]>prefer[j]) {
				register u_short tmppref;
				register char *tmphost;

				tmppref = prefer[i];
				prefer[i] = prefer[j];
				prefer[j] = tmppref;
				tmphost = MxHosts[i];
				MxHosts[i] = MxHosts[j];
				MxHosts[j] = tmphost;
			}
		}
	}
	snprintf(dest, destlen, "%s", MxHosts[0]);
#endif
	return (dest);
}
