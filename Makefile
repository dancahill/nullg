# Makefile for NullLogic Groupware

all: _dbutil _groupware

_dbutil:
	cd dbutil;make

_groupware:
	cd groupware;make

clean:
	cd dbutil;make clean
	cd groupware;make clean
	./lxmake clean
	rm -f distrib/bin/dbutil distrib/bin/groupware

install:
	@make
	@/etc/rc.d/rc.groupware stop
	@cp -av distrib/bin/groupware /usr/local/groupware/bin/groupware
	@cp -av distrib/bin/dbutil /usr/local/groupware/bin/dbutil
	@/etc/rc.d/rc.groupware start

dist:
	@./lxmake clean
	@make
	@./lxmake pack
