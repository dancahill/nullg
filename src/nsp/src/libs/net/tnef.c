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
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif


#include <fcntl.h>


#ifndef O_BINARY
#define O_BINARY 0
#endif

//#define TNEF_SIG 0x789f3e22

#define MAKE_TNEF_VERSION(_mj,_mn)  (((unsigned long)(0x0000FFFF & _mj) << 16) | (unsigned long)(0x0000FFFF & _mn))
#define TNEF_SIGNATURE  ((unsigned long) 0x223E9F78)
#define TNEF_VERSION    ((unsigned long) MAKE_TNEF_VERSION(1,0))














// Property Data Types
#define PtypInteger16         0x0002 // 2 bytes, a 16-bit integer
#define PtypInteger32         0x0003 // 4 bytes, a 32-bit integer
#define PtypFloating32        0x0004 // 4 bytes, a 32-bit floating point number
#define PtypFloating64        0x0005 // 8 bytes, a 64-bit floating point number
#define PtypCurrency          0x0006 // 8 bytes, a 64-bit signed, scaled integer representation of a decimal currency value, with 4 places to the right of the decimal point
#define PtypFloatingTime      0x0007 // 8 bytes, a 64-bit floating point number in which the whole number part represents the number of days since December 30, 1899, and the fractional part represents the fraction of a day since midnight
#define PtypErrorCode         0x000A // 4 bytes, a 32-bit integer encoding error information
#define PtypBoolean           0x000B // 1 byte, restricted to 1 or 0
#define PtypInteger64         0x0014 // 8 bytes, a 64-bit integer
#define PtypString            0x001F // Variable size, a string of Unicode characters in UTF-16LE encoding with terminating null character (2 bytes of 0)
#define PtypString8           0x001E // Variable size, a string of multi-byte characters in externally specified encoding with terminating null character (single 0 byte)
#define PtypTime              0x0040 // 8 bytes, a 64-bit integer representing the number of 100-nanosecond intervals since January 1, 1601
#define PtypGuid              0x0048 // 16 bytes, a GUID with Data1, Data2, and Data3 fields in little-endian format
#define PtypBinary            0x0102 // Variable size, a COUNT followed by that many bytes
#define PtypMultipleInteger32 0x1003 // Variable size, a COUNT followed by that many PtypInteger32 values
#define PtypMultipleString    0x101F // Variable size, a COUNT followed by that many PtypString values
#define PtypMultipleString8   0x101E // Variable size, a COUNT followed by that many PtypString8 values
#define PtypMultipleTime      0x1040 // Variable size, a COUNT followed by that many PtypTime values
#define PtypMultipleGuid      0x1048 // Variable size, a COUNT followed by that many PtypGuid values
#define PtypMultipleBinary    0x1102 // Variable size, a COUNT followed by that many PtypBinary values
#define PtypObject            0x000D
#define PtypEmbeddedTable     0x000D // The property value is a COM object















#define PidTagCreationTime        0x3007
#define PidTagAttachExtension     0x3703
#define PidTagAttachMethod        0x3705
#define PidTagAttachLongFilename  0x3707
#define PidTagRenderingPosition   0x370B
#define PidTagAttachTransportName 0x370C
#define PidTagAttachMimeTag       0x370E


