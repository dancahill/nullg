all:
	cc -s -O2 http_crashtest.c -o test -lpthread
	rm -f *.o
	cc -s -O2 -DHAVE_SSL http_crashtest.c -o testssl -lpthread -lssl -lcrypto

clean:
	rm -f test testssl *.o *~
