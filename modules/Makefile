# Makefile for NullLogic Groupware mods

include ../Rules.mak

all:
	@echo "core_modules"  ; cd core_modules  ; $(MAKE)
	@echo "httpd_modules" ; cd httpd_modules ; $(MAKE)

clean:
	@cd core_modules  ; $(MAKE) clean
	@cd httpd_modules ; $(MAKE) clean
	@rm -f *~
