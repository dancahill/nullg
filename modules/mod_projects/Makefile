# Makefile for NullLogic Groupware module

include ../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_projects_db.o mod_projects_main.o
TARGET  = $(MOD_TARGETDIR)/mod_projects.so

all: mod_projects

mod_projects: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
