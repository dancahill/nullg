# Makefile for NullLogic Groupware server mods

include ../Rules.mak

all:
	@echo " srv_httpd" ; cd srv_httpd ; $(MAKE)
	@echo " srv_pop3d" ; cd srv_pop3d ; $(MAKE)
	@echo " srv_smtpd" ; cd srv_smtpd ; $(MAKE)
	@echo " srv_smtpq" ; cd srv_smtpq ; $(MAKE)

clean:
	@cd srv_httpd ; $(MAKE) clean
	@cd srv_pop3d ; $(MAKE) clean
	@cd srv_smtpd ; $(MAKE) clean
	@cd srv_smtpq ; $(MAKE) clean
	@rm -f *~
