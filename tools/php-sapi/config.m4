dnl
dnl $Id: config.m4,v 1.17.2.5 2003/11/27 23:04:49 sniper Exp $
dnl

AC_MSG_CHECKING(for NullLogic Groupware)
AC_ARG_WITH(nullgw,
[  --with-nullgw=SRCDIR    Build PHP as NullLogic Groupware module],
[
  if test ! -d $withval; then
    AC_MSG_RESULT(nullgw directory does not exist ($withval))
  fi
  PHP_EXPAND_PATH($withval, NULLGW)
  PHP_ADD_INCLUDE($NULLGW/include)
  PHP_SELECT_SAPI(nullgw, shared, mod_php.c)
  INSTALL_IT="strip $SAPI_SHARED ; \
    \$(INSTALL) -m 0755 $SAPI_SHARED $NULLGW/distrib/lib/ghttpd/mod_php.so ; \
    \$(INSTALL) -m 0755 $SAPI_SHARED $NULLGW/distrib/lib/httpd/mod_php.so"
  PHP_NULLGW="yes, using $NULLGW"
],[
  PHP_NULLGW=no
])
AC_MSG_RESULT($PHP_NULLGW)
