@ECHO OFF
CD ..
REM DELTREE /Y DISTRIB\BIN\*.*
REM DELTREE /Y DISTRIB\LIB\*.*
MKDIR obj
MKDIR distrib\bin
MKDIR distrib\lib
MKDIR distrib\lib\core
MKDIR distrib\lib\httpd
COPY SQLITE\LIBSQLITE.DLL distrib\lib
CLS

CD TOOLS
CD WIN-CONFIG
NMAKE /C /S /F CONFIG.MAK
CD ..\WIN-GWMON
NMAKE /C /S /F GWMON.MAK
CD ..\..
CD DBUTIL
NMAKE /C /S /F DBUTIL.MAK
CD ..\SERVER
NMAKE /C /S /F SERVER.MAK
CD ..\MODULES\CORE_MODULES
CALL MAKE.BAT
CD ..\HTTPD_MODULES
CALL MAKE.BAT
CD ..\..
