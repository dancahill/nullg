/*
    Null Groupware -- web-based groupware system
    Copyright (C) 2000-2003 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define MDB_DBINFO "\
CREATE TABLE gw_dbinfo (\n\
	dbversion	varchar(10)	NOT NULL,\n\
	tax1name	varchar(20)	NOT NULL,\n\
	tax2name	varchar(20)	NOT NULL,\n\
	tax1percent	float		NOT NULL,\n\
	tax2percent	float		NOT NULL\n\
);\n\n"

/*
#define MDB_CALLS "\
CREATE TABLE gw_calls (\n\
	callid		integer NOT NULL autoincrement,\n\
	cdatetime	datetime,\n\
	mdatetime	datetime,\n\
	orderid		varchar(50),\n\
	calldate	datetime,\n\
	subject		varchar(254),\n\
	notes		memo,\n\
	PRIMARY KEY (callid)\n\
);\n\n"
*/

#define MDB_COMPANIES "\
CREATE TABLE gw_companies (\n\
	companyid	autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	companyname	varchar(50)	NOT NULL,\n\
	companytype	varchar(50),\n\
	phonenumber	varchar(25),\n\
	faxnumber	varchar(25),\n\
	website		varchar(50),\n\
	address		varchar(50),\n\
	city		varchar(50),\n\
	country		varchar(50),\n\
	postal		varchar(50),\n\
	contactid	integer,\n\
	PRIMARY KEY (companyid)\n\
);\n\n"

#define MDB_CONTACTS "\
CREATE TABLE gw_contacts (\n\
	contactid	autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	lastname	varchar(50)	NOT NULL,\n\
	firstname	varchar(50)	NOT NULL,\n\
	jobtitle	varchar(50),\n\
	companyid	integer,\n\
	contacttype	varchar(50),\n\
	address		varchar(50),\n\
	city		varchar(50),\n\
	country		varchar(50),\n\
	postal		varchar(10),\n\
	worknumber	varchar(25),\n\
	homenumber	varchar(25),\n\
	faxnumber	varchar(25),\n\
	cellnumber	varchar(25),\n\
	pagernumber	varchar(25),\n\
	email		varchar(50),\n\
	lastmeetingdate	date,\n\
	referredby	varchar(50),\n\
	iscustomer	varchar(10),\n\
	notes		memo,\n\
	PRIMARY KEY (contactid)\n\
);\n\n"

#define MDB_EVENTS "\
CREATE TABLE gw_events (\n\
	eventid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	assignedby	varchar(50),\n\
	assignedto	varchar(50),\n\
	eventname	varchar(50)	NOT NULL,\n\
	eventstatus	varchar(50),\n\
	eventdate	datetime	NOT NULL,\n\
	eventlength	integer,\n\
	contactid	integer,\n\
	ispublic	integer,\n\
	priority	integer,\n\
	reminder	integer,\n\
	repeat		integer,\n\
	repeatuntil	date,\n\
	eventnotes	memo,\n\
	PRIMARY KEY (eventid)\n\
);\n\n"

#define MDB_FORUMS "\
CREATE TABLE gw_forums (\n\
	forumid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	postername	varchar(50)	NOT NULL,\n\
	posttime	datetime	NOT NULL,\n\
	subject		varchar(40)	NOT NULL,\n\
	message		memo,\n\
	PRIMARY KEY (forumid)\n\
);\n\n"

#define MDB_FORUMPOSTS "\
CREATE TABLE gw_forumposts (\n\
	messageid	integer		NOT NULL,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	forumid		integer		NOT NULL,\n\
	referenceid	integer		NOT NULL,\n\
	postername	varchar(50)	NOT NULL,\n\
	posttime	datetime	NOT NULL,\n\
	subject		varchar(40)	NOT NULL,\n\
	message		memo,\n\
	PRIMARY KEY (messageid, forumid)\n\
);\n\n"

#define MDB_LINKS "\
CREATE TABLE gw_links (\n\
	linkid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	creator		varchar(50)	NOT NULL,\n\
	linkname	varchar(50)	NOT NULL,\n\
	linktype	varchar(50)	NOT NULL,\n\
	linkurl		varchar(255)	NOT NULL,\n\
	PRIMARY KEY (linkid)\n\
);\n\n"

#define MDB_MESSAGES "\
CREATE TABLE gw_messages (\n\
	messageid	autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	msgfrom		varchar(50)	NOT NULL,\n\
	msgto		varchar(50)	NOT NULL,\n\
	msgstatus	integer		NOT NULL,\n\
	msgsubject	varchar(50)	NOT NULL,\n\
	msgtext		memo,\n\
	PRIMARY KEY (messageid)\n\
);\n\n"

#define MDB_ORDERS "\
CREATE TABLE gw_orders (\n\
	orderid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	customerid	integer		NOT NULL,\n\
	employee	varchar(50)	NOT NULL,\n\
	orderdate	datetime	NOT NULL,\n\
	ordertype	varchar(50)	NOT NULL,\n\
	paymentmethod	varchar(50)	NOT NULL,\n\
	paymentdue	currency	NOT NULL,\n\
	paymentreceived	currency	NOT NULL,\n\
	notes		memo,\n\
	PRIMARY KEY (orderid)\n\
);\n\n"

