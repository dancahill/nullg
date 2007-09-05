# Makefile for NullLogic GroupServer 1.3.30

all:
	@echo "You should try running ./configure first."

redhat:
	./configure --bindir=/usr/bin --sysconfdir=/etc/nullgs --libdir=/usr/lib/nullgs --localstatedir=/var/lib/nullgs

distclean: all
