# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_calendar_assign.o mod_calendar_availmap.o mod_calendar_db.o mod_calendar_lang.o mod_calendar_listd.o mod_calendar_listw.o mod_calendar_listm.o mod_calendar_listy.o mod_calendar_main.o mod_calendar_mini.o mod_calendar_vcal.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_calendar.so

all: mod_calendar

mod_calendar: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