#define MDB_ORDERDETAILS "\
CREATE TABLE gw_orderdetails (\n\
	orderdetailid	autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	orderid		integer		NOT NULL,\n\
	productname	varchar(50)	NOT NULL,\n\
	quantity	float		NOT NULL,\n\
	discount	float		NOT NULL,\n\
	unitprice	currency	NOT NULL,\n\
	internalcost	currency	NOT NULL,\n\
	tax1		float		NOT NULL,\n\
	tax2		float		NOT NULL,\n\
	PRIMARY KEY (orderdetailid)\n\
);\n\n"

#define MDB_PRODUCTS "\
CREATE TABLE gw_products (\n\
	productid	autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	productname	varchar(50)	NOT NULL,\n\
	category	varchar(50)	NOT NULL,\n\
	discount	float		NOT NULL,\n\
	unitprice	currency	NOT NULL,\n\
	internalcost	currency	NOT NULL,\n\
	tax1		float		NOT NULL,\n\
	tax2		float		NOT NULL,\n\
	description	memo,\n\
	PRIMARY KEY (productid)\n\
);\n\n"

#define MDB_QUERIES "\
CREATE TABLE gw_queries (\n\
	queryid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	creator		varchar(50)	NOT NULL,\n\
	queryname	varchar(50)	NOT NULL,\n\
	query		memo		NOT NULL,\n\
	PRIMARY KEY (queryid)\n\
);\n\n"

#define MDB_TASKS "\
CREATE TABLE gw_tasks (\n\
	taskid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	assignedby	varchar(50)	NOT NULL,\n\
	assignedto	varchar(50)	NOT NULL,\n\
	taskname	varchar(50)	NOT NULL,\n\
	status		varchar(50)	NOT NULL,\n\
	ispublic	integer		NOT NULL,\n\
	priority	integer		NOT NULL,\n\
	duedate		datetime	NOT NULL,\n\
	tasktext	memo,\n\
	PRIMARY KEY (taskid)\n\
);\n\n"

#define MDB_USERS "\
CREATE TABLE gw_users (\n\
	userid		autoincrement,\n\
	cdatetime	datetime	NOT NULL,\n\
	mdatetime	datetime	NOT NULL,\n\
	lastloginip	varchar(20),\n\
	lastlogintime	datetime,\n\
	lastlogintoken	varchar(50),\n\
	username	varchar(50)	NOT NULL,\n\
	password	varchar(50),\n\
	groupid		integer		NOT NULL,\n\
	enabled		integer		NOT NULL,\n\
	emailaddress	varchar(50),\n\
	emailusername	varchar(50),\n\
	emailpassword	varchar(50),\n\
	pop3server	varchar(50),\n\
	smtpserver	varchar(50),\n\
	authadmin	integer		NOT NULL,\n\
	authcalendar	integer		NOT NULL,\n\
	authcontacts	integer		NOT NULL,\n\
	authfiles	integer		NOT NULL,\n\
	authforums	integer		NOT NULL,\n\
	authlinks	integer		NOT NULL,\n\
	authmessages	integer		NOT NULL,\n\
	authorders	integer		NOT NULL,\n\
	authquery	integer		NOT NULL,\n\
	authwebmail	integer		NOT NULL,\n\
	lastname	varchar(50),\n\
	firstname	varchar(50),\n\
	jobtitle	varchar(50),\n\
	division	varchar(50),\n\
	supervisor	varchar(50),\n\
	address		varchar(50),\n\
	city		varchar(50),\n\
	country		varchar(50),\n\
	postalcode	varchar(10),\n\
	homenumber	varchar(25),\n\
	worknumber	varchar(25),\n\
	cellnumber	varchar(25),\n\
	faxnumber	varchar(25),\n\
	pagernumber	varchar(25),\n\
	birthdate	date,\n\
	hiredate	date,\n\
	sin		varchar(15),\n\
	isactive	varchar(10),\n\
	comment		memo,\n\
	PRIMARY KEY (userid)\n\
);\n\n"

#define MDB_DATA0 "INSERT INTO gw_dbinfo (dbversion, tax1name, tax2name, tax1percent, tax2percent) VALUES ('1.0','PST','GST',0.08, 0.07);\n"
#define MDB_DATA1 "INSERT INTO gw_messages (messageid, cdatetime, mdatetime, msgfrom, msgto, msgstatus, msgsubject, msgtext) VALUES (1,'2000-01-01 00:00:00','2000-01-01 00:00:00','The Sentinel Groupware Team','administrator',2,'Welcome','Welcome to Sentinel Groupware.');\n"
#define MDB_DATA2 "INSERT INTO gw_users (userid, cdatetime, mdatetime, lastloginip, lastlogintime, lastlogintoken, username, password, groupid, enabled, emailaddress, emailusername, emailpassword, pop3server, smtpserver, authadmin, authcalendar, authcontacts, authfiles, authforums, authlinks, authmessages, authorders, authquery, authwebmail, lastname, firstname, jobtitle, division, supervisor, address, city, country, postalcode, homenumber, worknumber, cellnumber, faxnumber, pagernumber, birthdate, hiredate, sin, isactive, comment) VALUES (1,'2000-01-01 00:00:00','2000-01-01 00:00:00','0.0.0.0','1970-01-01 00:00:00','','administrator','$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/',0,1,'','','','','',4,4,4,4,4,4,4,4,4,4,'','','','','','','','','','','','','','','1970-01-01','1970-01-01','','','This is the default system administration account.  If you have not already done so, please change the default password.');\n"
