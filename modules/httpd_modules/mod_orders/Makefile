# Makefile for NullLogic Groupware module

include ../../../Rules.mak

CFLAGS  = $(MOD_CFLAGS)
OBJECTS = mod_orders_db.o mod_orders_items.o mod_orders_main.o mod_orders_products.o
TARGET  = $(MOD_TARGETDIR)/httpd/mod_orders.so

all: mod_orders

mod_orders: $(OBJECTS)
	$(CC) $(MOD_LDFLAGS) -o $(TARGET) $(OBJECTS)

clean:
	rm -f $(TARGET) $(OBJECTS) *~
