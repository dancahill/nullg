#!/bin/sh
# This is a simple script to scan e-mail for spam and virii.  Depending on
# what is found in the file, this script should return these values;
#
#   0 = CLEAN
#   1 = SPAM
#   2 = VIRUS
#   3 = SPAM + VIRUS
#
# Any value other than 1, 2, or 3 will be treated the same as a value of 0.

SPAM=0
VIRUS=0

# SpamAssassin
# If you don't have SpamAssassin installed, try this;
#   perl -MCPAN -e 'install Mail::SpamAssassin'

if [ -x /usr/bin/spamc ]; then
	/usr/bin/spamc < $1 > $1.tmp
	rm -f $1
	mv $1.tmp $1
	if test `grep -c "X-Spam-Status: Yes" $1` -gt 0 ; then
		SPAM=1
	fi
fi

# Bogofilter
if [ -x /usr/local/bin/bogofilter ]; then
	/usr/local/bin/bogofilter -I $1 -O $1.tmp -p -d /usr/local/nullgroupware/var/bogofilter
	rm -f $1
	mv $1.tmp $1
	if test `grep -c "X-Bogosity: Yes" $1` -gt 0 ; then
		SPAM=1
	fi
fi
chmod 600 $1

# If we have a script for virus scanning, run it.
if [ -x /usr/local/nullgroupware/bin/scan_file.sh ]; then
	/usr/local/nullgroupware/bin/scan_file.sh $1
	if test $? -eq 2 ; then
		if test $SPAM -eq 1 ; then
			exit 3
		else
			exit 2
		fi
	else
		if test $SPAM -eq 1 ; then
			exit 1
		fi
	fi
else
	if test $SPAM -eq 1 ; then
		exit 1
	fi
fi

exit 0
