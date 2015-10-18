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

#include "nsp/nsplib.h"
#include "crypto.h"

#if defined HAVE_OPENSSL

static void seed_prng(void)
{
	RAND_load_file("/dev/urandom", 1024);
}


/* Example 10-8. A signing and verifying utility */
/*
* This code appearing before the main function is all for X509_STORE setup.
*/
/* these are defintions to make the example simpler */
#define CA_FILE "CAfile.pem"
#define CA_DIR "/etc/nullgw/ssl"
#define CRL_FILE "CRLfile.pem"
static int verify_callback(int ok, X509_STORE_CTX *stor)
{
	if (!ok) {
		//		n_warn(NULL, __FN__, "Error: %s", X509_verify_cert_error_string(stor->error));
		//		printf("Error: %s\r\n", X509_verify_cert_error_string(stor->error));
	}
	return ok;
}

static X509_STORE *create_store(nsp_state *N)
{
#define __FN__ __FILE__ ":create_store()"
	X509_STORE *store;
	X509_LOOKUP *lookup;

	if (!(store = X509_STORE_new())) {
		n_warn(N, __FN__, "Error creating X509_STORE_CTX object");
		goto err;
	}
	X509_STORE_set_verify_cb_func(store, verify_callback);
	if (X509_STORE_load_locations(store, CA_FILE, CA_DIR) != 1) {
		n_warn(N, __FN__, "Error loading the CA file or directory");
		goto err;
	}
	if (X509_STORE_set_default_paths(store) != 1) {
		n_warn(N, __FN__, "Error loading the system-wide CA certificates");
		goto err;
	}
	if (!(lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file()))) {
		n_warn(N, __FN__, "Error creating X509_LOOKUP object");
		goto err;
	}
	if (X509_load_crl_file(lookup, CRL_FILE, X509_FILETYPE_PEM) != 1) {
		n_warn(N, __FN__, "Error reading the CRL file");
		goto err;
	}
	X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
	return store;
err:
	return NULL;
#undef __FN__
}

NSP_FUNCTION(libnsp_openssl_smime_decrypt)
{
#define __FN__ __FILE__ ":libnsp_openssl_smime_decrypt()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* privkey.pem */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* cert.pem */
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3"); /* infile */
	obj_t *cobj4 = nsp_getobj(N, &N->l, "4"); /* outfile */
	PKCS7 *pkcs7;
	X509 *cert;
	EVP_PKEY *pkey;
	FILE *fp;
	BIO *pkcs7_bio, *in, *out;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj3->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");
	if (cobj4->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg4");
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	seed_prng();
	/* if (!(in = BIO_new_fp(stdin, BIO_NOCLOSE)) || !(out = BIO_new_fp(stdout, BIO_NOCLOSE))) { */
	if (!(in = BIO_new_fp(fopen(cobj3->val->d.str, "rb"), BIO_NOCLOSE)) || !(out = BIO_new_fp(fopen(cobj4->val->d.str, "wb"), BIO_NOCLOSE))) {
		n_warn(N, __FN__, "Error creating BIO objects");
		goto err;
	}
	if (!(fp = fopen(cobj1->val->d.str, "r")) || !(pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL))) {
		n_warn(N, __FN__, "Error reading private key in %s", cobj1->val->d.str);
		goto err;
	}
	fclose(fp);
	if (!(fp = fopen(cobj2->val->d.str, "r")) || !(cert = PEM_read_X509(fp, NULL, NULL, NULL))) {
		n_warn(N, __FN__, "Error reading decryption certificate in %s", cobj2->val->d.str);
		goto err;
	}
	fclose(fp);
	if (!(pkcs7 = SMIME_read_PKCS7(in, &pkcs7_bio))) {
		n_warn(N, __FN__, "Error reading PKCS#7 object");
		goto err;
	}
	if (PKCS7_decrypt(pkcs7, pkey, cert, out, 0) != 1) {
		n_warn(N, __FN__, "Error decrypting PKCS#7 object");
		goto err;
	}

	BIO_ctrl(out, BIO_CTRL_FLUSH, 0, NULL);
	BIO_free_all(out);

	return 0;
err:
	return -1;
#undef __FN__
}

