# Makefile for NullLogic Groupware HTTPd

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = auth.o config.o format.o html.o http.o io.o log.o main.o md5.o \
	modctl.o sanity.o server.o sql.o
TARGET  = $(BIN_TARGETDIR)/nullgw-httpd

all: nullgw-httpd

nullgw-httpd: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS_T)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
