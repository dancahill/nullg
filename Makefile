# Makefile for NullLogic GroupServer 1.3.30

all:
	@echo "You should try running ./configure first."

redhat:
	./configure --bindir=/usr/bin --sysconfdir=/etc/nullg --libdir=/usr/lib/nullg --localstatedir=/var/lib/nullg

local:
	./configure --bindir=/usr/local/nullg/bin --sysconfdir=/usr/local/nullg/etc --libdir=/usr/local/nullg/lib --localstatedir=/usr/local/nullg/var

distclean: all
