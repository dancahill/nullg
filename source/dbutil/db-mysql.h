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

#define MYSQLDB_DBINFO "\
CREATE TABLE gw_dbinfo (\n\
	dbversion	varchar(10)	NOT NULL,\n\
	tax1name	varchar(20)	NOT NULL,\n\
	tax2name	varchar(20)	NOT NULL,\n\
	tax1percent	numeric(9,2)	NOT NULL DEFAULT '0.08',\n\
	tax2percent	numeric(9,2)	NOT NULL DEFAULT '0.07'\n\
);"

/*
#define MYSQLDB_CALLS "\
CREATE TABLE gw_calls (\n\
	callid		int4 NOT NULL auto_increment,\n\
	cdatetime	datetime,\n\
	mdatetime	datetime,\n\
	orderid		varchar(50),\n\
	calldate	datetime,\n\
	subject		varchar(254),\n\
	notes		text,\n\
	PRIMARY KEY (callid)\n\
);"
*/
#define MYSQLDB_COMPANIES "\
CREATE TABLE gw_companies (\n\
	companyid	int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	companyname	varchar(50)	NOT NULL DEFAULT '',\n\
	companytype	varchar(50)	NOT NULL DEFAULT '',\n\
	phonenumber	varchar(25)	NOT NULL DEFAULT '',\n\
	faxnumber	varchar(25)	NOT NULL DEFAULT '',\n\
	website		varchar(50)	NOT NULL DEFAULT '',\n\
	address		varchar(50)	NOT NULL DEFAULT '',\n\
	city		varchar(50)	NOT NULL DEFAULT '',\n\
	country		varchar(50)	NOT NULL DEFAULT '',\n\
	postal		varchar(50)	NOT NULL DEFAULT '',\n\
	contactid	int4		NOT NULL DEFAULT 0,\n\
	PRIMARY KEY (companyid)\n\
);"

#define MYSQLDB_CONTACTS "\
CREATE TABLE gw_contacts (\n\
	contactid	int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	lastname	varchar(50)	NOT NULL DEFAULT '',\n\
	firstname	varchar(50)	NOT NULL DEFAULT '',\n\
	jobtitle	varchar(50)	NOT NULL DEFAULT '',\n\
	companyid	int4		NOT NULL DEFAULT 0,\n\
	contacttype	varchar(50)	NOT NULL DEFAULT '',\n\
	address		varchar(50)	NOT NULL DEFAULT '',\n\
	city		varchar(50)	NOT NULL DEFAULT '',\n\
	country		varchar(50)	NOT NULL DEFAULT '',\n\
	postal		varchar(10)	NOT NULL DEFAULT '',\n\
	worknumber	varchar(25)	NOT NULL DEFAULT '',\n\
	homenumber	varchar(25)	NOT NULL DEFAULT '',\n\
	faxnumber	varchar(25)	NOT NULL DEFAULT '',\n\
	cellnumber	varchar(25)	NOT NULL DEFAULT '',\n\
	pagernumber	varchar(25)	NOT NULL DEFAULT '',\n\
	email		varchar(50)	NOT NULL DEFAULT '',\n\
	lastmeetingdate	date		NOT NULL DEFAULT '1970-01-01',\n\
	referredby	varchar(50)	NOT NULL DEFAULT '',\n\
	iscustomer	varchar(10)	NOT NULL DEFAULT '',\n\
	notes		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (contactid)\n\
);"

#define MYSQLDB_EVENTS "\
CREATE TABLE gw_events (\n\
	eventid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	assignedby	varchar(50)	NOT NULL DEFAULT '',\n\
	assignedto	varchar(50)	NOT NULL DEFAULT '',\n\
	eventname	varchar(50)	NOT NULL DEFAULT '',\n\
	eventstatus	varchar(50)	NOT NULL DEFAULT '',\n\
	eventdate	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	eventlength	int4		NOT NULL DEFAULT 0,\n\
	contactid	int4		NOT NULL DEFAULT 0,\n\
	ispublic	int4		NOT NULL DEFAULT 0,\n\
	priority	int4		NOT NULL DEFAULT 0,\n\
	reminder	int4		NOT NULL DEFAULT 0,\n\
	repeat		int4		NOT NULL DEFAULT 0,\n\
	repeatuntil	date		NOT NULL DEFAULT '1970-01-01',\n\
	eventnotes	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (eventid)\n\
);"

