dnl
dnl $Id: config.m4,v 1.17.2.5 2003/11/27 23:04:49 sniper Exp $
dnl

AC_MSG_CHECKING(for NullLogic GroupServer)
AC_ARG_WITH(nullgs,
[  --with-nullgs=SRCDIR    Build PHP as NullLogic GroupServer module],
[
  if test ! -d $withval; then
    AC_MSG_RESULT(nullgs directory does not exist ($withval))
  fi
  PHP_EXPAND_PATH($withval, NULLGS)
  PHP_ADD_INCLUDE($NULLGS/include)
  PHP_SELECT_SAPI(nullgs, shared, mod_php.c)
  INSTALL_IT="strip $SAPI_SHARED ; \
    \$(INSTALL) -m 0755 $SAPI_SHARED $NULLGS/lib/httpd/mod_php.so"
  PHP_NULLGS="yes, using $NULLGS"
],[
  PHP_NULLGS=no
])
AC_MSG_RESULT($PHP_NULLGS)
