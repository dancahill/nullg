# MAKEFILE FOR NESLA CGI

include ../../config.mak

#CC      = gcc
#CFLAGS  = -Wall -g -O2 -I../../../include
CFLAGS  += -I../../../include
LD_FLAGS = -L../../../lib -lnsp -lnsp_base $(LDFLAGS)
#The following LDFLAGS might be needed for Solaris
#LDFLAGS = -lsocket -lnsl
OBJECTS = cgi.o config.o main.o
TARGET = ../../../bin/nsp.cgi

all: nsp.cgi

nsp.cgi: $(OBJECTS)
	$(CC) -o $(TARGET) -s $(OBJECTS) $(LD_FLAGS)

clean:
	@rm -f $(TARGET) $(OBJECTS) *~
