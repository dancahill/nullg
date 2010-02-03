From: "Dan Cahill" <nulllogic@users.sf.net>
To: New NullLogic GroupServer User
Subject: Welcome
Date: Wed, 17 May 2006 18:18:25 -0400
MIME-Version: 1.0
Content-Type: text/html; charset=iso-8859-1
X-Mailer: NullLogic GroupServer 1.3.30

<HTML>
<CENTER>
<STRONG>
<FONT color=red>welcome to null groupserver</FONT>
</STRONG>
<P>blah and stuff</P>
</CENTER>
-------------------------------------------<BR />
NullLogic GroupServer version 1.3 Readme File<BR />
-------------------------------------------<BR />
<BR />
This readme is intended to provide only the most cursory introduction
to NullLogic GroupServer.  For more information on installation and
configuration please refer to the included html documentation.<BR />
<BR />
--------<BR />
CONTENTS<BR />
--------<BR />
1. DESCRIPTION<BR />
2. INSTALLATION<BR />
3. DATA ACCESS<BR />
4. REPORTING A BUG<BR />
<BR />
--------------<BR />
1. DESCRIPTION<BR />
--------------<BR />
NullLogic GroupServer is a multi-user web-based groupware package designed
for contact management and event scheduling, which includes additional
features such as private messaging, public discussion forums, shared
hyperlinks, file bases, a web e-mail client, and a simple order
processing system.<BR />
<BR />
The main goal of NullLogic GroupServer is to make your data accessible to
you from anywhere at any time without the need for special software to
be installed on the client computer.<BR />
<BR />
Linux and Windows are both supported and NullLogic GroupServer has been
thoroughly tested with MySQL, PostgreSQL, SQLite and ODBC.<BR />
<BR />
---------------<BR />
2. INSTALLATION<BR />
---------------<BR />
To install NullLogic GroupServer, you will need to run 'SETUP.EXE'
(Windows) or 'setup' (Linux).  If any information is required during
this process, you will be prompted for the appropriate information.<BR />
<BR />
Once you have finished running the setup program, you should have a
fully functional installation of NullLogic GroupServer.  There are,
however, several different ways of configuring NullLogic GroupServer which
may be much better suited to its desired task.<BR />
<BR />
NullLogic GroupServer can be configured to use an SQL server such as MySQL
or PostgreSQL.  To initialise, backup, or restore a NullLogic GroupServer
database, use the command line tool 'NULLSD-DBUTIL.EXE' or 'nullsd-dbutil'
(on Windows and Linux respectively).<BR />
<BR />
To log in to NullLogic GroupServer for the first time, the username is
'administrator', and the password is 'visual'.  Be sure to change this
password as soon as possible.<BR />
<BR />
You will need to make sure the server is in your startup.  On Windows, this
should be done automatically.  On Linux, you will need to call 'rc.groupware'
from the appropriate startup script.<BR />
<BR />
--------------<BR />
3. DATA ACCESS<BR />
--------------<BR />
The default installation of NullLogic GroupServer uses SQLite for database
access.  Although reliable, this configuration may not offer the level
of scalability or accessibility you require.  In this case, you may want
to consider using MySQL or PostgreSQL instead.<BR />
<BR />
First, you will need to make an account in your SQL server for NullLogic
GroupServer.  It is probably wise not to give this account permission to
create databases and users, but it should have permission to create new
tables.<BR />
<BR />
After you have configured the new database and user, change the SQL
server type in 'nullsd.conf' to match the appropriate settings.<BR />
<BR />
The command line tool 'nullsd-dbutil' can be used to initialise the
database for either MySQL or PostgreSQL.  Note that the NullLogic
GroupServer database must be created (not the tables, just the bare
database) prior to running nullsd-dbutil if you are using MySQL or
PostgreSQL.  The nullsd-dbutil program will create and populate all
of the necessary tables.<BR />
<BR />
------------------<BR />
4. REPORTING A BUG<BR />
------------------<BR />
If you believe you have found a bug, the first question is can you
reproduce it?  If you can, then it is really a bug.<BR />
<BR />
Next, please visit http://nullgroupware.sourceforge.net/ and check the
forums to see if anyone else is having the same problem, or if the
problem has already been fixed.<BR />
<BR />
If the bug you just found is not on the list, please send an e-mail to
nulllogic@users.sourceforge.net with the Subject line 'NullLogic GroupServer
Bug'.  In the body of the message please describe the bug, how it can be
reproduced, the full version number (e.g. 1.2.3), and the operating system
of the server.<BR />
</HTML>
