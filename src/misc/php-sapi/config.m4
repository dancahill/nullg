dnl
dnl $Id: config.m4,v 1.17.2.5 2003/11/27 23:04:49 sniper Exp $
dnl

AC_MSG_CHECKING(for NullLogic GroupServer)
AC_ARG_WITH(nullg,
[  --with-nullg=SRCDIR    Build PHP as NullLogic GroupServer module],
[
  if test ! -d $withval; then
    AC_MSG_RESULT(nullg directory does not exist ($withval))
  fi
  PHP_EXPAND_PATH($withval, NULLG)
  PHP_ADD_INCLUDE($NULLG/include)
  PHP_SELECT_SAPI(nullg, shared, mod_php.c)
  INSTALL_IT="strip $SAPI_SHARED ; \
    \$(INSTALL) -m 0755 $SAPI_SHARED $NULLG/lib/httpd/mod_php.so"
  PHP_NULLG="yes, using $NULLG"
],[
  PHP_NULLG=no
])
AC_MSG_RESULT($PHP_NULLG)
