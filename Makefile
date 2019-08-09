# Makefile for NullLogic Groupware 1.3.30

all:
	@echo "You should try running ./configure first."

redhat:
	./configure --bindir=/usr/bin --sysconfdir=/etc/nullgw --libdir=/usr/lib/nullgw --localstatedir=/var/lib/nullgw
