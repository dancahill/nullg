# NullLogic Embedded Scripting Language
# GCC Makefile
# run 'make'

CC      = gcc
MAKE    = make
NSPCLI  = ./bin/nsp

#.SILENT:

all: _all
	@./bin/nsp -e "global quiet=true;" -f tests/test1.ns
#	@echo "	Run 'make targets' to list more options."
#	@echo

_all: dirs libnsp cli deps static_libs shared_libs

dirs:
	@mkdir -p bin
	@mkdir -p lib
	@mkdir -p lib/shared
	@mkdir -p int

libnsp: dirs
	@cd src/libnsp     && $(MAKE) && cd ../..

deps: cli
	@cd src/libs && ../../$(NSPCLI) -b autoconf.ns gcc posix && cd ../..

reconf: cli
	@rm -f include/nsp/config*.h src/config*.mak src/config*.ns
	@cd src/libs && ../../$(NSPCLI) -b autoconf.ns gcc posix && cd ../..

# rules for libs

static_libs: deps
	@cd src/libs/base    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/crypto  && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/data    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/fbsql   && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/ldap    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/mysql   && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/net     && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/odbc    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/pgsql   && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/ssh2    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@cd src/libs/sqlite  && ../../../$(NSPCLI) -b -e "global _LINKAGE_='static';include('Makefile.ns');" && cd ../../..
	@echo

static: static_libs

shared_libs: deps
	@cd src/libs/base     && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/crypto   && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/data     && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/fbsql    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/ldap     && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/mysql    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/net      && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/odbc     && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/pgsql    && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/sqlite   && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
	@cd src/libs/ssh2     && ../../../$(NSPCLI) -b -e "global _LINKAGE_='shared';include('Makefile.ns');" && cd ../../..
#	@./bin/nsp -e "dl.load('./lib/shared/libnsp_test.so');print('\n'+dltest()+'\n');"

shared: shared_libs

# rules for hosts
cgi: _all
	@cd src/hosts/cgi     && $(MAKE) && cd ../../..

cli: libnsp
	@cd src/hosts/cli     && $(MAKE) -s && cd ../../..
	@echo
	@echo -n "Interpreter built"
	@./bin/nsp -b -e "var x='gnikrow dna ';for (i=string.len(x);i>=0;i--) print(string.sub(x, i, 1));"
	@echo
	@echo

# rules for everything else
clean:
	@touch src/config.mak
	@cd src/libnsp        && $(MAKE) -s clean && cd ../..
	@cd src/hosts/cgi     && $(MAKE) -s clean && cd ../../..
	@cd src/hosts/cli     && $(MAKE) -s clean && cd ../../..
#	@cd src/libs/base     && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/crypto   && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/data     && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/dl       && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/mysql    && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/net      && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/odbc     && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/pgsql    && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/sqlite   && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/ssh2     && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@cd src/libs/winapi   && $(NSPCLI) Makefile.ns clean && cd ../../..
#	@rm -rf bin int lib `find -name *.o`
	@rm -rf bin int lib obj Debug

distclean: clean
	@chown root.root * -R
	@setfacl -b `find . -name "*"`
	@chmod 755 `find . -type d`
	@chmod 644 `find . -type f`
	@chmod 755 `find . -type f \( -name "*.ns" \)`
#	@setfacl -b `find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" \)`
	@rm -rf .vs
	@rm -f NSP.ncb NSP.suo NSP.v12.suo
	@rm -f include/nsp/config*.h src/config*.mak src/config*.ns
	@rm -f src/preconf.mak
	@rm -f `find . -name *.null.user`

install: _all
	@cp -pR bin/nsp /usr/bin/nsp
	mkdir -p /usr/local/man/man1/
	@cp -pR doc/nsp.1 /usr/local/man/man1/nsp.1
	@mkdir -p /usr/lib/nsp
	@cp -pR lib/*.a /usr/lib/nsp/
	@cp -pR lib/shared/*.so /usr/lib/nsp/
	@cp -pR src/libs/preload.ns /usr/lib/nsp/preload-default.ns

# minimal one-line command for making a minimal unoptimized binary
hack:
	$(CC) -I./include src/libnsp/*.c src/hosts/cli/main.c -o bin/nsp_h

tcchack:
	tcc -I./include -o nsp src/libnsp/*.c src/hosts/cli/main.c
	./nsp -e "var x='\n\ngnikrow dna ';print('Standard Interpreter built');for (i=string.len(x);i>=0;i--) print(string.sub(x, i, 1));"

gprof:
#	time ./bin/nsp_d -e "i=\"\"; for (j=0;j<1000000;j++) { i+=j; }"
	@cd src/libnsp    && $(MAKE) clean && $(MAKE) -f Makefile.gprof && cd ../..
	@cd src/hosts/cli && $(MAKE) clean && $(MAKE) -s -f Makefile.gprof && cd ../../..
	./bin/nsp_d -e "i=0; for (j=0;j<100000;j++) { i+=j; }"
	gprof -c -z ./bin/nsp_d > gprof.txt
	rm `find . -name gmon.out`

targets:
	@echo
	@echo "useful targets include:"
	@echo "	all         - Builds all basic libraries and the CLI (default)"
	@echo "	cli         - Builds the Command Line Interpreter"
	@echo "	shared_libs - Builds all extension modules as shared libraries"
	@echo "	static_libs - Builds all extension modules as static libraries"
	@echo "	cgi         - Builds the CGI host program"
	@echo "	install     - Installs the cli binary to /usr/bin/nsp"
	@echo "	cgiinstall  - Installs the cgi binary to /var/www/cgi-bin/nsp.cgi"
	@echo "	test        - Runs a suite of test scripts"
	@echo "	test2       - Runs a more aggressive version of the first test"
	@echo

cgiinstall: cgi
	@mkdir -p /var/www/cgi-bin
	cp -pR bin/nsp.cgi /var/www/cgi-bin/nsp.cgi

test:
	@./bin/nsp -f tests/test1.ns

test2:
	@./bin/nsp -f tests/test2.ns

# the rest are more useful to me than they are to you

ver:
	@make distclean
	@joe `grep -lR "0\.9\." *`
	@rm `find -name *~`

showbug:
	@./bin/nsp -d tests/test1.ns

debug:
	@gdb --args ./bin/nsp_d tests/test1.ns

strace:
	@strace ./bin/nsp_d tests/test1.ns

valgrind:
	@valgrind -v --leak-check=full --leak-resolution=high --show-reachable=yes ./bin/nsp_d -s -f tests/test1.ns

datestamp:
#	cd scripts && touch --date="`date +%Y-%m-%d` 01:00:00" `find *` && cd ..
#	touch --date="`date +'%Y-%m-%d %H:%M:%S'`" `find *`
	touch --date="`date +'%Y-%m-%d %H:00:00'`" `find *`

wc:
	@wc `find . -name *.[ch]`
	@wc src/libnsp/*.c

#docs:
#	@cd doc/syntax && $(NSPCLI) syntax.ns && cd ../..

time:
	@$(NSPCLI) -b -f tests/speed1.ns

timex:
	@$(NSPCLI) -b -s -e "for (i=1;i<100001;i++) { if (i%20000==0) { print(i/20000); } }; print('\r\n',runtime(),'\r\n');"

valtime:
	@valgrind -v --leak-check=full --leak-resolution=high --show-reachable=yes $(NSPCLI)_d -e "for (i=1;i<100001;i++) { if (i%20000==0) { print(i/20000); } }"

calltest:
	time make timex > x1.txt && sort < x1.txt > x2.txt && uniq -c < x2.txt > x3.txt && sort -n < x3.txt > x4.txt