#define MYSQLDB_FORUMS "\
CREATE TABLE gw_forums (\n\
	forumid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	postername	varchar(50)	NOT NULL DEFAULT '',\n\
	posttime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	subject		varchar(40)	NOT NULL DEFAULT '',\n\
	message		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (forumid)\n\
);"

#define MYSQLDB_FORUMPOSTS "\
CREATE TABLE gw_forumposts (\n\
	messageid	int4		NOT NULL DEFAULT 0,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	forumid		int4		NOT NULL DEFAULT 0,\n\
	referenceid	int4		NOT NULL DEFAULT 0,\n\
	postername	varchar(50)	NOT NULL DEFAULT '',\n\
	posttime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	subject		varchar(40)	NOT NULL DEFAULT '',\n\
	message		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (messageid, forumid)\n\
);"

#define MYSQLDB_LINKS "\
CREATE TABLE gw_links (\n\
	linkid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	creator		varchar(50)	NOT NULL DEFAULT '',\n\
	linkname	varchar(50)	NOT NULL DEFAULT '',\n\
	linktype	varchar(50)	NOT NULL DEFAULT '',\n\
	linkurl		varchar(100)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (linkid)\n\
);"

#define MYSQLDB_MESSAGES "\
CREATE TABLE gw_messages (\n\
	messageid	int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	msgfrom		varchar(50)	NOT NULL DEFAULT '',\n\
	msgto		varchar(50)	NOT NULL DEFAULT '',\n\
	msgstatus	int4		NOT NULL DEFAULT 2,\n\
	msgsubject	varchar(50)	NOT NULL DEFAULT '',\n\
	msgtext		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (messageid)\n\
);"

#define MYSQLDB_ORDERS "\
CREATE TABLE gw_orders (\n\
	orderid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	customerid	int4		NOT NULL DEFAULT 0,\n\
	employee	varchar(50)	NOT NULL DEFAULT '',\n\
	orderdate	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	ordertype	varchar(50)	NOT NULL DEFAULT '',\n\
	paymentmethod	varchar(50)	NOT NULL DEFAULT '',\n\
	paymentdue	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	paymentreceived	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	notes		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (orderid)\n\
);"

#define MYSQLDB_ORDERDETAILS "\
CREATE TABLE gw_orderdetails (\n\
	orderdetailid	int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	orderid		int4		NOT NULL DEFAULT 0,\n\
	productname	varchar(50)	NOT NULL DEFAULT '',\n\
	quantity	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	discount	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	unitprice	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	internalcost	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	tax1		numeric(9,2)	NOT NULL DEFAULT '0.08',\n\
	tax2		numeric(9,2)	NOT NULL DEFAULT '0.07',\n\
	PRIMARY KEY (orderdetailid)\n\
);"

#define MYSQLDB_PRODUCTS "\
CREATE TABLE gw_products (\n\
	productid	int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	productname	varchar(50)	NOT NULL DEFAULT '',\n\
	category	varchar(50)	NOT NULL DEFAULT '',\n\
	discount	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	unitprice	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	internalcost	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	tax1		numeric(9,2)	NOT NULL DEFAULT '0.08',\n\
	tax2		numeric(9,2)	NOT NULL DEFAULT '0.07',\n\
	description	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (productid)\n\
);"

#define MYSQLDB_QUERIES "\
CREATE TABLE gw_queries (\n\
	queryid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	creator		varchar(50)	NOT NULL DEFAULT '',\n\
	queryname	varchar(50)	NOT NULL DEFAULT '',\n\
	query		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (queryid)\n\
);"

#define MYSQLDB_TASKS "\
CREATE TABLE gw_tasks (\n\
	taskid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	taskname	varchar(50)	NOT NULL DEFAULT '',\n\
	assignedby	varchar(50)	NOT NULL DEFAULT '',\n\
	assignedto	varchar(50)	NOT NULL DEFAULT '',\n\
	ispublic	int4		NOT NULL DEFAULT 0,\n\
	priority	int4		NOT NULL DEFAULT 0,\n\
	duedate		datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	status		varchar(50)	NOT NULL DEFAULT '',\n\
	tasktext	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (taskid)\n\
);"

