@ECHO OFF
mkdir ..\..\bin
mkdir ..\..\lib
mkdir ..\..\obj
COPY \NullLogic\gw\libs\libsqlite3.dll ..\..\lib
COPY \NullLogic\gw\libs\libeay32.dll ..\..\lib
COPY \NullLogic\gw\libs\ssleay32.dll ..\..\lib
@ECHO libnesla
CD ..\LIBNESLA
NMAKE /C /S /F MAKEFILE.VC
@ECHO libngs
CD ..\LIBNGS
NMAKE /C /S /F MAKEFILE.VC
@ECHO confutil
CD ..\CONFUTIL
NMAKE /C /S /F MAKEFILE.VC
@ECHO dbutil
CD ..\DBUTIL
NMAKE /C /S /F MAKEFILE.VC
@ECHO server
CD ..\SERVER
NMAKE /C /S /F MAKEFILE.VC
CD ..
@ECHO gsmon
CD WIN\GSMON
NMAKE /C /S /F GSMON.MAK
CD ..\..


@ECHO core_modules

@ECHO  httpd
CD MODULES\CORE\HTTPD
NMAKE /C /S /F MAKEFILE.VC
@ECHO  pop3d
CD ..\..\..\MODULES\CORE\POP3D
NMAKE /C /S /F MAKEFILE.VC
@ECHO  smtpd
CD ..\..\..\MODULES\CORE\SMTPD
NMAKE /C /S /F MAKEFILE.VC
@ECHO  smtpq
CD ..\..\..\MODULES\CORE\SMTPQ
NMAKE /C /S /F MAKEFILE.VC

@ECHO httpd_modules

rem @ECHO  mod_admin
rem CD ..\..\..\MODULES\HTTPD\MOD_ADMIN
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_bookmarks
rem CD ..\..\..\MODULES\HTTPD\MOD_BOOKMARKS
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_calendar
rem CD ..\..\..\MODULES\HTTPD\MOD_CALENDAR
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_calls
rem CD ..\..\..\MODULES\HTTPD\MOD_CALLS
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_cgi
rem CD ..\..\..\MODULES\HTTPD\MOD_CGI
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_contacts
rem CD ..\..\..\MODULES\HTTPD\MOD_CONTACTS
rem NMAKE /C /S /F MAKEFILE.VC
@ECHO  mod_email
CD ..\..\..\MODULES\HTTPD\MOD_EMAIL
NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_files
rem CD ..\..\..\MODULES\HTTPD\MOD_FILES
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_finance
rem CD ..\..\..\MODULES\HTTPD\MOD_FINANCE
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_forums
rem CD ..\..\..\MODULES\HTTPD\MOD_FORUMS
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_messages
rem CD ..\..\..\MODULES\HTTPD\MOD_MESSAGES
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_notes
rem CD ..\..\..\MODULES\HTTPD\MOD_NOTES
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_profile
rem CD ..\..\..\MODULES\HTTPD\MOD_PROFILE
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_projects
rem CD ..\..\..\MODULES\HTTPD\MOD_PROJECTS
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_searches
rem CD ..\..\..\MODULES\HTTPD\MOD_SEARCHES
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_spellcheck
rem CD ..\..\..\MODULES\HTTPD\MOD_SPELLCHECK
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_tasks
rem CD ..\..\..\MODULES\HTTPD\MOD_TASKS
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_weblog
rem CD ..\..\..\MODULES\HTTPD\MOD_WEBLOG
rem NMAKE /C /S /F MAKEFILE.VC
rem @ECHO  mod_xmlrpc
rem CD ..\..\..\MODULES\HTTPD\MOD_XMLRPC
rem NMAKE /C /S /F MAKEFILE.VC

CD ..\..\..\win
