/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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
	#define X509_FILETYPE_PEM            1
	#define SSL_RECEIVED_SHUTDOWN        2
	#define SSL_FILETYPE_PEM             X509_FILETYPE_PEM
	#define SSLeay_add_ssl_algorithms()  SSL_library_init()
#endif
