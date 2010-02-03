# Makefile for NullLogic GroupServer 1.3.30

all:
	@echo "You should try running ./configure first."

redhat:
	./configure --bindir=/usr/bin --sysconfdir=/etc/nullsd --libdir=/usr/lib/nullsd --localstatedir=/var/lib/nullsd

distclean: all
