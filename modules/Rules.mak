# Rules.mak for NullLogic Groupware modules

CC        = gcc
CFLAGS    = -Wall -O2 -fPIC -I../include -I../../include
LDFLAGS   = -s -shared
TARGETDIR = ../../distrib/lib
