/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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

#undef HAVE_MYSQL
#undef PACKAGE_BUGREPORT
//#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

//#include "php_embed.h"
/*
#include "php.h"
#include "php_globals.h"
#include "php_variables.h"
#include "zend_modules.h"

#include "SAPI.h"
*/
/*
#include <php.h>
#include <SAPI.h>
#include <php_main.h>
*/
//#include "php/php.h"
//#include "php/SAPI.h"
//#include "php/php_main.h"


/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Edin Kadribasic <edink@php.net>                              |
   +----------------------------------------------------------------------+
*/
/* $Id: php_embed.h,v 1.1.2.2 2003/01/28 10:54:17 edink Exp $ */

#ifndef _PHP_EMBED_H_
#define _PHP_EMBED_H_

#include <main/php.h>
#include <main/SAPI.h>
#include <main/php_main.h>
#include <main/php_variables.h>
#include <main/php_ini.h>
#include <zend_ini.h>

#ifdef ZTS
#define PTSRMLS_D        void ****ptsrm_ls
#define PTSRMLS_DC       , PTSRMLS_D
#define PTSRMLS_C        &tsrm_ls
#define PTSRMLS_CC       , PTSRMLS_C
#else
#define PTSRMLS_D
#define PTSRMLS_DC
#define PTSRMLS_C
#define PTSRMLS_CC
#endif

#define PHP_EMBED_START_BLOCK(x,y) { \
    void ***tsrm_ls; \
    php_embed_init(x, y PTSRMLS_CC); \
    zend_first_try {

#define PHP_EMBED_END_BLOCK() \
  } zend_catch { \
    /* int exit_status = EG(exit_status); */ \
  } zend_end_try(); \
  php_embed_shutdown(TSRMLS_C); \
}

BEGIN_EXTERN_C() 
int php_embed_init(int argc, char **argv PTSRMLS_DC);
void php_embed_shutdown(TSRMLS_D);
extern sapi_module_struct php_embed_module;
END_EXTERN_C()

#endif /* _PHP_EMBED_H_ */
