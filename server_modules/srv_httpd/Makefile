# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(SRVMOD_CFLAGS)
OBJECTS = auth.o format.o html.o http.o io.o log.o main.o modctl.o
TARGET  = $(SRVMOD_TARGETDIR)/srv_httpd.so

all: srv_http

srv_http: $(OBJECTS)
	$(CC) $(SRVMOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
