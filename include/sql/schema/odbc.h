/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

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

static const tabledef mdb_tables[] = {
	{
		"gw_dbinfo",
		"dbversion",
		NULL,
		"CREATE TABLE gw_dbinfo (\n"
		"	dbversion	varchar(10)	NOT NULL,\n"
		"	tax1name	varchar(20)	NOT NULL,\n"
		"	tax2name	varchar(20)	NOT NULL,\n"
		"	tax1percent	float		NOT NULL,\n"
		"	tax2percent	float		NOT NULL\n"
		")"
	},
	{
		"gw_accounting_accounts",
		"accountid",
		NULL,
		"CREATE TABLE gw_accounting_accounts (\n"
		"	accountid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	accountname	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (accountid)\n"
		")"
	},
	{
		"gw_accounting_journal",
		"journalentryid",
		NULL,
		"CREATE TABLE gw_accounting_journal (\n"
		"	journalentryid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	entrydate	datetime	NOT NULL,\n"
		"	accountid	integer,\n"
		"	debit		currency	NOT NULL,\n"
		"	credit		currency	NOT NULL,\n"
		"	details		varchar(250)	NOT NULL,\n"
		"	PRIMARY KEY (journalentryid)\n"
		")"
	},
	{
		"gw_activity",
		"activityid",
		NULL,
		"CREATE TABLE gw_activity (\n"
		"	activityid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	userid		integer,\n"
		"	clientip	varchar(50)	NOT NULL,\n"
		"	category	varchar(50)	NOT NULL,\n"
		"	indexid		integer,\n"
		"	action		varchar(50)	NOT NULL,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (activityid)\n"
		")"
	},
	{
		"gw_bookmarks",
		"bookmarkid",
		NULL,
		"CREATE TABLE gw_bookmarks (\n"
		"	bookmarkid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	folderid	integer,\n"
		"	bookmarkname	varchar(50)	NOT NULL,\n"
		"	bookmarkurl	varchar(255)	NOT NULL,\n"
		"	PRIMARY KEY (bookmarkid)\n"
		")"
	},
	{
		"gw_bookmarkfolders",
		"folderid",
		NULL,
		"CREATE TABLE gw_bookmarkfolders (\n"
		"	folderid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	parentid	integer,\n"
		"	foldername	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (folderid)\n"
		")"
	},
	{
		"gw_calls",
		"callid",
		NULL,
		"CREATE TABLE gw_calls (\n"
		"	callid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	assignedby	integer,\n"
		"	assignedto	integer,\n"
		"	callname	varchar(50)	NOT NULL,\n"
		"	callstart	datetime,\n"
		"	callfinish	datetime,\n"
		"	contactid	integer,\n"
		"	action		integer,\n"
		"	status		integer,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (callid)\n"
		")"
	},
	{
		"gw_callactions",
		"callactionid",
		NULL,
		"CREATE TABLE gw_callactions (\n"
		"	callactionid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	actionname	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (callactionid)\n"
		")"
	},
	{
		"gw_contacts",
		"contactid",
		NULL,
		"CREATE TABLE gw_contacts (\n"
		"	contactid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	folderid	integer,\n"
		"	username	varchar(50),\n"
		"	password	varchar(50),\n"
		"	enabled		integer,\n"
		"	geozone		integer,\n"
		"	timezone	integer,\n"
		"	surname		varchar(50),\n"
		"	givenname	varchar(50),\n"
		"	salutation	varchar(50),\n"
		"	contacttype	varchar(50),\n"
		"	referredby	varchar(50),\n"
		"	altcontact	varchar(50),\n"
		"	prefbilling	varchar(50),\n"
		"	website		varchar(250),\n"
		"	email		varchar(250),\n"
		"	homenumber	varchar(25),\n"
		"	worknumber	varchar(25),\n"
		"	faxnumber	varchar(25),\n"
		"	mobilenumber	varchar(25),\n"
		"	jobtitle	varchar(50),\n"
		"	organization	varchar(50),\n"
		"	homeaddress	varchar(50),\n"
		"	homelocality	varchar(50),\n"
		"	homeregion	varchar(50),\n"
		"	homecountry	varchar(50),\n"
		"	homepostalcode	varchar(10),\n"
		"	workaddress	varchar(50),\n"
		"	worklocality	varchar(50),\n"
		"	workregion	varchar(50),\n"
		"	workcountry	varchar(50),\n"
		"	workpostalcode	varchar(10),\n"
		"	PRIMARY KEY (contactid)\n"
		")"
	},
	{
		"gw_contactsessions",
		"sessionid",
		NULL,
		"CREATE TABLE gw_contactsessions (\n"
		"	sessionid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	remoteip	varchar(20)\n"
		"	token		varchar(50),\n"
		"	contactid	integer,\n"
		"	domainid	integer,\n"
		"	PRIMARY KEY (sessionid)\n"
		")"
	},
	{
		"gw_domains",
		"domainid",
		NULL,
		"CREATE TABLE gw_domains (\n"
		"	domainid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	domainname	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (domainid)\n"
		")"
	},
	{
		"gw_domainaliases",
		"domainaliasid",
		NULL,
		"CREATE TABLE gw_domainaliases (\n"
		"	domainaliasid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	domainid	integer,\n"
		"	domainname	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (domainaliasid)\n"
		")"
	},
	{
		"gw_events",
		"eventid",
		NULL,
		"CREATE TABLE gw_events (\n"
		"	eventid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	assignedby	integer,\n"
		"	assignedto	integer,\n"
		"	eventname	varchar(50)	NOT NULL,\n"
		"	eventtype	integer,\n"
		"	contactid	integer,\n"
		"	projectid	integer		NOT NULL,\n"
		"	priority	integer,\n"
		"	reminder	integer,\n"
		"	eventstart	datetime	NOT NULL,\n"
		"	eventfinish	datetime	NOT NULL,\n"
		"	busy		integer,\n"
		"	status		integer,\n"
		"	closingstatus	integer,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (eventid)\n"
		")"
	},
	{
		"gw_eventclosings",
		"eventclosingid",
		NULL,
		"CREATE TABLE gw_eventclosings (\n"
		"	eventclosingid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	closingname	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (eventclosingid)\n"
		")"
	},
	{
		"gw_eventtypes",
		"eventtypeid",
		NULL,
		"CREATE TABLE gw_eventtypes (\n"
		"	eventtypeid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	eventtypename	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (eventtypeid)\n"
		")"
	},
	{
		"gw_files",
		"fileid",
		NULL,
		"CREATE TABLE gw_files (\n"
		"	fileid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	filepath	varchar(255)	NOT NULL,\n"
		"	filename	varchar(255)	NOT NULL,\n"
		"	filetype	varchar(10)	NOT NULL,\n"
		"	uldate		datetime	NOT NULL,\n"
		"	lastdldate	datetime	NOT NULL,\n"
		"	numdownloads	integer		NOT NULL,\n"
		"	description	memo,\n"
		"	PRIMARY KEY (fileid)\n"
		")"
	},
	{
		"gw_forums",
		"forumid",
		NULL,
		"CREATE TABLE gw_forums (\n"
		"	forumid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	forumgroupid	integer,\n"
		"	postername	varchar(50)	NOT NULL,\n"
		"	posttime	datetime	NOT NULL,\n"
		"	subject		varchar(50)	NOT NULL,\n"
		"	message		memo,\n"
		"	PRIMARY KEY (forumid)\n"
		")"
	},
	{
		"gw_forumgroups",
		"forumgroupid",
		NULL,
		"CREATE TABLE gw_forumgroups (\n"
		"	forumgroupid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	title		varchar(50)	NOT NULL,\n"
		"	description	memo,\n"
		"	PRIMARY KEY (forumgroupid)\n"
		")"
	},
	{
		"gw_forumposts",
		"messageid",
		NULL,
		"CREATE TABLE gw_forumposts (\n"
		"	messageid	integer		NOT NULL,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	forumid		integer		NOT NULL,\n"
		"	referenceid	integer		NOT NULL,\n"
		"	postername	varchar(50)	NOT NULL,\n"
		"	posttime	datetime	NOT NULL,\n"
		"	subject		varchar(50)	NOT NULL,\n"
		"	message		memo,\n"
		"	PRIMARY KEY (messageid, forumid)\n"
		")"
	},
	{
		"gw_groups",
		"groupid",
		NULL,
		"CREATE TABLE gw_groups (\n"
		"	groupid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	groupname	varchar(50)	NOT NULL,\n"
		"	availability	varchar(170),\n"
		"	motd		memo,\n"
		"	members		memo,\n"
		"	PRIMARY KEY (groupid)\n"
		")"
	},
	{
		"gw_groupmembers",
		"groupmemberid",
		NULL,
		"CREATE TABLE gw_groupmembers (\n"
		"	groupmemberid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	userid		integer,\n"
		"	groupid		integer,\n"
		"	PRIMARY KEY (groupmemberid)\n"
		")"
	},
	{
		"gw_mailaccounts",
		"mailaccountid",
		NULL,
		"CREATE TABLE gw_mailaccounts (\n"
		"	mailaccountid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	accountname	varchar(50)	NOT NULL,\n"
		"	realname	varchar(50)	NOT NULL,\n"
		"	organization	varchar(50)	NOT NULL,\n"
		"	address		varchar(50)	NOT NULL,\n"
		"	replyto		varchar(50)	NOT NULL,\n"
		"	hosttype	varchar(12)	NOT NULL,\n"
		"	pophost		varchar(50)	NOT NULL,\n"
		"	popport		integer,\n"
		"	smtphost	varchar(50)	NOT NULL,\n"
		"	smtpport	integer,\n"
		"	popusername	varchar(50)	NOT NULL,\n"
		"	poppassword	varchar(50)	NOT NULL,\n"
		"	smtpauth	varchar(10)	NOT NULL,\n"
		"	lastcount	integer,\n"
		"	notify		integer,\n"
		"	remove		integer,\n"
		"	lastcheck	datetime	NOT NULL,\n"
		"	showdebug	varchar(10)	NOT NULL,\n"
		"	signature	memo,\n"
		"	PRIMARY KEY (mailaccountid)\n"
		")"
	},
	{
		"gw_mailfilters",
		"mailfilterid",
		NULL,
		"CREATE TABLE gw_mailfilters (\n"
		"	mailfilterid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	accountid	integer,\n"
		"	filtername	varchar(50)	NOT NULL,\n"
		"	header		varchar(50)	NOT NULL,\n"
		"	string		varchar(50)	NOT NULL,\n"
		"	rule		varchar(10)	NOT NULL,\n"
		"	action		varchar(10)	NOT NULL,\n"
		"	dstfolderid	integer,\n"
		"	PRIMARY KEY (mailfilterid, accountid)\n"
		")"
	},
	{
		"gw_mailfolders",
		"mailfolderid",
		NULL,
		"CREATE TABLE gw_mailfolders (\n"
		"	mailfolderid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	accountid	integer,\n"
		"	parentfolderid	integer,\n"
		"	foldername	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (mailfolderid, accountid)\n"
		")"
	},
	{
		"gw_mailheaders",
		"mailheaderid",
		NULL,
		"CREATE TABLE gw_mailheaders (\n"
		"	mailheaderid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	accountid	integer,\n"
		"	folder		integer,\n"
		"	status		varchar(10)	NOT NULL,\n"
		"	size		integer		NOT NULL,\n"
		"	uidl		varchar(100)	NOT NULL,\n"
		"	hdr_from	varchar(100)	NOT NULL,\n"
		"	hdr_replyto	varchar(100)	NOT NULL,\n"
		"	hdr_to		memo		NOT NULL,\n"
		"	hdr_cc		memo		NOT NULL,\n"
		"	hdr_bcc		memo		NOT NULL,\n"
		"	hdr_subject	varchar(100)	NOT NULL,\n"
		"	hdr_date	datetime	NOT NULL,\n"
		"	hdr_messageid	varchar(100)	NOT NULL,\n"
		"	hdr_inreplyto	varchar(100)	NOT NULL,\n"
		"	hdr_contenttype	varchar(100)	NOT NULL,\n"
		"	hdr_boundary	varchar(100)	NOT NULL,\n"
		"	hdr_encoding	varchar(100)	NOT NULL,\n"
		"	hdr_scanresult	varchar(100)	NOT NULL,\n"
		"	msg_text	memo,\n"
		"	PRIMARY KEY (mailheaderid, accountid)\n"
		")"
	},
	{
		"gw_messages",
		"messageid",
		NULL,
		"CREATE TABLE gw_messages (\n"
		"	messageid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	sender		integer,\n"
		"	rcpt		integer,\n"
		"	status		integer		NOT NULL,\n"
		"	message		memo,\n"
		"	PRIMARY KEY (messageid)\n"
		")"
	},
	{
		"gw_notes",
		"noteid",
		NULL,
		"CREATE TABLE gw_notes (\n"
		"	noteid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	tablename	varchar(50)	NOT NULL,\n"
		"	tableindex	integer,\n"
		"	notetitle	varchar(50)	NOT NULL,\n"
		"	notetext	memo,\n"
		"	PRIMARY KEY (noteid)\n"
		")"
	},
	{
		"gw_orders",
		"orderid",
		NULL,
		"CREATE TABLE gw_orders (\n"
		"	orderid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	contactid	integer		NOT NULL,\n"
		"	userid		integer		NOT NULL,\n"
		"	orderdate	datetime	NOT NULL,\n"
		"	ordertype	varchar(50)	NOT NULL,\n"
		"	paymentmethod	varchar(50)	NOT NULL,\n"
		"	paymentdue	currency	NOT NULL,\n"
		"	paymentreceived	currency	NOT NULL,\n"
		"	status		integer		NOT NULL,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (orderid)\n"
		")"
	},
	{
		"gw_orderitems",
		"orderitemid",
		NULL,
		"CREATE TABLE gw_orderitems (\n"
		"	orderitemid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	orderid		integer		NOT NULL,\n"
		"	productid	integer		NOT NULL,\n"
		"	quantity	float		NOT NULL,\n"
		"	discount	float		NOT NULL,\n"
		"	unitprice	currency	NOT NULL,\n"
		"	internalcost	currency	NOT NULL,\n"
		"	tax1		float		NOT NULL,\n"
		"	tax2		float		NOT NULL,\n"
		"	PRIMARY KEY (orderitemid)\n"
		")"
	},
	{
		"gw_products",
		"productid",
		NULL,
		"CREATE TABLE gw_products (\n"
		"	productid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	productname	varchar(50)	NOT NULL,\n"
		"	category	varchar(50)	NOT NULL,\n"
		"	discount	float		NOT NULL,\n"
		"	unitprice	currency	NOT NULL,\n"
		"	internalcost	currency	NOT NULL,\n"
		"	tax1		float		NOT NULL,\n"
		"	tax2		float		NOT NULL,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (productid)\n"
		")"
	},
	{
		"gw_projects",
		"projectid",
		NULL,
		"CREATE TABLE gw_projects (\n"
		"	projectid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	projectname	varchar(50)	NOT NULL,\n"
		"	projectadmin	integer		NOT NULL,\n"
		"	projectstart	datetime	NOT NULL,\n"
		"	projectfinish	datetime	NOT NULL,\n"
		"	status		integer		NOT NULL,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (projectid)\n"
		")"
	},
	{
		"gw_queries",
		"queryid",
		NULL,
		"CREATE TABLE gw_queries (\n"
		"	queryid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	queryname	varchar(50)	NOT NULL,\n"
		"	query		memo		NOT NULL,\n"
		"	PRIMARY KEY (queryid)\n"
		")"
	},
	{
		"gw_smtp_relayrules",
		"relayruleid",
		NULL,
		"CREATE TABLE gw_smtp_relayrules (\n"
		"	relayruleid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	persistence	varchar(8)	NOT NULL,\n"
		"	ipaddress	varchar(16)	NOT NULL,\n"
		"	PRIMARY KEY (relayruleid)\n"
		")"
	},
	{
		"gw_tasks",
		"taskid",
		NULL,
		"CREATE TABLE gw_tasks (\n"
		"	taskid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	assignedby	integer,\n"
		"	assignedto	integer,\n"
		"	taskname	varchar(50)	NOT NULL,\n"
		"	contactid	integer		NOT NULL,\n"
		"	projectid	integer		NOT NULL,\n"
		"	duedate		datetime	NOT NULL,\n"
		"	priority	integer		NOT NULL,\n"
		"	reminder	integer		NOT NULL,\n"
		"	status		integer		NOT NULL,\n"
		"	details		memo,\n"
		"	PRIMARY KEY (taskid)\n"
		")"
	},
	{
		"gw_users",
		"userid",
		NULL,
		"CREATE TABLE gw_users (\n"
		"	userid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	username	varchar(50)	NOT NULL,\n"
		"	password	varchar(50),\n"
		"	groupid		integer		NOT NULL,\n"
		"	domainid	integer		NOT NULL,\n"
		"	enabled		integer		NOT NULL,\n"
		"	authdomainadmin	integer		NOT NULL,\n"
		"	authadmin	integer		NOT NULL,\n"
		"	authbookmarks	integer		NOT NULL,\n"
		"	authcalendar	integer		NOT NULL,\n"
		"	authcalls	integer		NOT NULL,\n"
		"	authcontacts	integer		NOT NULL,\n"
		"	authfiles	integer		NOT NULL,\n"
		"	authforums	integer		NOT NULL,\n"
		"	authmessages	integer		NOT NULL,\n"
		"	authorders	integer		NOT NULL,\n"
		"	authprofile	integer		NOT NULL,\n"
		"	authprojects	integer		NOT NULL,\n"
		"	authquery	integer		NOT NULL,\n"
		"	authwebmail	integer		NOT NULL,\n"
		"	prefdaystart	integer		NOT NULL,\n"
		"	prefdaylength	integer		NOT NULL,\n"
		"	prefmailcurrent	integer		NOT NULL,\n"
		"	prefmaildefault	integer		NOT NULL,\n"
		"	prefmaxlist	integer		NOT NULL,\n"
		"	prefmenustyle	integer		NOT NULL,\n"
		"	preftimezone	integer		NOT NULL,\n"
		"	prefgeozone	integer		NOT NULL,\n"
		"	preflanguage	varchar(4)	NOT NULL,\n"
		"	preftheme	varchar(40)	NOT NULL,\n"
		"	availability	varchar(170),\n"
		"	surname		varchar(50),\n"
		"	givenname	varchar(50),\n"
		"	jobtitle	varchar(50),\n"
		"	division	varchar(50),\n"
		"	supervisor	varchar(50),\n"
		"	address		varchar(50),\n"
		"	locality	varchar(50),\n"
		"	region		varchar(50),\n"
		"	country		varchar(50),\n"
		"	postalcode	varchar(10),\n"
		"	homenumber	varchar(25),\n"
		"	worknumber	varchar(25),\n"
		"	faxnumber	varchar(25),\n"
		"	cellnumber	varchar(25),\n"
		"	pagernumber	varchar(25),\n"
		"	email		varchar(50),\n"
		"	birthdate	datetime,\n"
		"	hiredate	datetime,\n"
		"	sin		varchar(15),\n"
		"	isactive	varchar(10),\n"
		"	PRIMARY KEY (userid)\n"
		")"
	},
	{
		"gw_usersessions",
		"sessionid",
		NULL,
		"CREATE TABLE gw_usersessions (\n"
		"	sessionid	autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	remoteip	varchar(20),\n"
		"	token		varchar(50),\n"
		"	userid		integer,\n"
		"	domainid	integer,\n"
		"	PRIMARY KEY (sessionid)\n"
		")"
	},
	{
		"gw_zones",
		"zoneid",
		NULL,
		"CREATE TABLE gw_zones (\n"
		"	zoneid		autoincrement,\n"
		"	obj_ctime	datetime	NOT NULL,\n"
		"	obj_mtime	datetime	NOT NULL,\n"
		"	obj_uid		integer,\n"
		"	obj_gid		integer,\n"
		"	obj_did		integer,\n"
		"	obj_gperm	integer,\n"
		"	obj_operm	integer,\n"
		"	zonename	varchar(50)	NOT NULL,\n"
		"	PRIMARY KEY (zoneid)\n"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};