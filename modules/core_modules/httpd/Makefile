# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(LIB_CFLAGS)
OBJECTS = httpd_auth.o httpd_conf.o httpd_format.o httpd_html.o httpd_http.o httpd_io.o httpd_lang.o httpd_log.o httpd_main.o httpd_modctl.o
TARGET  = $(LIB_TARGETDIR)/core/httpd.so

all: core_httpd

core_httpd: $(OBJECTS)
	$(CC) $(LIB_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
