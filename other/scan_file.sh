#!/bin/sh

if [ -x /usr/local/mks/mksscan ]; then
	/usr/local/mks/mksscan $1
	if test $? -eq 1 ; then
		exit 2
	fi
fi
exit 0
