# Makefile for NullLogic Groupware SMTPd

include ../Rules.mak

CFLAGS  = $(BIN_CFLAGS)
OBJECTS = auth.o config.o format.o io.o log.o main.o md5.o sanity.o server.o \
	smtp.o smtpclient.o sql.o
TARGET  = $(BIN_TARGETDIR)/nullgw-smtpd

all: nullgw-smtpd

nullgw-smtpd: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(BIN_LDFLAGS_T)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
