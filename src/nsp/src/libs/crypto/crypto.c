/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2019 Dan Cahill

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
int nspopenssl_register_all(nsp_state *N);
#endif

int nspcrypto_register_all(nsp_state *N)
{
	obj_t *tobj;

	tobj = nsp_settable(N, &N->g, "crypto");
	tobj->val->attr |= NST_HIDDEN;
	nsp_setcfunc(N, tobj, "aes_cbc_encrypt", (NSP_CFUNC)libnsp_crypto_aes_encrypt);
	nsp_setcfunc(N, tobj, "aes_cbc_decrypt", (NSP_CFUNC)libnsp_crypto_aes_decrypt);
	nsp_setcfunc(N, tobj, "aes_ecb_encrypt", (NSP_CFUNC)libnsp_crypto_aes_encrypt);
	nsp_setcfunc(N, tobj, "aes_ecb_decrypt", (NSP_CFUNC)libnsp_crypto_aes_decrypt);
	nsp_setcfunc(N, tobj, "md5_passwd", (NSP_CFUNC)libnsp_crypto_md5_passwd);
	tobj = nsp_settable(N, &N->g, "file");
	nsp_setcfunc(N, tobj, "md5", (NSP_CFUNC)libnsp_crypto_md5_file);
	nsp_setcfunc(N, tobj, "sha1", (NSP_CFUNC)libnsp_crypto_sha1_file);
	tobj = nsp_settable(N, &N->g, "string");
	nsp_setcfunc(N, tobj, "md5", (NSP_CFUNC)libnsp_crypto_md5_string);
	nsp_setcfunc(N, tobj, "sha1", (NSP_CFUNC)libnsp_crypto_sha1_string);
#if defined HAVE_OPENSSL
	nspopenssl_register_all(N);
#endif
	return 0;
}

#ifdef PIC
DllExport int nsplib_init(nsp_state *N)
{
	nspcrypto_register_all(N);
	return 0;
}
#endif
