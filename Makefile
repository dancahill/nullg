# Makefile for NullLogic Groupware

all: _dbutil _groupware _modules sqlite
	@cp -av other/rc.groupware distrib/bin/rc.groupware
	@cp -av other/setup distrib/setup

sqlite:
	@echo ""
	@echo "Compiling sqlite"
	@echo ""
	@cd contrib;./make_sqlite build

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
	@cd contrib;./make_sqlite clean
	@./lxmake clean
	rm -f distrib/bin/* distrib/lib/* *~

install:
	@make
	@cd distrib;./setup

linecount:
	wc `find -name *.[ch]`

dist:
	@chmod 0755 lxmake
	@./lxmake clean
	@make
	@./lxmake pack
