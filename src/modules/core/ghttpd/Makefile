# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = ghttpd_auth.o ghttpd_conf.o ghttpd_format.o ghttpd_html.o ghttpd_http.o ghttpd_io.o ghttpd_lang.o ghttpd_log.o ghttpd_main.o ghttpd_modctl.o ghttpd_sql.o
TARGET  = $(LIB_TARGETDIR)/core/ghttpd.so

all: core_ghttpd

core_ghttpd: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
