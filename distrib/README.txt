-------------------------------------------
NullLogic Groupware version 1.2 Readme File
-------------------------------------------

This readme is intended to provide only the most cursory introduction to
NullLogic Groupware.  For more information on installation and configuration
please refer to the included html documentation.

--------
CONTENTS
--------
1. DESCRIPTION
2. INSTALLATION
2a. METHOD OF OPERATION
2b. DATA ACCESS
3. REPORTING A BUG

--------------
1. DESCRIPTION
--------------
NullLogic Groupware is a multi-user web-based groupware package designed for
contact management and event scheduling, which includes additional features
such as private messaging, public discussion forums, shared hyperlinks, file
bases, a web e-mail client, and a simple order processing system.

The main goal of NullLogic Groupware is to make your data accessible to you
from anywhere at any time without the need for special software to be
installed on the client computer.

Linux and Windows are both supported and NullLogic Groupware has been
thoroughly tested with MySQL, PostgreSQL, SQLite and ODBC.

---------------
2. INSTALLATION
---------------
To install NullLogic Groupware, you will need to run 'SETUP.EXE' (Windows)
or 'setup' (Linux).  If any information is required during this process, you
will be prompted for the appropriate information.

Once you have finished running the setup program, you should have a fully
functional installation of NullLogic Groupware.  There are, however, several
different ways of configuring NullLogic Groupware which may be much better
suited to its desired task.

NullLogic Groupware can be configured to use an SQL server such as MySQL
or PostgreSQL.  To initialise, backup, or restore a NullLogic Groupware
database, use the command line tool 'DBUTIL.EXE' or 'dbutil' (on Windows and
Linux respectively).

To log in to NullLogic Groupware for the first time, the username is
'administrator', and the password is 'visual'.  Be sure to change this
password as soon as possible.

-----------------------
2a. METHOD OF OPERATION
-----------------------
If you intend to run NullLogic Groupware in server mode, you will need
to make sure it is in your startup.  On Windows, this should be done
automatically.  On Linux, you will need to call 'rc.groupware' from the
appropriate startup script.

Although no longer supported, the main program can still be run as a CGI.
If you intend to run NullLogic Groupware as a CGI, copy the main NullLogic
Groupware executable (NULLGW-HTTPD.EXE on Windows, nullgw-httpd on Linux)
to a directory which your web server recognizes as being a CGI directory,
and copy the file groupware.cfg to the same directory.  You may want to
rename the CGI to 'GROUPWARE.EXE' or 'groupware.cgi'.  You will also need
to copy the directory called groupware from your htdocs directory to your
web server's base directory.

In both configurations, it may be necessary to ensure the libraries for
your SQL server can be found in your server's library path.

---------------
2b. DATA ACCESS
---------------
The default installation of NullLogic Groupware uses SQLite for database
access.  Although reliable, this configuration may not offer the scalability
or accessibility you require.  In this case, you may want to consider using
MySQL or PostgreSQL instead.

First, you will need to make an account in your SQL server for NullLogic
Groupware.  It is probably wise not to give this account permission to create
databases and users, but it should have permission to create new tables.

After you have configured the new database and user, change the SQL server
type in 'groupware.cfg' to match the appropriate settings.

The command line tool 'dbutil.exe' can be used to initialise the database for
either MySQL or PostgreSQL.  Note that the NullLogic Groupware database must
be created (not the tables, just the bare database) prior to running dbutil
if you are using MySQL or PostgreSQL.  The dbutil program will create and
populate all of the necessary tables.

------------------
3. REPORTING A BUG
------------------
If you believe you have found a bug, the first question is can you reproduce
it?  If you can, then it is really a bug.

Next, please visit http://nullgroupware.sourceforge.net/ and check the forums
to see if anyone else is having the same problem, or if the problem has already
been fixed.

If the bug you just found is not on the list, please send an e-mail to
groupware@nulllogic.com with the Subject line 'NullLogic Groupware Bug'. In the
body of the message please describe the bug, how it can be reproduced, the
full version number (e.g. 1.2.3), the operating system of the server, and the
name and version number of your third party web server (if you are using one).
