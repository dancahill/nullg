#!/bin/sh

# If you're running an AV daemon, you need to add the daemon user to
# the 'nullgw' group.  If you don't, the daemon probably won't have
# permission to read the files you want to scan.

PATH_TO_MKS="/usr/local/mks/mksscan"
PATH_TO_CLAMAV="/usr/bin/clamdscan"

# MKS Antivirus
if [ -x $PATH_TO_MKS ]; then
	$PATH_TO_MKS $1
	if test $? -eq 1 ; then
		exit 2
	fi
fi

# ClamAV
if [ -x $PATH_TO_CLAMAV ]; then
	$PATH_TO_CLAMAV $1
	if test $? -eq 1 ; then
		exit 2
	fi
fi

exit 0
