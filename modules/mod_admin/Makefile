# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS  = mod_admin_config.o mod_admin_db.o mod_admin_groups.o mod_admin_logs.o mod_admin_main.o mod_admin_syscheck.o mod_admin_users.o
TARGET   = $(TARGETDIR)/mod_admin.so

all: mod_admin

mod_admin: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
