# Makefile for NullLogic Groupware mods

include ../Rules.mak

all:
	@echo "server_modules" ; cd server_modules ; $(MAKE)
	@echo "httpd_modules"  ; cd httpd_modules  ; $(MAKE)

clean:
	@cd server_modules ; $(MAKE) clean
	@cd httpd_modules  ; $(MAKE) clean
	@rm -f *~
