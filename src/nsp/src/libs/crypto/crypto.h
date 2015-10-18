/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2015 Dan Cahill

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

#if defined(HAVE_OPENSSL)
#include <stdio.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#endif

/* aes.c */
NSP_FUNCTION(libnsp_crypto_aes_decrypt);
NSP_FUNCTION(libnsp_crypto_aes_encrypt);
/* md5.c */
NSP_FUNCTION(libnsp_crypto_md5_file);
NSP_FUNCTION(libnsp_crypto_md5_string);
NSP_FUNCTION(libnsp_crypto_md5_passwd);
/* sha1.c */
NSP_FUNCTION(libnsp_crypto_sha1_file);
NSP_FUNCTION(libnsp_crypto_sha1_string);
/* smime.c */
NSP_FUNCTION(libnsp_openssl_smime_decrypt);
NSP_FUNCTION(libnsp_openssl_smime_encrypt);
NSP_FUNCTION(libnsp_openssl_smime_sign);
NSP_FUNCTION(libnsp_openssl_smime_verify);

int nspcrypto_register_all(nsp_state *N);
