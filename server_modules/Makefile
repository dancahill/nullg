# Makefile for NullLogic Groupware server mods

all:
	@echo " srv_httpd" ; cd srv_httpd ; $(MAKE)
	@echo " srv_pop3d" ; cd srv_pop3d ; $(MAKE)
	@echo " srv_smtpd" ; cd srv_smtpd ; $(MAKE)

clean:
	@cd srv_httpd ; $(MAKE) clean
	@cd srv_pop3d ; $(MAKE) clean
	@cd srv_smtpd ; $(MAKE) clean
	@rm -f *~
