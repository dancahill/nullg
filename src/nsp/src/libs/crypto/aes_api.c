/**
 * rijndael-api-fst.c
 *
 * @version 2.9 (December 2000)
 *
 * Optimised ANSI C code for the Rijndael cipher (now AES)
 *
 * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Acknowledgements:
 *
 * We are deeply indebted to the following people for their bug reports,
 * fixes, and improvement suggestions to this implementation. Though we
 * tried to list all contributions, we apologise in advance for any
 * missing reference.
 *
 * Andrew Bales <Andrew.Bales@Honeywell.com>
 * Markus Friedl <markus.friedl@informatik.uni-erlangen.de>
 * John Skodon <skodonj@webquill.com>
 */

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "aes_api.h"
#include "aes_core.h"

int makeKey(keyInstance *key, BYTE direction, int keyLen, char *keyMaterial) {
	int i;
	char *keyMat;
	u8 cipherKey[MAXKB];

	if (key == NULL) {
		return BAD_KEY_INSTANCE;
	}

	if ((direction == DIR_ENCRYPT) || (direction == DIR_DECRYPT)) {
		key->direction = direction;
	}
	else {
		return BAD_KEY_DIR;
	}

	if ((keyLen == 128) || (keyLen == 192) || (keyLen == 256)) {
		key->keyLen = keyLen;
	}
	else {
		return BAD_KEY_MAT;
	}

	if (keyMaterial != NULL) {
		strncpy(key->keyMaterial, keyMaterial, keyLen / 4);
	}

	/* initialize key schedule: */
	keyMat = key->keyMaterial;
	for (i = 0; i < key->keyLen / 8; i++) {
		int t, v;

		t = *keyMat++;
		if ((t >= '0') && (t <= '9')) v = (t - '0') << 4;
		else if ((t >= 'a') && (t <= 'f')) v = (t - 'a' + 10) << 4;
		else if ((t >= 'A') && (t <= 'F')) v = (t - 'A' + 10) << 4;
		else return BAD_KEY_MAT;

		t = *keyMat++;
		if ((t >= '0') && (t <= '9')) v ^= (t - '0');
		else if ((t >= 'a') && (t <= 'f')) v ^= (t - 'a' + 10);
		else if ((t >= 'A') && (t <= 'F')) v ^= (t - 'A' + 10);
		else return BAD_KEY_MAT;

		cipherKey[i] = (u8)v;
	}
	if (direction == DIR_ENCRYPT) {
		key->Nr = rijndaelKeySetupEnc(key->rk, cipherKey, keyLen);
	}
	else {
		key->Nr = rijndaelKeySetupDec(key->rk, cipherKey, keyLen);
	}
	rijndaelKeySetupEnc(key->ek, cipherKey, keyLen);
	return TRUE;
}

int cipherInit(cipherInstance *cipher, BYTE mode, char *IV) {
	if ((mode == MODE_ECB) || (mode == MODE_CBC) || (mode == MODE_CFB1)) {
		cipher->mode = mode;
	}
	else {
		return BAD_CIPHER_MODE;
	}
	if (IV != NULL) {
		int i;
		for (i = 0; i < MAX_IV_SIZE; i++) {
			int t, j;

			t = IV[2 * i];
			if ((t >= '0') && (t <= '9')) j = (t - '0') << 4;
			else if ((t >= 'a') && (t <= 'f')) j = (t - 'a' + 10) << 4;
			else if ((t >= 'A') && (t <= 'F')) j = (t - 'A' + 10) << 4;
			else return BAD_CIPHER_INSTANCE;

			t = IV[2 * i + 1];
			if ((t >= '0') && (t <= '9')) j ^= (t - '0');
			else if ((t >= 'a') && (t <= 'f')) j ^= (t - 'a' + 10);
			else if ((t >= 'A') && (t <= 'F')) j ^= (t - 'A' + 10);
			else return BAD_CIPHER_INSTANCE;

			cipher->IV[i] = (u8)j;
		}
	}
	else {
		memset(cipher->IV, 0, MAX_IV_SIZE);
	}
	return TRUE;
}

