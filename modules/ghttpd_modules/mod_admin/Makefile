# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_admin_config.o mod_admin_db.o mod_admin_domains.o mod_admin_groups.o mod_admin_lang.o mod_admin_logs.o mod_admin_main.o mod_admin_syscheck.o mod_admin_users.o mod_admin_zones.o
TARGET  = $(MOD_TARGETDIR)/ghttpd/mod_admin.so

all: mod_admin

mod_admin: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
