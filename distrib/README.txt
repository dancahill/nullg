------------------------------------------
Sentinel Groupware version 1.0 Readme File
------------------------------------------
(c) Copyright NullLogic, 2000-2001

This readme is intended to provide only the most cursory introduction to
Sentinel Groupware.  For more information on installation and configuration
please refer to the included html documentation.

--------
CONTENTS
--------
1. DESCRIPTION
2. INSTALLATION
2a. METHOD OF OPERATION
2b. DATA ACCESS
2c. OTHER CUSTOMIZATION
3. BUGS, LIMITATIONS, AND WORKAROUNDS
4. REPORTING A BUG

--------------
1. DESCRIPTION
--------------
Sentinel Groupware is a multi-user web-based groupware package designed for
contact management and event scheduling, which includes additional features
such as private messaging, public discussion forums, shared hyperlinks, file
bases, a web e-mail client, and a simple order processing system.

The main goal of Sentinel Groupware is to make your data accessible to you
from anywhere at any time without the need for special software to be
installed on the client computer.

Linux and Windows are both supported and Sentinel Groupware has been
thoroughly tested with MySQL, PostgreSQL and ODBC.

---------------
2. INSTALLATION
---------------
To install Sentinel Groupware, you will need to run 'SETUP.EXE' (Windows) or
'setup' (Linux).  If any information is required during this process, you will
be prompted for the appropriate information.

Once you have finished running the setup program, you should have a fully
functional installation of Sentinel Groupware.  There are, however, several
different ways of configuring Sentinel which may be much better suited to its
desired task.

Sentinel Groupware can be configured to run either as a standalone server or
as a CGI with Apache, IIS, or almost any other web server.

Sentinel Groupware can also be configured to use an SQL server such as MySQL
or PostgreSQL.  To initialise, backup, or restore a Sentinel Groupware
database, use the command line tool 'DBUTIL.EXE' or 'dbutil' (on Windows and
Linux respectively).

To log in to Sentinel Groupware for the first time, the username is
'administrator', and the password is 'visual'.  Be sure to change this
password as soon as possible.

-----------------------
2a. METHOD OF OPERATION
-----------------------
If you intend to run Sentinel as a standalone server, you may want to create a
shortcut to Sentinel in your startup directory.

If you intend to run Sentinel Groupware as a CGI, simply copy the main Sentinel
Groupware executable (SENTINEL.EXE on Windows, sentinel on Linux) to a directory
which your web server recognizes as being a CGI directory, and copy the file
sentinel.cfg to the same directory.  You will also need to copy the directory
called sentinel from your http directory to your web server's base directory.
On Windows systems, it may also be necessary to copy LIBMYSQL.DLL and LIBPQ.DLL
to your windows\system directory.

---------------
2b. DATA ACCESS
---------------
Although the default Windows installation uses ODBC for database access, this
configuration is the slowest and least flexible.  It is recommended that you
consider using MySQL or PostgreSQL instead.

First, you will need to make an account in your SQL server for Sentinel.  It
is probably wise not to give this account permission to create databases and
users, but Sentinel Groupware should have permission to create new tables.

The command line tool 'dbutil.exe' can be used to initialise the database for
either MySQL or PostgreSQL.  Note that the Sentinel Groupware database must be
created (not the tables, just the bare database) prior to running dbutil if
you are using MySQL or PostgreSQL.  The dbutil program will create and
populate all of the necessary tables.

After you have configured the new database, change the SQL server type in the
system configuration to match the appropriate choice.

-----------------------
2c. OTHER CUSTOMIZATION
-----------------------

TAXES

The default taxes, PST (Provincial Sales Tax) and GST (Goods and Services Tax)
may have the wrong percentages for your region, or may be entirely the wrong
taxes for your country. The names and default percentages of both taxes can
be changed by modifying the 'gw_dbinfo' table with a query similar to the
following:
   UPDATE gw_dbinfo SET tax1name='TAX1', tax2name='TAX2', tax1percent='0.05', tax2percent='0.04';

-------------------------------------
3. BUGS, LIMITATIONS, AND WORKAROUNDS
-------------------------------------
If you are warned when starting the server that you may need to update your
MDAC drivers, the you can download the MDAC drivers update directly from
Microsoft's web site.  You may also need to update your JET drivers.
The Microsoft Data Access Components are available at:
	http://www.microsoft.com/data/download_260rtm.htm
The Microsoft JET Drivers are available at:
	http://www.microsoft.com/data/download_Jet4SP3.htm

------------------
4. REPORTING A BUG
------------------
If you believe you have found a bug, the first question is can you reproduce
it?  If you can, then it is really a bug.

Next, please visit http://www.nulllogic.com/groupware/bugs.html for a
list of already corrected bugs.

If the bug you just found is not on the list, please send an e-mail to
nulllogic42@yahoo.com with the Subject line 'Sentinel Groupware Bug'. In the
body of the message please describe the bug, how it can be reproduced, the
full version number (e.g. 1.0.0), the operating system of the server, and the
name and version number of your third party web server (if you are using one).
