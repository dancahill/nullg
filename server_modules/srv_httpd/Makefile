# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = auth.o conf.o format.o html.o http.o io.o log.o main.o modctl.o
TARGET  = $(LIB_TARGETDIR)/srv_httpd.so

all: srv_http

srv_http: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
