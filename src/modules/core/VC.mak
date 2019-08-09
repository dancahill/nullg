# Makefile for NullLogic Groupware server mods
MAKE=NMAKE /C /S /Y /F

all:
	if not exist "..\..\..\obj" mkdir "..\..\..\obj"
	if not exist "..\..\..\obj\modules" mkdir "..\..\..\obj\modules"
	if not exist "..\..\..\obj\modules\core" mkdir "..\..\..\obj\modules\core"
	if not exist "..\..\..\distrib" mkdir "..\..\..\distrib"
	if not exist "..\..\..\distrib\lib" mkdir "..\..\..\distrib\lib"
	if not exist "..\..\..\distrib\lib\core" mkdir "..\..\..\distrib\lib\core"
	@CD GHTTPD
	@$(MAKE) CORE_GHTTPD.MAK
	@CD ..\HTTPD
	@$(MAKE) CORE_HTTPD.MAK
	@CD ..\POP3D
	@$(MAKE) CORE_POP3D.MAK
	@CD ..\SMTPD
	@$(MAKE) CORE_SMTPD.MAK
	@CD ..\SMTPQ
	@$(MAKE) CORE_SMTPQ.MAK
	@CD ..

clean:
	@CD GHTTPD
	@$(MAKE) CORE_GHTTPD.MAK CLEAN
	@CD ..\HTTPD
	@$(MAKE) CORE_HTTPD.MAK CLEAN
	@CD ..\POP3D
	@$(MAKE) CORE_POP3D.MAK CLEAN
	@CD ..\SMTPD
	@$(MAKE) CORE_SMTPD.MAK CLEAN
	@CD ..\SMTPQ
	@$(MAKE) CORE_SMTPQ.MAK CLEAN
	@CD ..
