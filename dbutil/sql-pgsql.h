/*
    NullLogic Groupware - Copyright (C) 2000-2003 Dan Cahill

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
#define PGSQLDB_SEQUENCES1 "\
CREATE SEQUENCE actiid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE bkmkid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE bfldid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE callid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE calaid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE contid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE evenid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE ecloid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE etypid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE fileid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE foruid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE forgid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\n"
#define PGSQLDB_SEQUENCES2 "\
CREATE SEQUENCE grouid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE mailid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE mhdrid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE messid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE noteid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE ordeid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE ordiid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE prodid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE querid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE taskid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE userid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\
CREATE SEQUENCE zoneid_seq start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;\n\n"

#define PGSQLDB_DBINFO "\
CREATE TABLE gw_dbinfo (\n\
	dbversion	varchar(10)	NOT NULL,\n\
	tax1name	varchar(20)	NOT NULL,\n\
	tax2name	varchar(20)	NOT NULL,\n\
	tax1percent	numeric(9,2)	NOT NULL DEFAULT '0.08',\n\
	tax2percent	numeric(9,2)	NOT NULL DEFAULT '0.07'\n\
);\n\n"

#define PGSQLDB_ACTIVITY "\
CREATE TABLE gw_activity (\n\
	activityid	int4		NOT NULL DEFAULT nextval('actiid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	userid		int4		NOT NULL DEFAULT 0,\n\
	clientip	varchar(50)	NOT NULL DEFAULT '',\n\
	category	varchar(50)	NOT NULL DEFAULT '',\n\
	indexid		int4		NOT NULL DEFAULT 0,\n\
	action		varchar(50)	NOT NULL DEFAULT '',\n\
	details		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (activityid)\n\
);"

#define PGSQLDB_BOOKMARKS "\
CREATE TABLE gw_bookmarks (\n\
	bookmarkid	int4		NOT NULL DEFAULT nextval('bkmkid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	folderid	int4		NOT NULL DEFAULT 0,\n\
	bookmarkname	varchar(50)	NOT NULL DEFAULT '',\n\
	bookmarkurl	varchar(255)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (bookmarkid)\n\
);\n\n"

#define PGSQLDB_BOOKMARKFOLDERS "\
CREATE TABLE gw_bookmarkfolders (\n\
	folderid	int4		NOT NULL DEFAULT nextval('bfldid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	parentid	int4		NOT NULL DEFAULT 0,\n\
	foldername	varchar(50)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (folderid)\n\
);"

#define PGSQLDB_CALLS "\
CREATE TABLE gw_calls (\n\
	callid		int4		NOT NULL DEFAULT nextval('callid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	assignedby	int4		NOT NULL DEFAULT 0,\n\
	assignedto	int4		NOT NULL DEFAULT 0,\n\
	callname	varchar(50)	NOT NULL DEFAULT '',\n\
	callstart	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	callfinish	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	contactid	int4		NOT NULL DEFAULT 0,\n\
	action		int4		NOT NULL DEFAULT 0,\n\
	status		int4		NOT NULL DEFAULT 0,\n\
	details		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (callid)\n\
);\n\n"

#define PGSQLDB_CALLACTIONS "\
CREATE TABLE gw_callactions (\n\
	callactionid	int4		NOT NULL DEFAULT nextval('calaid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	actionname	varchar(50)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (callactionid)\n\
);"

#define PGSQLDB_CONTACTS "\
CREATE TABLE gw_contacts (\n\
	contactid	int4		NOT NULL DEFAULT nextval('contid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	loginip		varchar(20)	NOT NULL DEFAULT '0.0.0.0',\n\
	logintime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	logintoken	varchar(50)	NOT NULL DEFAULT '',\n\
	username	varchar(50)	NOT NULL DEFAULT '',\n\
	password	varchar(50)	NOT NULL DEFAULT '',\n\
	enabled		int4		NOT NULL DEFAULT 0,\n\
	geozone		int4		NOT NULL DEFAULT 0,\n\
	timezone	int4		NOT NULL DEFAULT 0,\n\
	surname		varchar(50)	NOT NULL DEFAULT '',\n\
	givenname	varchar(50)	NOT NULL DEFAULT '',\n\
	salutation	varchar(50)	NOT NULL DEFAULT '',\n\
	contacttype	varchar(50)	NOT NULL DEFAULT '',\n\
	referredby	varchar(50)	NOT NULL DEFAULT '',\n\
	altcontact	varchar(50)	NOT NULL DEFAULT '',\n\
	prefbilling	varchar(50)	NOT NULL DEFAULT '',\n\
	email		varchar(50)	NOT NULL DEFAULT '',\n\
	homenumber	varchar(25)	NOT NULL DEFAULT '',\n\
	worknumber	varchar(25)	NOT NULL DEFAULT '',\n\
	faxnumber	varchar(25)	NOT NULL DEFAULT '',\n\
	mobilenumber	varchar(25)	NOT NULL DEFAULT '',\n\
	jobtitle	varchar(50)	NOT NULL DEFAULT '',\n\
	organization	varchar(50)	NOT NULL DEFAULT '',\n\
	homeaddress	varchar(50)	NOT NULL DEFAULT '',\n\
	homelocality	varchar(50)	NOT NULL DEFAULT '',\n\
	homeregion	varchar(50)	NOT NULL DEFAULT '',\n\
	homecountry	varchar(50)	NOT NULL DEFAULT '',\n\
	homepostalcode	varchar(10)	NOT NULL DEFAULT '',\n\
	workaddress	varchar(50)	NOT NULL DEFAULT '',\n\
	worklocality	varchar(50)	NOT NULL DEFAULT '',\n\
	workregion	varchar(50)	NOT NULL DEFAULT '',\n\
	workcountry	varchar(50)	NOT NULL DEFAULT '',\n\
	workpostalcode	varchar(10)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (contactid)\n\
);\n\n"

#define PGSQLDB_EVENTS "\
CREATE TABLE gw_events (\n\
	eventid		int4		NOT NULL DEFAULT nextval('evenid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	assignedby	int4		NOT NULL DEFAULT 0,\n\
	assignedto	int4		NOT NULL DEFAULT 0,\n\
	eventname	varchar(50)	NOT NULL DEFAULT '',\n\
	eventtype	int4		NOT NULL DEFAULT 0,\n\
	contactid	int4		NOT NULL DEFAULT 0,\n\
	priority	int4		NOT NULL DEFAULT 0,\n\
	reminder	int4		NOT NULL DEFAULT 0,\n\
	eventstart	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	eventfinish	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	busy		int4		NOT NULL DEFAULT 0,\n\
	status		int4		NOT NULL DEFAULT 0,\n\
	closingstatus	int4		NOT NULL DEFAULT 0,\n\
	details		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (eventid)\n\
);\n\n"

#define PGSQLDB_EVENTCLOSINGS "\
CREATE TABLE gw_eventclosings (\n\
	eventclosingid	int4		NOT NULL DEFAULT nextval('ecloid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	closingname	varchar(50)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (eventclosingid)\n\
);"

#define PGSQLDB_EVENTTYPES "\
CREATE TABLE gw_eventtypes (\n\
	eventtypeid	int4		NOT NULL DEFAULT nextval('etypid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	eventtypename	varchar(50)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (eventtypeid)\n\
);"

#define PGSQLDB_FILES "\
CREATE TABLE gw_files (\n\
	fileid		int4		NOT NULL DEFAULT nextval('fileid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	filename	varchar(255)	NOT NULL DEFAULT '',\n\
	filepath	varchar(255)	NOT NULL DEFAULT '',\n\
	filetype	varchar(10)	NOT NULL DEFAULT '',\n\
	uldate		datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	lastdldate	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	numdownloads	int4		NOT NULL DEFAULT 0,\n\
	description	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (fileid)\n\
);\n\n"

#define PGSQLDB_FORUMS "\
CREATE TABLE gw_forums (\n\
	forumid		int4		NOT NULL DEFAULT nextval('foruid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	forumgroupid	int4		NOT NULL DEFAULT 0,\n\
	postername	varchar(50)	NOT NULL DEFAULT '',\n\
	posttime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	subject		varchar(50)	NOT NULL DEFAULT '',\n\
	message		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (forumid)\n\
);\n\n"

#define PGSQLDB_FORUMGROUPS "\
CREATE TABLE gw_forumgroups (\n\
	forumgroupid	int4		NOT NULL DEFAULT nextval('forgid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	title		varchar(50)	NOT NULL DEFAULT '',\n\
	description	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (forumgroupid)\n\
);"

#define PGSQLDB_FORUMPOSTS "\
CREATE TABLE gw_forumposts (\n\
	messageid	int4		NOT NULL DEFAULT 0,\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	forumid		int4		NOT NULL DEFAULT 0,\n\
	referenceid	int4		NOT NULL DEFAULT 0,\n\
	postername	varchar(50)	NOT NULL DEFAULT '',\n\
	posttime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	subject		varchar(50)	NOT NULL DEFAULT '',\n\
	message		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (messageid, forumid)\n\
);\n\n"

#define PGSQLDB_GROUPS "\
CREATE TABLE gw_groups (\n\
	groupid		int4		NOT NULL DEFAULT nextval('grouid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	groupname	varchar(50)	NOT NULL DEFAULT '',\n\
	availability	varchar(170)	NOT NULL DEFAULT '',\n\
	motd		text		NOT NULL DEFAULT '',\n\
	members		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (groupid)\n\
);\n\n"

#define PGSQLDB_MAILACCOUNTS "\
CREATE TABLE gw_mailaccounts (\n\
	mailaccountid	int4		NOT NULL DEFAULT nextval('mailid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	accountname	varchar(50)	NOT NULL DEFAULT '',\n\
	realname	varchar(50)	NOT NULL DEFAULT '',\n\
	organization	varchar(50)	NOT NULL DEFAULT '',\n\
	address		varchar(50)	NOT NULL DEFAULT '',\n\
	hosttype	varchar(12)	NOT NULL DEFAULT '',\n\
	pophost		varchar(50)	NOT NULL DEFAULT '',\n\
	popport		int4		NOT NULL DEFAULT 110,\n\
	smtphost	varchar(50)	NOT NULL DEFAULT '',\n\
	smtpport	int4		NOT NULL DEFAULT 25,\n\
	popusername	varchar(50)	NOT NULL DEFAULT '',\n\
	poppassword	varchar(50)	NOT NULL DEFAULT '',\n\
	lastcount	int4		NOT NULL DEFAULT 0,\n\
	notify		int4		NOT NULL DEFAULT 0,\n\
	remove		int4		NOT NULL DEFAULT 0,\n\
	lastcheck	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	signature	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (mailaccountid)\n\
);"

#define PGSQLDB_MAILHEADERS "\
CREATE TABLE gw_mailheaders (\n\
	mailheaderid	int4		NOT NULL DEFAULT nextval('mhdrid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	accountid	int4		NOT NULL DEFAULT 0,\n\
	folder		varchar(50)	NOT NULL DEFAULT '',\n\
	status		varchar(10)	NOT NULL DEFAULT 'n',\n\
	size		int4		NOT NULL DEFAULT -1,\n\
	uidl		varchar(80)	NOT NULL DEFAULT '',\n\
	hdr_from	varchar(100)	NOT NULL DEFAULT '',\n\
	hdr_replyto	varchar(100)	NOT NULL DEFAULT '',\n\
	hdr_to		text		NOT NULL DEFAULT '',\n\
	hdr_cc		text		NOT NULL DEFAULT '',\n\
	hdr_bcc		text		NOT NULL DEFAULT '',\n\
	hdr_subject	varchar(100)	NOT NULL DEFAULT '',\n\
	hdr_date	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	hdr_contenttype	varchar(100)	NOT NULL DEFAULT '',\n\
	hdr_boundary	varchar(100)	NOT NULL DEFAULT '',\n\
	hdr_encoding	varchar(100)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (mailheaderid, accountid)\n\
);\n\n"

#define PGSQLDB_MESSAGES "\
CREATE TABLE gw_messages (\n\
	messageid	int4		NOT NULL DEFAULT nextval('messid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	sender		int4		NOT NULL DEFAULT 0,\n\
	rcpt		int4		NOT NULL DEFAULT 0,\n\
	status		int4		NOT NULL DEFAULT 2,\n\
	message		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (messageid)\n\
);\n\n"

#define PGSQLDB_NOTES "\
CREATE TABLE gw_notes (\n\
	noteid		int4		NOT NULL DEFAULT nextval('noteid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	tablename	varchar(50)	NOT NULL DEFAULT '',\n\
	tableindex	int4		NOT NULL DEFAULT 0,\n\
	notetitle	varchar(50)	NOT NULL DEFAULT '',\n\
	notetext	text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (noteid)\n\
);\n\n"

#define PGSQLDB_ORDERS "\
CREATE TABLE gw_orders (\n\
	orderid		int4		NOT NULL DEFAULT nextval('ordeid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	contactid	int4		NOT NULL DEFAULT 0,\n\
	userid		int4		NOT NULL DEFAULT 0,\n\
	orderdate	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	ordertype	varchar(50)	NOT NULL DEFAULT '',\n\
	paymentmethod	varchar(50)	NOT NULL DEFAULT '',\n\
	paymentdue	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	paymentreceived	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	details		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (orderid)\n\
);\n\n"

#define PGSQLDB_ORDERITEMS "\
CREATE TABLE gw_orderitems (\n\
	orderitemid	int4		NOT NULL DEFAULT nextval('ordiid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	orderid		int4		NOT NULL DEFAULT 0,\n\
	productid	int4		NOT NULL DEFAULT 0,\n\
	quantity	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	discount	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	unitprice	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	internalcost	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	tax1		numeric(9,2)	NOT NULL DEFAULT '0.08',\n\
	tax2		numeric(9,2)	NOT NULL DEFAULT '0.07',\n\
	PRIMARY KEY (orderitemid)\n\
);\n\n"

#define PGSQLDB_PRODUCTS "\
CREATE TABLE gw_products (\n\
	productid	int4		NOT NULL DEFAULT nextval('prodid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	productname	varchar(50)	NOT NULL DEFAULT '',\n\
	category	varchar(50)	NOT NULL DEFAULT '',\n\
	discount	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	unitprice	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	internalcost	numeric(9,2)	NOT NULL DEFAULT '0.00',\n\
	tax1		numeric(9,2)	NOT NULL DEFAULT '0.08',\n\
	tax2		numeric(9,2)	NOT NULL DEFAULT '0.07',\n\
	details		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (productid)\n\
);\n\n"

#define PGSQLDB_QUERIES "\
CREATE TABLE gw_queries (\n\
	queryid		int4		NOT NULL DEFAULT nextval('querid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	queryname	varchar(50)	NOT NULL DEFAULT '',\n\
	query		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (queryid)\n\
);\n\n"

#define PGSQLDB_TASKS "\
CREATE TABLE gw_tasks (\n\
	taskid		int4		NOT NULL DEFAULT nextval('taskid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	assignedby	int4		NOT NULL DEFAULT 0,\n\
	assignedto	int4		NOT NULL DEFAULT 0,\n\
	taskname	varchar(50)	NOT NULL DEFAULT '',\n\
	duedate		datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	priority	int4		NOT NULL DEFAULT 0,\n\
	reminder	int4		NOT NULL DEFAULT 0,\n\
	status		int4		NOT NULL DEFAULT 0,\n\
	details		text		NOT NULL DEFAULT '',\n\
	PRIMARY KEY (taskid)\n\
);\n\n"

#define PGSQLDB_USERS "\
CREATE TABLE gw_users (\n\
	userid		int4		NOT NULL DEFAULT nextval('userid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	loginip		varchar(20)	NOT NULL DEFAULT '',\n\
	logintime	datetime	NOT NULL DEFAULT '1970-01-01',\n\
	logintoken	varchar(50)	NOT NULL DEFAULT '',\n\
	username	varchar(50)	NOT NULL DEFAULT '',\n\
	password	varchar(50)	NOT NULL DEFAULT '',\n\
	groupid		int4		NOT NULL DEFAULT 0,\n\
	enabled		int4		NOT NULL DEFAULT 0,\n\
	authadmin	int4		NOT NULL DEFAULT 0,\n\
	authbookmarks	int4		NOT NULL DEFAULT 0,\n\
	authcalendar	int4		NOT NULL DEFAULT 0,\n\
	authcalls	int4		NOT NULL DEFAULT 0,\n\
	authcontacts	int4		NOT NULL DEFAULT 0,\n\
	authfiles	int4		NOT NULL DEFAULT 0,\n\
	authforums	int4		NOT NULL DEFAULT 0,\n\
	authmessages	int4		NOT NULL DEFAULT 0,\n\
	authorders	int4		NOT NULL DEFAULT 0,\n\
	authprofile	int4		NOT NULL DEFAULT 0,\n\
	authquery	int4		NOT NULL DEFAULT 0,\n\
	authwebmail	int4		NOT NULL DEFAULT 0,\n\
	prefdaystart	int4		NOT NULL DEFAULT 0,\n\
	prefdaylength	int4		NOT NULL DEFAULT 0,\n\
	prefmailcurrent	int4		NOT NULL DEFAULT 0,\n\
	prefmaildefault	int4		NOT NULL DEFAULT 0,\n\
	prefmaxlist	int4		NOT NULL DEFAULT 0,\n\
	prefmenustyle	int4		NOT NULL DEFAULT 1,\n\
	preftimezone	int4		NOT NULL DEFAULT 0,\n\
	prefgeozone	int4		NOT NULL DEFAULT 0,\n\
	availability	varchar(170)	NOT NULL DEFAULT '',\n\
	surname		varchar(50),\n\
	givenname	varchar(50),\n\
	jobtitle	varchar(50),\n\
	division	varchar(50),\n\
	supervisor	varchar(50),\n\
	address		varchar(50),\n\
	locality	varchar(50),\n\
	region		varchar(50),\n\
	country		varchar(50),\n\
	postalcode	varchar(10),\n\
	homenumber	varchar(25),\n\
	worknumber	varchar(25),\n\
	faxnumber	varchar(25),\n\
	cellnumber	varchar(25),\n\
	pagernumber	varchar(25),\n\
	email		varchar(50),\n\
	birthdate	date		NOT NULL DEFAULT '1970-01-01',\n\
	hiredate	date		NOT NULL DEFAULT '1970-01-01',\n\
	sin		varchar(15),\n\
	isactive	varchar(10),\n\
	PRIMARY KEY (userid)\n\
);\n\n"

#define PGSQLDB_ZONES "\
CREATE TABLE gw_zones (\n\
	zoneid		int4		NOT NULL DEFAULT nextval('bkmkid_seq'::text),\n\
	obj_ctime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_mtime	datetime	NOT NULL DEFAULT '1970-01-01 00:00:00',\n\
	obj_uid		int4		NOT NULL DEFAULT 0,\n\
	obj_gid		int4		NOT NULL DEFAULT 0,\n\
	obj_did		int4		NOT NULL DEFAULT 0,\n\
	obj_gperm	int4		NOT NULL DEFAULT 0,\n\
	obj_operm	int4		NOT NULL DEFAULT 0,\n\
	zonename	varchar(50)	NOT NULL DEFAULT '',\n\
	PRIMARY KEY (zoneid)\n\
);\n\n"