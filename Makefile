# Makefile for NullLogic Groupware

all: _dbutil _groupware _modules
	@cp -av contrib/libsqlite.so distrib/lib/libsqlite.so
	@chmod 755 distrib/lib/libsqlite.so

_dbutil:
	@cd dbutil;make

_groupware:
	@cd server;make

_modules:
	@cd modules;make

clean:
	@chmod 0755 dedos lxmake
	@cd dbutil;make clean
	@cd server;make clean
	@cd modules;make clean
	@./lxmake clean
	rm -f distrib/bin/dbutil distrib/bin/groupware *~

install:
	@make
	@/etc/rc.d/rc.groupware stop
	@cp -av distrib/bin/groupware /usr/local/groupware/bin/groupware
	@cp -av distrib/bin/dbutil /usr/local/groupware/bin/dbutil
	@cp -av distrib/lib/* /usr/local/groupware/lib/
	@/etc/rc.d/rc.groupware start

linecount:
	wc `find -name *.[ch]`

dist:
	@chmod 0755 lxmake
	@./lxmake clean
	@make
	@./lxmake pack