NSP_FUNCTION(libnsp_openssl_smime_encrypt)
{
#define __FN__ __FILE__ ":libnsp_openssl_smime_encrypt()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* privkey.pem */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* cert.pem */
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3"); /* infile */
	obj_t *cobj4 = nsp_getobj(N, &N->l, "4"); /* outfile */
	PKCS7 *pkcs7;
	const EVP_CIPHER *cipher;
	STACK_OF(X509) *certs;
	FILE *fp;
	BIO *in, *out;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj3->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");
	if (cobj4->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg4");
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	seed_prng();
	/* if (!(in = BIO_new_fp(stdin, BIO_NOCLOSE)) || !(out = BIO_new_fp(stdout, BIO_NOCLOSE))) { */
	if (!(in = BIO_new_fp(fopen(cobj3->val->d.str, "rb"), BIO_NOCLOSE)) || !(out = BIO_new_fp(fopen(cobj4->val->d.str, "wb"), BIO_NOCLOSE))) {
		n_warn(N, __FN__, "Error creating BIO objects");
		goto err;
	}
	/* choose cipher and read in all certificates as encryption targets */
	cipher = EVP_des_ede3_cbc();
	certs = sk_X509_new_null();
	//	while (argc) {
	X509 *tmp;

	if (!(fp = fopen(cobj2->val->d.str, "r")) || !(tmp = PEM_read_X509(fp, NULL, NULL, NULL))) {
		n_warn(N, __FN__, "Error reading encryption certificate in %s", cobj2->val->d.str);
		goto err;
	}
	sk_X509_push(certs, tmp);
	fclose(fp);
	//		--argc, ++argv;
	//	}
	if (!(pkcs7 = PKCS7_encrypt(certs, in, cipher, 0))) {
		ERR_print_errors_fp(stderr);
		n_warn(N, __FN__, "Error making the PKCS#7 object");
		goto err;
	}
	if (SMIME_write_PKCS7(out, pkcs7, in, 0) != 1) {
		n_warn(N, __FN__, "Error writing the S/MIME data");
		goto err;
	}

	BIO_ctrl(out, BIO_CTRL_FLUSH, 0, NULL);
	BIO_free_all(out);

	return 0;
err:
	return -1;

#undef __FN__
}

NSP_FUNCTION(libnsp_openssl_smime_sign)
{
#define __FN__ __FILE__ ":libnsp_openssl_smime_sign()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* privkey.pem */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* cert.pem */
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3"); /* infile */
	obj_t *cobj4 = nsp_getobj(N, &N->l, "4"); /* outfile */
	X509 *cert;
	EVP_PKEY *pkey;
	STACK_OF(X509) *chain = NULL;
	PKCS7 *pkcs7;
	FILE *fp;
	BIO *in, *out;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj3->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");
	if (cobj4->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg4");
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	seed_prng();
	/* if (!(in = BIO_new_fp(stdin, BIO_NOCLOSE)) || !(out = BIO_new_fp(stdout, BIO_NOCLOSE))) { */
	if (!(in = BIO_new_fp(fopen(cobj3->val->d.str, "rb"), BIO_NOCLOSE)) || !(out = BIO_new_fp(fopen(cobj4->val->d.str, "wb"), BIO_NOCLOSE))) {
		n_warn(N, __FN__, "Error creating BIO objects");
		goto err;
	}
	if (!(fp = fopen(cobj1->val->d.str, "r")) || !(pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL))) {
		n_warn(N, __FN__, "Error reading signer private key in %s", cobj1->val->d.str);
		goto err;
	}
	fclose(fp);
	/* read the signer certificate */
	if (!(fp = fopen(cobj2->val->d.str, "r")) || !(cert = PEM_read_X509(fp, NULL, NULL, NULL))) {
		ERR_print_errors_fp(stderr);
		n_warn(N, __FN__, "Error reading signer certificate in %s", cobj2->val->d.str);
		goto err;
	}
	fclose(fp);
	//	--argc, ++argv;
	//	if (argc)
	//		chain = sk_X509_new_null();
	//	while (argc) {
	//		X509 *tmp;
	//
	//		if (!(fp = fopen(*argv, "r")) || !(tmp = PEM_read_X509(fp, NULL, NULL, NULL))) {
	//			n_warn(N, __FN__, "Error reading chain certificate in %s", *argv);
	//			goto err;
	//		}
	//		sk_X509_push(chain, tmp);
	//		fclose(fp);
	//		--argc, ++argv;
	//	}
	if (!(pkcs7 = PKCS7_sign(cert, pkey, chain, in, 0))) {
		n_warn(N, __FN__, "Error making the PKCS#7 object");
		goto err;
	}
	if (SMIME_write_PKCS7(out, pkcs7, in, 0) != 1) {
		n_warn(N, __FN__, "Error writing the S/MIME data");
		goto err;
	}

	BIO_ctrl(out, BIO_CTRL_FLUSH, 0, NULL);
	BIO_free_all(out);

	return 0;
err:
	return -1;
#undef __FN__
}

NSP_FUNCTION(libnsp_openssl_smime_verify)
{
#define __FN__ __FILE__ ":libnsp_openssl_smime_verify()"
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1"); /* privkey.pem */
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2"); /* cert.pem */
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3"); /* infile */
	obj_t *cobj4 = nsp_getobj(N, &N->l, "4"); /* outfile */
	X509 *cert;
	STACK_OF(X509) *chain = NULL;
	X509_STORE *store = NULL;
	PKCS7 *pkcs7;
	FILE *fp;
	BIO *in, *out, *pkcs7_bio;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg2");
	if (cobj3->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg3");
	if (cobj4->val->type != NT_STRING) n_error(N, NE_SYNTAX, __FN__, "expected a string for arg4");
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
	seed_prng();
	/* if (!(in = BIO_new_fp(stdin, BIO_NOCLOSE)) || !(out = BIO_new_fp(stdout, BIO_NOCLOSE))) { */
	if (!(in = BIO_new_fp(fopen(cobj3->val->d.str, "rb"), BIO_NOCLOSE)) || !(out = BIO_new_fp(fopen(cobj4->val->d.str, "wb"), BIO_NOCLOSE))) {
		n_warn(N, __FN__, "Error creating BIO objects");
		goto err;
	}
	/* create the cert store and set the verify callback */
	if (!(store = create_store(N))) {
		n_warn(N, __FN__, "Error setting up X509_STORE object");
		//		goto err;
	}
	/* read the signer certificate */
	if (!(fp = fopen(cobj2->val->d.str, "r")) || !(cert = PEM_read_X509(fp, NULL, NULL, NULL))) {
		ERR_print_errors_fp(stderr);
		n_warn(N, __FN__, "Error reading signer certificate in %s", cobj2->val->d.str);
		goto err;
	}
	fclose(fp);
	//	if (argc)
	//		chain = sk_X509_new_null();
	//	while (argc) {
	//		X509 *tmp;
	//
	//		if (!(fp = fopen(*argv, "r")) || !(tmp = PEM_read_X509(fp, NULL, NULL, NULL))) {
	//			n_warn(N, __FN__, "Error reading chain certificate in %s", *argv);
	//			goto err;
	//		}
	//		sk_X509_push(chain, tmp);
	//		fclose(fp);
	//		--argc, ++argv;
	//	}
	if (!(pkcs7 = SMIME_read_PKCS7(in, &pkcs7_bio))) {
		n_warn(N, __FN__, "Error reading PKCS#7 object");
		goto err;
	}
	//	if (PKCS7_verify(pkcs7, chain, store, pkcs7_bio, out, PKCS7_NOINTERN) != 1) {
	//	if (PKCS7_verify(pkcs7, chain, store, pkcs7_bio, out, 0) != 1) {
	if (PKCS7_verify(pkcs7, chain, store, pkcs7_bio, out, PKCS7_NOVERIFY) != 1) {
		n_warn(N, __FN__, "Error writing PKCS#7 object");
		goto err;
	}
	else {
		n_warn(N, __FN__, "Certifiate and Signature verified!");
	}

	BIO_ctrl(out, BIO_CTRL_FLUSH, 0, NULL);
	BIO_free_all(out);

	return 0;
err:
	return -1;
#undef __FN__
}

#endif /* HAVE_OPENSSL */
