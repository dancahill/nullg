@ECHO OFF
CD LIBNESLA
NMAKE /C /S /F MAKEFILE.VC
CD ..
CD WIN\GWMON
NMAKE /C /S /F GWMON.MAK
CD ..\..
CD CONFUTIL
NMAKE /C /S /F CONFUTIL.MAK
CD ..\DBUTIL
NMAKE /C /S /F DBUTIL.MAK
CD ..\SERVER
NMAKE /C /S /F SERVER.MAK
CD ..\MODULES\CORE
NMAKE /C /S /F VC.MAK
CD ..\GHTTPD
NMAKE /C /S /F VC.MAK
CD ..\HTTPD
NMAKE /C /S /F VC.MAK
CD ..\..
COPY \NullLogic\gw\libs\libsqlite3.dll ..\distrib\lib
COPY \NullLogic\gw\libs\libeay32.dll ..\distrib\lib
COPY \NullLogic\gw\libs\ssleay32.dll ..\distrib\lib
