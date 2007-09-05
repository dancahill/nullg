# Makefile for NullLogic GroupServer module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_admin_config.o mod_admin_db.o mod_admin_domains.o mod_admin_groups.o mod_admin_logs.o mod_admin_main.o mod_admin_syscheck.o mod_admin_time.o mod_admin_users.o mod_admin_zones.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_admin.so

all: mod_admin

mod_admin: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
