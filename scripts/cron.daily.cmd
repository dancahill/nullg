@ECHO OFF
REM This is a fairly basic script to make a backup of your database, e-mail,
REM files, etc.  It might be good policy to run a cron job like this daily.

SET BASEDIR=C:\Program Files\NullLogic Groupware
SET CURDATE=%DATE:~10,4%-%DATE:~4,2%-%DATE:~7,2%

mkdir "%BASEDIR%\var\backup" 2> NUL
chdir "%BASEDIR%\bin"
dbutil.exe dump "..\backup-%CURDATE%"
cd "%BASEDIR%"
gzip "backup-%CURDATE%"
echo Archiving backup tarball...
DEL var\backup\backup-%CURDATE%.tgz 2> NUL
tar zcf backup-%CURDATE%.tgz backup-%CURDATE%.gz cgi-bin etc var --exclude=var/backup/*
DEL backup-%CURDATE%.gz 2> NUL
MOVE backup-%CURDATE%.tgz var\backup
echo done.
