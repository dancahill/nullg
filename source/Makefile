# Makefile for Sentinel Groupware

all:
	cd dbutil;make both
	cd sentinel;make both
	@echo
	@echo Make complete.
	@echo

dbutil:
	cd dbutil;make both

sentinel:
	cd sentinel;make both

clean:
	cd dbutil;make clean
	cd sentinel;make clean

dist:
	cp -av dbutil/dbutil-mysql ../linux/bin/dbutil-mysql
	cp -av dbutil/dbutil-pgsql ../linux/bin/dbutil-pgsql
	cp -av sentinel/sentinel-mysql ../linux/bin/sentinel-mysql
	cp -av sentinel/sentinel-pgsql ../linux/bin/sentinel-pgsql
	@echo
	@echo Sentinel Groupware dist is complete.
	@echo