int blockEncrypt(cipherInstance *cipher, keyInstance *key, BYTE *input, int inputLen, BYTE *outBuffer) {
	int i, k, t, numBlocks;
	u8 block[16], *iv;

	if (cipher == NULL ||
		key == NULL ||
		key->direction == DIR_DECRYPT) {
		return BAD_CIPHER_STATE;
	}
	if (input == NULL || inputLen <= 0) {
		return 0; /* nothing to do */
	}

	numBlocks = inputLen / 128;

	switch (cipher->mode) {
	case MODE_ECB:
		for (i = numBlocks; i > 0; i--) {
			rijndaelEncrypt(key->rk, key->Nr, input, outBuffer);
			input += 16;
			outBuffer += 16;
		}
		break;

	case MODE_CBC:
		iv = cipher->IV;
		for (i = numBlocks; i > 0; i--) {
			((u32*)block)[0] = ((u32*)input)[0] ^ ((u32*)iv)[0];
			//block[0] = input[0] ^ iv[0];
			((u32*)block)[1] = ((u32*)input)[1] ^ ((u32*)iv)[1];
			((u32*)block)[2] = ((u32*)input)[2] ^ ((u32*)iv)[2];
			((u32*)block)[3] = ((u32*)input)[3] ^ ((u32*)iv)[3];
			rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
			iv = outBuffer;
			input += 16;
			outBuffer += 16;
		}
		break;

	case MODE_CFB1:
		iv = cipher->IV;
		for (i = numBlocks; i > 0; i--) {
			memcpy(outBuffer, input, 16);
			for (k = 0; k < 128; k++) {
				rijndaelEncrypt(key->ek, key->Nr, iv, block);
				outBuffer[k >> 3] ^= (block[0] & 0x80U) >> (k & 7);
				for (t = 0; t < 15; t++) {
					iv[t] = (iv[t] << 1) | (iv[t + 1] >> 7);
				}
				iv[15] = (iv[15] << 1) | ((outBuffer[k >> 3] >> (7 - (k & 7))) & 1);
			}
			outBuffer += 16;
			input += 16;
		}
		break;

	default:
		return BAD_CIPHER_STATE;
	}

	return 128 * numBlocks;
}

/**
 * Encrypt data partitioned in octets, using RFC 2040-like padding.
 *
 * @param   input           data to be encrypted (octet sequence)
 * @param   inputOctets		input length in octets (not bits)
 * @param   outBuffer       encrypted output data
 *
 * @return	length in octets (not bits) of the encrypted output buffer.
 */
int padEncrypt(cipherInstance *cipher, keyInstance *key, BYTE *input, int inputOctets, BYTE *outBuffer) {
	int i, numBlocks, padLen;
	u8 block[16], *iv;

	if (cipher == NULL ||
		key == NULL ||
		key->direction == DIR_DECRYPT) {
		return BAD_CIPHER_STATE;
	}
	if (input == NULL || inputOctets <= 0) {
		return 0; /* nothing to do */
	}

	numBlocks = inputOctets / 16;

	switch (cipher->mode) {
	case MODE_ECB:
		for (i = numBlocks; i > 0; i--) {
			rijndaelEncrypt(key->rk, key->Nr, input, outBuffer);
			input += 16;
			outBuffer += 16;
		}
		padLen = 16 - (inputOctets - 16 * numBlocks);
		assert(padLen > 0 && padLen <= 16);
		memcpy(block, input, 16 - padLen);
		memset(block + 16 - padLen, padLen, padLen);
		rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
		break;

	case MODE_CBC:
		iv = cipher->IV;
		for (i = numBlocks; i > 0; i--) {
			((u32*)block)[0] = ((u32*)input)[0] ^ ((u32*)iv)[0];
			((u32*)block)[1] = ((u32*)input)[1] ^ ((u32*)iv)[1];
			((u32*)block)[2] = ((u32*)input)[2] ^ ((u32*)iv)[2];
			((u32*)block)[3] = ((u32*)input)[3] ^ ((u32*)iv)[3];
			rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
			iv = outBuffer;
			input += 16;
			outBuffer += 16;
		}
		padLen = 16 - (inputOctets - 16 * numBlocks);
		assert(padLen > 0 && padLen <= 16);
		for (i = 0; i < 16 - padLen; i++) {
			block[i] = input[i] ^ iv[i];
		}
		for (i = 16 - padLen; i < 16; i++) {
			block[i] = (BYTE)padLen ^ iv[i];
		}
		rijndaelEncrypt(key->rk, key->Nr, block, outBuffer);
		break;

	default:
		return BAD_CIPHER_STATE;
	}

	return 16 * (numBlocks + 1);
}