NSP_FUNCTION(libnsp_net_tnef_debug)
{
#define __FN__ __FILE__ ":libnsp_net_tnef_debug()"
	obj_t *cobj1 = nsp_getobj(N, &N->context->l, "1");
	unsigned long int i, bl;
	unsigned char *p;
	//	unsigned char iobuf[1024];
	//	int fd;
	//	int r;
	unsigned long len;
	unsigned short lvl;
	unsigned long attr, attr_type, attr_id;

	if ((cobj1->val->type != NT_STRING) || (cobj1->val->size < 1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");

	p = (unsigned char *)cobj1->val->d.str;
	i = 0;
	bl = cobj1->val->size - i;


	/* TNEF signature */
	if (bl >= 4) {
		n_warn(N, __FN__, "0x%x 0x%x", TNEF_SIGNATURE, readi4((&p[i])));
		i += 4; bl -= 4;
	}
	/* key */
	if (bl >= 2) {
		n_warn(N, __FN__, "0x%x", readi2((&p[i])));
		i += 2; bl -= 2;
	}
	for (; bl > 0;) {

		/* ? */
		if (bl < 1) {
			n_warn(N, __FN__, "missing data");
			goto err;
		}
		lvl = p[i];
		//		n_warn(N, __FN__, "lvl = %d", lvl);
		i += 1; bl -= 1;

		/* Attributes */
		if (bl < 4) {
			n_warn(N, __FN__, "missing data");
			goto err;
		}
		attr = readi4((&p[i]));
		attr_id = attr & 0xFFFF;
		attr_type = attr >> 16 & 0xFFFF;
		//		n_warn(N, __FN__, "Attributes = 0x%x", attr);
		//		n_warn(N, __FN__, "Attributes = 0x%x 0x%x", attr_id, attr_type);
		i += 4; bl -= 4;

		/* Length */
		if (bl < 4) {
			n_warn(N, __FN__, "length error 1");
			goto err;
		}
		len = readi4((&p[i]));
		//		n_warn(N, __FN__, "Length = %d", len);
		i += 4; bl -= 4;
		if (bl < len) {
			n_warn(N, __FN__, "length error 2");
			goto err;
		}


		if (attr_id == 0x800F && attr_type == 0x6) {
			//		if (attr_id==0x9005 && attr_type==0x6) {
			nsp_setstr(N, &N->r, "", (char *)&p[i], len);
		}

		if (attr_id == 0x9005 && attr_type == 0x6) {
			int pcount;
			int j;
			unsigned long pattr, pattr_type, pattr_id;
			unsigned long plen;

			/* ... */
			if (bl < 4) {
				n_warn(N, __FN__, "missing data");
				goto err;
			}
			pcount = readi4((&p[i]));
			i += 4; bl -= 4;
			n_warn(N, __FN__, "pcount = %d", pcount);
			/* Attributes */
			if (bl < 4) {
				n_warn(N, __FN__, "missing data");
				goto err;
			}
			pattr = readi4((&p[i]));
			pattr_type = pattr & 0xFFFF;
			pattr_id = pattr >> 16 & 0xFFFF;
			n_warn(N, __FN__, "Attributes = 0x%x 0x%x", pattr_id, pattr_type);
			i += 4; bl -= 4;
			/* ... */
			if (bl < 4) {
				n_warn(N, __FN__, "missing data");
				goto err;
			}
			n_warn(N, __FN__, "... = %d", readi4((&p[i])));
			i += 4; bl -= 4;
			for (j = 0; j < pcount; j++) {
				/* Attributes */
				if (bl < 4) {
					n_warn(N, __FN__, "missing data");
					goto err;
				}
				pattr = readi4((&p[i]));
				pattr_type = pattr & 0xFFFF;
				pattr_id = pattr >> 16 & 0xFFFF;
				n_warn(N, __FN__, "p Attributes = 0x%x 0x%x", pattr_id, pattr_type);
				i += 4; bl -= 4;
				/* ... */
				if (bl < 4) {
					n_warn(N, __FN__, "missing data");
					goto err;
				}
				n_warn(N, __FN__, "... = %d", readi4((&p[i])));
				i += 4; bl -= 4;
				if (pattr_type == 0x1F) {
					if (pattr_id == PidTagAttachExtension || pattr_id == PidTagAttachLongFilename || pattr_id == PidTagAttachMimeTag) {
						// * Length * /
						n_warn(N, __FN__, "check");
						if (bl < 4) {
							n_warn(N, __FN__, "length error 1");
							goto err;
						}
						plen = readi4((&p[i]));
						n_warn(N, __FN__, "Length = %d", plen);
						i += 4; bl -= 4;
						if (bl < plen) {
							n_warn(N, __FN__, "length error 2");
							goto err;
						}
						i += plen;
						bl -= plen;
						/* checksum/padding? */
						// if (fmod(plen, 4)!=0) {
						if (plen % 4 != 0) {
							if (bl < 2) {
								n_warn(N, __FN__, "missing data");
								goto err;
							}
							n_warn(N, __FN__, "padding = 0x%x", readi2((&p[i])));
							i += 2; bl -= 2;
						}
					} else {
						n_warn(N, __FN__, "unknown attr %x with type %x", pattr_id, pattr_type);
					}
				} else if (pattr_type == 0x40) {
					n_warn(N, __FN__, "PtypTime skipped");
					i += 16; bl -= 16;
				} else if (pattr_type == 0x3) {
					n_warn(N, __FN__, "PtypInteger32 unhandled type");
				} else if (pattr_type == 0x001E) {
					// * Length * /
					n_warn(N, __FN__, "check");
					if (bl < 4) {
						n_warn(N, __FN__, "length error 1");
						goto err;
					}
					plen = readi4((&p[i]));
					n_warn(N, __FN__, "Length = %d", plen);
					i += 4; bl -= 4;
					if (bl < plen) {
						n_warn(N, __FN__, "length error 2");
						goto err;
					}
					n_warn(N, __FN__, "PtypString8 [%s]", &p[i]);
					i += plen;
					bl -= plen;
					// should this be padded?
					i += 3;
					bl -= 3;
					break;
				} else {
					break;
				}
			}
//			for (;bl>0;) {
//				n_warn(N, __FN__, "[%c][%d][%x]", p[i], p[i], p[i]);
//				i++; bl--;
//			}
		} else {
			i += len; bl -= len;
		}
		/* checksum */
		if (bl < 2) {
			n_warn(N, __FN__, "missing data");
			goto err;
		}
//		n_warn(N, __FN__, "Checksum = 0x%x", readi2((&p[i])));
		i += 2; bl -= 2;
//		n_warn(N, __FN__, "i= %d, bl = %d", i, bl);
		n_warn(N, __FN__, "[lvl=%d][Attributes = 0x%x 0x%x][Length = %d]", lvl, attr_id, attr_type, len);
	}
err:
	n_warn(N, __FN__, "done");
	return 0;
#undef __FN__
}

/*
NSP_FUNCTION(libnsp_net_tnef_debug)
{
#define __FN__ __FILE__ ":libnsp_net_tnef_debug()"
	obj_t *cobj1=nsp_getobj(N, &N->l, "1");
	unsigned char iobuf[1024];
	int fd;
	int r;

	if ((cobj1->val->type!=NT_STRING)||(cobj1->val->size<1)) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");



	if ((fd=open(cobj1->val->d.str, O_RDONLY|O_BINARY))==-1) {
		n_warn(N, __FN__, "could not open file");
		return -1;
	}
	do {
//		r=read(fd, iobuf, sizeof(iobuf)-1);
		r=read(fd, iobuf, 1);
		n_warn(N, __FN__, "[%d][%x]", (unsigned int)iobuf[0], iobuf[0]);
		if (r<1) break;

	} while (r>0);
	close(fd);




	return 0;
#undef __FN__
}
*/
