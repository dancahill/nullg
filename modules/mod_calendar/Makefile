# Makefile for NullLogic Groupware module

TOPDIR=../
include $(TOPDIR)Rules.mak

OBJECTS = mod_calendar_assign.o mod_calendar_db.o mod_calendar_main.o mod_calendar_mini.o
TARGET  = $(TARGETDIR)/mod_calendar.so

all: mod_calendar

mod_calendar: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