int blockDecrypt(cipherInstance *cipher, keyInstance *key, BYTE *input, int inputLen, BYTE *outBuffer) {
	int i, k, t, numBlocks;
	u8 block[16], *iv;

	if (cipher == NULL ||
		key == NULL ||
		(cipher->mode != MODE_CFB1 && key->direction == DIR_ENCRYPT)) {
		return BAD_CIPHER_STATE;
	}
	if (input == NULL || inputLen <= 0) {
		return 0; /* nothing to do */
	}

	numBlocks = inputLen / 128;

	switch (cipher->mode) {
	case MODE_ECB:
		for (i = numBlocks; i > 0; i--) {
			rijndaelDecrypt(key->rk, key->Nr, input, outBuffer);
			input += 16;
			outBuffer += 16;
		}
		break;

	case MODE_CBC:
		iv = cipher->IV;
		for (i = numBlocks; i > 0; i--) {
			rijndaelDecrypt(key->rk, key->Nr, input, block);
			((u32*)block)[0] ^= ((u32*)iv)[0];
			((u32*)block)[1] ^= ((u32*)iv)[1];
			((u32*)block)[2] ^= ((u32*)iv)[2];
			((u32*)block)[3] ^= ((u32*)iv)[3];
			memcpy(cipher->IV, input, 16);
			memcpy(outBuffer, block, 16);
			input += 16;
			outBuffer += 16;
		}
		break;

	case MODE_CFB1:
		iv = cipher->IV;
		for (i = numBlocks; i > 0; i--) {
			memcpy(outBuffer, input, 16);
			for (k = 0; k < 128; k++) {
				rijndaelEncrypt(key->ek, key->Nr, iv, block);
				for (t = 0; t < 15; t++) {
					iv[t] = (iv[t] << 1) | (iv[t + 1] >> 7);
				}
				iv[15] = (iv[15] << 1) | ((input[k >> 3] >> (7 - (k & 7))) & 1);
				outBuffer[k >> 3] ^= (block[0] & 0x80U) >> (k & 7);
			}
			outBuffer += 16;
			input += 16;
		}
		break;

	default:
		return BAD_CIPHER_STATE;
	}

	return 128 * numBlocks;
}

int padDecrypt(cipherInstance *cipher, keyInstance *key, BYTE *input, int inputOctets, BYTE *outBuffer) {
	int i, numBlocks, padLen;
	u8 block[16];

	if (cipher == NULL ||
		key == NULL ||
		key->direction == DIR_ENCRYPT) {
		return BAD_CIPHER_STATE;
	}
	if (input == NULL || inputOctets <= 0) {
		return 0; /* nothing to do */
	}
	if (inputOctets % 16 != 0) {
		return BAD_DATA;
	}

	numBlocks = inputOctets / 16;

	switch (cipher->mode) {
	case MODE_ECB:
		/* all blocks but last */
		for (i = numBlocks - 1; i > 0; i--) {
			rijndaelDecrypt(key->rk, key->Nr, input, outBuffer);
			input += 16;
			outBuffer += 16;
		}
		/* last block */
		rijndaelDecrypt(key->rk, key->Nr, input, block);
		padLen = block[15];
		/* padLen of 16 was valid when encrypting...  which one is rignt? */
//		if (padLen >= 16) { <- original
		if (padLen > 16) {
			//			printf("%d[padlen = %d]\n", __LINE__, padLen);
			return BAD_DATA;
		}
		for (i = 16 - padLen; i < 16; i++) {
			if (block[i] != padLen) {
				return BAD_DATA;
			}
		}
		memcpy(outBuffer, block, 16 - padLen);
		break;

	case MODE_CBC:
		/* all blocks but last */
		for (i = numBlocks - 1; i > 0; i--) {
			rijndaelDecrypt(key->rk, key->Nr, input, block);
			((u32*)block)[0] ^= ((u32*)cipher->IV)[0];
			((u32*)block)[1] ^= ((u32*)cipher->IV)[1];
			((u32*)block)[2] ^= ((u32*)cipher->IV)[2];
			((u32*)block)[3] ^= ((u32*)cipher->IV)[3];
			memcpy(cipher->IV, input, 16);
			memcpy(outBuffer, block, 16);
			input += 16;
			outBuffer += 16;
		}
		/* last block */
		rijndaelDecrypt(key->rk, key->Nr, input, block);
		((u32*)block)[0] ^= ((u32*)cipher->IV)[0];
		((u32*)block)[1] ^= ((u32*)cipher->IV)[1];
		((u32*)block)[2] ^= ((u32*)cipher->IV)[2];
		((u32*)block)[3] ^= ((u32*)cipher->IV)[3];
		padLen = block[15];
		if (padLen <= 0 || padLen > 16) {
			return BAD_DATA;
		}
		for (i = 16 - padLen; i < 16; i++) {
			if (block[i] != padLen) {
				return BAD_DATA;
			}
		}
		memcpy(outBuffer, block, 16 - padLen);
		break;

	default:
		return BAD_CIPHER_STATE;
	}

	return 16 * numBlocks - padLen;
}

