# Makefile for NullLogic Groupware httpd mods
MAKE=NMAKE /C /S /Y /F

all:
	if not exist "..\..\..\obj" mkdir "..\..\..\obj"
	if not exist "..\..\..\obj\modules" mkdir "..\..\..\obj\modules"
	if not exist "..\..\..\obj\modules\httpd" mkdir "..\..\..\obj\modules\httpd"
	if not exist "..\..\..\distrib" mkdir "..\..\..\distrib"
	if not exist "..\..\..\distrib\lib" mkdir "..\..\..\distrib\lib"
	if not exist "..\..\..\distrib\lib\httpd" mkdir "..\..\..\distrib\lib\httpd"
	@CD MOD_CGI
	@$(MAKE) MOD_CGI.MAK
	@CD ..

clean:
	@CD MOD_CGI
	@$(MAKE) MOD_CGI.MAK CLEAN
	@CD ..
