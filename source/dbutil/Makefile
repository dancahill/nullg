# Makefile for Sentinel Groupware dbutil
MYSQLDEF = -DHAVE_MYSQL
MYSQLINC = -I/usr/local/mysql/include/mysql
MYSQLLIB = -L/usr/local/mysql/lib/mysql -lmysqlclient
PGSQLDEF = -DHAVE_PGSQL
PGSQLINC = -I/usr/local/pgsql/include
PGSQLLIB = -L/usr/local/pgsql/lib -lpq
CC = gcc
OBJECTS = \
	sql.o config.o dump.o init.o main.o restore.o

ifeq ($(MAKECMDGOALS), mysql)
	CFLAGS  = -Wall -O2 -s $(MYSQLDEF) $(MYSQLINC)
	LDFLAGS = $(MYSQLLIB)
endif
ifeq ($(MAKECMDGOALS), pgsql)
	CFLAGS  = -Wall -O2 -s $(PGSQLDEF) $(PGSQLINC)
	LDFLAGS = $(PGSQLLIB)
endif

all:
	@echo
	@echo "make both|mysql|pgsql"
	@echo

both:
	rm -f *.o
	make mysql
	rm -f *.o
	make pgsql
	rm -f *.o

mysql: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o dbutil-mysql
	@echo Make dbutil-mysql is complete.

pgsql: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o dbutil-pgsql
	@echo Make dbutil-pgsql is complete.

clean:
	rm -f dbutil-mysql dbutil-pgsql *.o *~
