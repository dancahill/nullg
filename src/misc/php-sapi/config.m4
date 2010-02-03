dnl
dnl $Id: config.m4,v 1.17.2.5 2003/11/27 23:04:49 sniper Exp $
dnl

AC_MSG_CHECKING(for NullLogic GroupServer)
AC_ARG_WITH(nullsd,
[  --with-nullsd=SRCDIR    Build PHP as NullLogic GroupServer module],
[
  if test ! -d $withval; then
    AC_MSG_RESULT(nullsd directory does not exist ($withval))
  fi
  PHP_EXPAND_PATH($withval, NULLSD)
  PHP_ADD_INCLUDE($NULLSD/include)
  PHP_SELECT_SAPI(nullsd, shared, mod_php.c)
  INSTALL_IT="strip $SAPI_SHARED ; \
    \$(INSTALL) -m 0755 $SAPI_SHARED $NULLSD/lib/httpd/mod_php.so"
  PHP_NULLSD="yes, using $NULLSD"
],[
  PHP_NULLSD=no
])
AC_MSG_RESULT($PHP_NULLSD)
