# Makefile for NullLogic Groupware

include Rules.mak

all: _dbutil _httpd _pop3d _smtpd _modules _sqlite
	@cp -p other/rc.groupware distrib/bin/rc.groupware
	@cp -p other/configure distrib/bin/configure
	@cp -p other/setup distrib/setup

linux:
	@ln -sf Rules/Linux Rules.mak
	@$(MAKE) all

oldlinux:
	@ln -sf Rules/Linux2.0 Rules.mak
	@$(MAKE) all

freebsd:
	@ln -sf Rules/FreeBSD Rules.mak
	@$(MAKE) all

_sqlite:
	@echo ""
	@echo "Compiling SQLite"
	@echo ""
	@cd contrib;./make_sqlite build

_dbutil:
	@echo "dbutil"
	@cd dbutil;$(MAKE)

_httpd:
	@echo "httpd"
	@cd httpd;$(MAKE)

_pop3d:
	@echo "pop3d"
	@cd pop3d;$(MAKE)

_smtpd:
	@echo "smtpd"
	@cd smtpd;$(MAKE)

_modules:
	@echo "modules"
	@cd modules;$(MAKE)

clean:
	@echo -n "Cleaning..."
	@chmod 0755 other/lxtool
	@cd dbutil;$(MAKE) clean
	@cd httpd;$(MAKE) clean
	@cd pop3d;$(MAKE) clean
	@cd smtpd;$(MAKE) clean
	@cd modules;$(MAKE) clean
	@./other/lxtool clean
	@rm -f distrib/bin/* distrib/lib/* *~
	@echo "done."

mrclean: clean
	@ln -sf Rules/Linux Rules.mak
	@cd contrib;./make_sqlite clean

install: all
	@cd distrib;./setup

install2: all
	/usr/local/nullgroupware/bin/rc.groupware stop
	@cd distrib;./setup
	/usr/local/nullgroupware/bin/rc.groupware start

wc:
	wc `find -name *.[ch]`

dist:
	@chmod 0755 other/lxtool
	@./other/lxtool clean
	@$(MAKE)
	@./other/lxtool pack

slackdist:
	@chmod 0755 other/lxtool
	@./other/lxtool clean
	@$(MAKE)
	@./other/lxtool slackpack
