# Makefile for Sentinel Groupware
MYSQLDEF = -DHAVE_MYSQL
MYSQLINC = -I/usr/local/mysql/include/mysql
MYSQLLIB = -L/usr/local/mysql/lib/mysql -lmysqlclient -lpthread
PGSQLDEF = -DHAVE_PGSQL
PGSQLINC = -I/usr/local/pgsql/include
PGSQLLIB = -L/usr/local/pgsql/lib -lpq -lpthread
CC = gcc
OBJECTS = \
	sql.o main.o common.o files.o html.o http.o md5.o security.o server.o \
	admin.o calendar.o companies.o contacts.o forums.o links.o messages.o \
	orders.o products.o queries.o tasks.o users.o webmail.o

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
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o sentinel-mysql
	@echo Make sentinel-mysql is complete.

pgsql: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o sentinel-pgsql
	@echo Make sentinel-pgsql is complete.

clean:
	rm -f sentinel-mysql sentinel-pgsql *.o *~