#ifdef INTERMEDIATE_VALUE_KAT
/**
 *	cipherUpdateRounds:
 *
 *	Encrypts/Decrypts exactly one full block a specified number of rounds.
 *	Only used in the Intermediate Value Known Answer Test.
 *
 *	Returns:
 *		TRUE - on success
 *		BAD_CIPHER_STATE - cipher in bad state (e.g., not initialized)
 */
int cipherUpdateRounds(cipherInstance *cipher, keyInstance *key,
	BYTE *input, int inputLen, BYTE *outBuffer, int rounds) {
	u8 block[16];

	if (cipher == NULL || key == NULL) {
		return BAD_CIPHER_STATE;
	}

	memcpy(block, input, 16);

	switch (key->direction) {
	case DIR_ENCRYPT:
		rijndaelEncryptRound(key->rk, key->Nr, block, rounds);
		break;

	case DIR_DECRYPT:
		rijndaelDecryptRound(key->rk, key->Nr, block, rounds);
		break;

	default:
		return BAD_KEY_DIR;
	}

	memcpy(outBuffer, block, 16);

	return TRUE;
}
#endif /* INTERMEDIATE_VALUE_KAT */

/*
    NESLA NullLogic Embedded Scripting Language
    Copyright (C) 2007-2018 Dan Cahill

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

/* should only need 16... */
#define CHICKENBUF 64

static int prep_key(keyInstance *key, int keylen, BYTE dir, char *keytext, int keytextlen)
{
	BYTE keyMaterial[320];
	const char *hex = "0123456789abcdef";
	int i;

	memset(keyMaterial, 0, sizeof(keyMaterial));
	memset(keyMaterial, '0', keylen / 4);
	if (memcmp(keytext, "hex:", 4) == 0) {
		keytext += 4;
		for (i = 0;i < keylen / 4;i++) {
			if (!keytext[i]) break;
			keyMaterial[i] = keytext[i];
		}
	}
	else {
		for (i = 0;i < keylen / 8;i++) {
			if (!keytext[i] || !keytext[i + 1]) break;
			keyMaterial[i * 2 + 0] = hex[keytext[i] >> 4];
			keyMaterial[i * 2 + 1] = hex[keytext[i] & 15];
		}
	}
	keyMaterial[keylen / 4] = 0;
	//	printf("[%s][%s]\n", keyMaterial, keytext);
	//	printf("[%s]\n", keyMaterial);
	makeKey(key, dir, keylen, (char *)keyMaterial);
	return TRUE;
}

NSP_FUNCTION(libnsp_crypto_aes_encrypt)
{
	char *fname = nsp_getstr(N, &N->l, "0");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3");
	obj_t *cobj4 = nsp_getobj(N, &N->l, "4");
	obj_t *robj;
	char *iv = NULL;
	keyInstance keyInst;
	cipherInstance cipherInst;
	int keylen = 128;
	int n;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, fname, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, fname, "expected a string for arg2");
	if (cobj3->val->type == NT_NUMBER) {
		switch ((int)cobj3->val->d.num) {
		case 128: keylen = 128; break;
		case 192: keylen = 192; break;
		case 256: keylen = 256; break;
		default: n_error(N, NE_SYNTAX, fname, "invalid key length");
		}
		if (cobj4->val->type == NT_STRING) {
			if (cobj4->val->size != 0 && cobj4->val->size != 32) n_error(N, NE_SYNTAX, fname, "expected 32 bytes of hex data");
			iv = cobj4->val->d.str;
		}
	}
	prep_key(&keyInst, keylen, DIR_ENCRYPT, cobj2->val->d.str, cobj2->val->size);
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	if ((robj->val->d.str = n_alloc(N, cobj1->val->size + 1 + CHICKENBUF, 0)) == NULL) n_error(N, NE_SYNTAX, fname, "malloc error");
	robj->val->size = cobj1->val->size + 1 + CHICKENBUF;
	if (nc_strcmp(fname, "aes_cbc_encrypt") == 0) {
		/* Cipher Block Chaining (CBC) Mode */
		n = cipherInit(&cipherInst, MODE_CBC, iv);
	}
	else if (nc_strcmp(fname, "aes_ecb_encrypt") == 0) {
		/* Electronic Codebook (ECB) Mode */
		n = cipherInit(&cipherInst, MODE_ECB, iv);
	}
	else {
		n = BAD_CIPHER_INSTANCE;
	}
	switch (n) {
	case BAD_CIPHER_INSTANCE: n_error(N, NE_SYNTAX, fname, "BAD_CIPHER_INSTANCE");
	default: if (n < 0) n_error(N, NE_SYNTAX, fname, "broken cipher init %d", n);
	}
	n = padEncrypt(&cipherInst, &keyInst, (uchar *)cobj1->val->d.str, cobj1->val->size, (uchar *)robj->val->d.str);
	switch (n) {
	case BAD_DATA: n_error(N, NE_SYNTAX, fname, "Data contents are invalid, e.g., invalid padding");
	case BAD_CIPHER_STATE: n_error(N, NE_SYNTAX, fname, "Cipher in wrong state (e.g., not initialized)");
	default: if (n < 0) n_error(N, NE_SYNTAX, fname, "broken size returned %d", n);
	}
	//	printf("e[%d][%d] %d %d\n", cobj1->val->size, n, sizeof(keyInst), sizeof(cipherInst));
	robj->val->size = n;
	robj->val->d.str[n] = '\0';
	return 0;
}

