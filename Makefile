# Makefile for NullLogic Groupware

include Rules.mak

all: _dbutil _server _server_modules _modules _sqlite
	@cp -p other/rc.groupware distrib/bin/rc.groupware
	@cp -p other/cron.daily distrib/bin/cron.daily
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
	@make all

_dbutil:
	@echo "dbutil"
	@cd dbutil;$(MAKE)

_server:
	@echo "server"
	@cd server;$(MAKE)

_server_modules:
	@echo "server_modules"
	@cd server_modules;$(MAKE)

_modules:
	@echo "modules"
	@cd modules;$(MAKE)

_sqlite:
	@echo "sqlite"
	@cd contrib;./make_sqlite build

clean:
	@echo -n "Cleaning..."
	@chmod 0755 other/lxtool
	@cd dbutil;$(MAKE) clean
	@cd server;$(MAKE) clean
	@cd server_modules;$(MAKE) clean
	@cd modules;$(MAKE) clean
	@./other/lxtool clean
	@rm -f distrib/bin/* distrib/lib/* *~
	@echo "done."

mrclean: clean
	@ln -sf Rules/Linux Rules.mak
	@cd contrib;./make_sqlite clean
	@rm -f `find -name *~`

install: all
	@cd distrib;./setup

install2: all
	/etc/rc.d/init.d/nullgw stop
	@cd distrib;./setup
	/etc/rc.d/init.d/nullgw start

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
