# Makefile for NullLogic Groupware server mods

include ../../Rules.mak

all:
	@echo " httpd" ; cd httpd ; $(MAKE)
	@echo " pop3d" ; cd pop3d ; $(MAKE)
	@echo " smtpd" ; cd smtpd ; $(MAKE)
	@echo " smtpq" ; cd smtpq ; $(MAKE)

clean:
	@cd httpd ; $(MAKE) clean
	@cd pop3d ; $(MAKE) clean
	@cd smtpd ; $(MAKE) clean
	@cd smtpq ; $(MAKE) clean
	@rm -f *~