NSP_FUNCTION(libnsp_crypto_aes_decrypt)
{
	char *fname = nsp_getstr(N, &N->l, "0");
	obj_t *cobj1 = nsp_getobj(N, &N->l, "1");
	obj_t *cobj2 = nsp_getobj(N, &N->l, "2");
	obj_t *cobj3 = nsp_getobj(N, &N->l, "3");
	obj_t *cobj4 = nsp_getobj(N, &N->l, "4");
	obj_t *robj;
	char *iv = NULL;
	keyInstance keyInst;
	cipherInstance cipherInst;
	int keylen = 128;
	int n;

	if (cobj1->val->type != NT_STRING) n_error(N, NE_SYNTAX, fname, "expected a string for arg1");
	if (cobj2->val->type != NT_STRING) n_error(N, NE_SYNTAX, fname, "expected a string for arg2");
	if (cobj3->val->type == NT_NUMBER) {
		switch ((int)cobj3->val->d.num) {
		case 128: keylen = 128; break;
		case 192: keylen = 192; break;
		case 256: keylen = 256; break;
		default: n_error(N, NE_SYNTAX, fname, "invalid key length");
		}
		if (cobj4->val->type == NT_STRING) {
			if (cobj4->val->size != 0 && cobj4->val->size != 32) n_error(N, NE_SYNTAX, fname, "expected 32 bytes of hex data");
			iv = cobj4->val->d.str;
		}
	}
	prep_key(&keyInst, keylen, DIR_DECRYPT, cobj2->val->d.str, cobj2->val->size);
	robj = nsp_setstr(N, &N->r, "", NULL, 0);
	if ((robj->val->d.str = n_alloc(N, cobj1->val->size + 1 + CHICKENBUF, 0)) == NULL) n_error(N, NE_SYNTAX, fname, "malloc error");
	robj->val->size = cobj1->val->size + 1 + CHICKENBUF;
	if (nc_strcmp(fname, "aes_cbc_decrypt") == 0) {
		/* Cipher Block Chaining (CBC) Mode */
		n = cipherInit(&cipherInst, MODE_CBC, iv);
	}
	else if (nc_strcmp(fname, "aes_ecb_decrypt") == 0) {
		/* Electronic Codebook (ECB) Mode */
		n = cipherInit(&cipherInst, MODE_ECB, iv);
	}
	else {
		n = BAD_CIPHER_INSTANCE;
	}
	switch (n) {
	case BAD_CIPHER_INSTANCE: n_error(N, NE_SYNTAX, fname, "BAD_CIPHER_INSTANCE");
	default: if (n < 0) n_error(N, NE_SYNTAX, fname, "broken cipher init %d", n);
	}
	n = padDecrypt(&cipherInst, &keyInst, (uchar *)cobj1->val->d.str, cobj1->val->size, (uchar *)robj->val->d.str);
	switch (n) {
	case BAD_DATA:
		n_warn(N, fname, "Data contents are invalid, e.g., invalid padding");
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	case BAD_CIPHER_STATE:
		n_warn(N, fname, "Cipher in wrong state (e.g., not initialized)");
		nsp_setnum(N, &N->r, "", -1);
		return -1;
	default:
		if (n < 0) {
			n_warn(N, fname, "broken size returned %d", n);
			nsp_setnum(N, &N->r, "", -1);
			return -1;
		}
	}
	//	printf("d[%d][%d] %d %d\n", cobj1->val->size, n, sizeof(keyInst), sizeof(cipherInst));
	robj->val->size = n;
	robj->val->d.str[n] = '\0';
	return 0;
}
