/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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
#define HAVE_OPENSSL_SSL_H
#endif


#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
/*
	#include <openssl/rsa.h>
	#include <openssl/crypto.h>
	#include <openssl/x509.h>
	#include <openssl/pem.h>
	#include <openssl/err.h>
*/
#else
/* working substitutes for missing ssl headers */
typedef char SSL;
typedef char SSL_CTX;
typedef char SSL_METHOD;
typedef char EC_KEY;
#define X509_FILETYPE_PEM            1
#define SSL_RECEIVED_SHUTDOWN        2
#define SSL_FILETYPE_PEM             X509_FILETYPE_PEM
#define SSLeay_add_ssl_algorithms()  SSL_library_init()

#define NID_X9_62_prime256v1         415

#define SSL_CTRL_SET_TMP_ECDH        4
#define SSL_CTRL_OPTIONS             32
#define SSL_CTRL_SET_ECDH_AUTO       94

#define SSL_OP_NO_SSLv2              0x01000000L
#define SSL_OP_NO_SSLv3              0x02000000L
#define SSL_OP_NO_TLSv1              0x04000000L
#define SSL_OP_NO_TLSv1_2            0x08000000L
#define SSL_OP_NO_TLSv1_1            0x10000000L
#define SSL_OP_ALL                   0x80000BF7L
#endif