#define MYSQLDB_USERS "\
CREATE TABLE gw_users (\n\
	userid		int4		NOT NULL auto_increment,\n\
	cdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	mdatetime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	lastloginip	varchar(20)	NOT NULL DEFAULT '0.0.0.0',\n\
	lastlogintime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	lastlogintoken	varchar(50)	NOT NULL DEFAULT '',\n\
	username	varchar(50)	NOT NULL DEFAULT '',\n\
	password	varchar(50)	NOT NULL DEFAULT '',\n\
	groupid		int4		NOT NULL DEFAULT 0,\n\
	enabled		int4		NOT NULL DEFAULT 0,\n\
	emailaddress	varchar(50)	NOT NULL DEFAULT '',\n\
	emailusername	varchar(50)	NOT NULL DEFAULT '',\n\
	emailpassword	varchar(50)	NOT NULL DEFAULT '',\n\
	pop3server	varchar(50)	NOT NULL DEFAULT '',\n\
	smtpserver	varchar(50)	NOT NULL DEFAULT '',\n\
	authadmin	int4		NOT NULL DEFAULT 0,\n\
	authcalendar	int4		NOT NULL DEFAULT 0,\n\
	authcontacts	int4		NOT NULL DEFAULT 0,\n\
	authfiles	int4		NOT NULL DEFAULT 0,\n\
	authforums	int4		NOT NULL DEFAULT 0,\n\
	authlinks	int4		NOT NULL DEFAULT 0,\n\
	authmessages	int4		NOT NULL DEFAULT 0,\n\
	authorders	int4		NOT NULL DEFAULT 0,\n\
	authquery	int4		NOT NULL DEFAULT 0,\n\
	authwebmail	int4		NOT NULL DEFAULT 0,\n\
	lastname	varchar(50)	NOT NULL DEFAULT '',\n\
	firstname	varchar(50)	NOT NULL DEFAULT '',\n\
	jobtitle	varchar(50)	NOT NULL DEFAULT '',\n\
	division	varchar(50)	NOT NULL DEFAULT '',\n\
	supervisor	varchar(50)	NOT NULL DEFAULT '',\n\
	address		varchar(50)	NOT NULL DEFAULT '',\n\
	city		varchar(50)	NOT NULL DEFAULT '',\n\
	country		varchar(50)	NOT NULL DEFAULT '',\n\
	postalcode	varchar(10)	NOT NULL DEFAULT '',\n\
	homenumber	varchar(25)	NOT NULL DEFAULT '',\n\
	worknumber	varchar(25)	NOT NULL DEFAULT '',\n\
	cellnumber	varchar(25)	NOT NULL DEFAULT '',\n\
	faxnumber	varchar(25)	NOT NULL DEFAULT '',\n\
	pagernumber	varchar(25)	NOT NULL DEFAULT '',\n\
	birthdate	date		NOT NULL DEFAULT '1970-01-01',\n\
	hiredate	date		NOT NULL DEFAULT '1970-01-01',\n\
	sin		varchar(15)	NOT NULL DEFAULT '',\n\
	isactive	varchar(10)	NOT NULL DEFAULT '',\n\
	comment		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (userid)\n\
);"

#define MYSQLDB_DATA0 "INSERT INTO gw_dbinfo (dbversion, tax1name, tax2name, tax1percent, tax2percent) VALUES ('1.0','PST','GST',0.08, 0.07);"
#define MYSQLDB_DATA1 "INSERT INTO gw_messages (messageid, cdatetime, mdatetime, msgfrom, msgto, msgstatus, msgsubject, msgtext) VALUES (1,'2000-01-01 00:00:00','2000-01-01 00:00:00','The Sentinel Groupware Team','administrator',2,'Welcome','Welcome to Sentinel Groupware.');"
#define MYSQLDB_DATA2 "INSERT INTO gw_users (userid, cdatetime, mdatetime, lastloginip, lastlogintime, lastlogintoken, username, password, groupid, enabled, emailaddress, emailusername, emailpassword, pop3server, smtpserver, authadmin, authcalendar, authcontacts, authfiles, authforums, authlinks, authmessages, authorders, authquery, authwebmail, lastname, firstname, jobtitle, division, supervisor, address, city, country, postalcode, homenumber, worknumber, cellnumber, faxnumber, pagernumber, birthdate, hiredate, sin, isactive, comment) VALUES (1,'2000-01-01 00:00:00','2000-01-01 00:00:00','0.0.0.0','1970-01-01 00:00:00','NULL','administrator','$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/',0,1,'','','','','',4,4,4,4,4,4,4,4,4,4,'','','','','','','','','','','','','','','1970-01-01','1970-01-01','','','This is the default system administration account.  If you have not already done so, please change the default password.');"
