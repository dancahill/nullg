norm: clean
	rm -f *.o
	cc -s -O2 -I ../../include http_crashtest.c -o testhttp -lpthread
	cc -s -O2 -I ../../include mail_basher.c -o testmail -lpthread

wssl: clean
	rm -f *.o
	cc -s -O2 -I ../../include -DHAVE_SSL http_crashtest.c -o testhttps -lpthread -lssl -lcrypto
	cc -s -O2 -I ../../include -DHAVE_SSL mail_basher.c -o testmails -lpthread -lssl -lcrypto

clean:
	rm -f testmail testmails testhttp testhttps *.o *~
