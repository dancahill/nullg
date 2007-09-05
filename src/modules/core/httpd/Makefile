# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = httpd_auth.o httpd_cgi.o httpd_format.o httpd_html.o httpd_http.o httpd_io.o httpd_lang.o httpd_ldir.o httpd_log.o httpd_main.o httpd_modctl.o httpd_nesla.o httpd_sql.o
TARGET  = $(LIB_TARGETDIR)/core/httpd.so

all: core_httpd

core_httpd: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
