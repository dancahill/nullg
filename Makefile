# Makefile for NullLogic Groupware

all: _dbutil _groupware _modules
	@cp -av other/sqlite/sqlite.so distrib/lib/sqlite.so
	@chmod 755 distrib/lib/sqlite.so

_dbutil:
	@cd dbutil;make

_groupware:
	@cd server;make

_modules:
	@cd modules;make

clean:
	@chmod 0755 fromdos lxmake
	@./fromdos
	@chmod 0755 fromdos lxmake
	@cd dbutil;make clean;../fromdos
	@cd server;make clean;../fromdos
	@cd modules;make clean;../fromdos
	@cd include;../fromdos
	@./lxmake clean
	rm -f distrib/bin/dbutil distrib/bin/groupware *~

install:
	@make
	@/etc/rc.d/rc.groupware stop
	@cp -av distrib/bin/groupware /usr/local/groupware/bin/groupware
	@cp -av distrib/bin/dbutil /usr/local/groupware/bin/dbutil
	@cp -av distrib/lib/* /usr/local/groupware/lib/
	@/etc/rc.d/rc.groupware start

dist:
	@chmod 0755 lxmake
	@./lxmake clean
	@make
	@./lxmake pack
