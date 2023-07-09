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
#include "crypto.h"

#if defined HAVE_OPENSSL

int nspopenssl_register_all(nsp_state *N)
{
	obj_t *tobj;
#ifdef _WIN32
	static WSADATA wsaData;
	if (WSAStartup(0x101, &wsaData)) return -1;
#endif

	tobj = nsp_settable(N, &N->g, "openssl");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "smime_decrypt", (NSP_CFUNC)libnsp_openssl_smime_decrypt);
	nsp_setcfunc(N, tobj, "smime_encrypt", (NSP_CFUNC)libnsp_openssl_smime_encrypt);
	nsp_setcfunc(N, tobj, "smime_sign", (NSP_CFUNC)libnsp_openssl_smime_sign);
	nsp_setcfunc(N, tobj, "smime_verify", (NSP_CFUNC)libnsp_openssl_smime_verify);
	return 0;
}

#endif /* HAVE_OPENSSL */
