# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_tasks_db.o mod_tasks_main.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_tasks.so

all: mod_tasks

mod_tasks: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
