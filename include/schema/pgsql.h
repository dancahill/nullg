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

static const tabledef pgsqldb_tables[] = {
	{
		"gw_dbinfo",
		"dbversion",
		NULL,
		"CREATE TABLE gw_dbinfo (\n"
		"	dbversion	varchar(10)	NOT NULL,\n"
		"	tax1name	varchar(20)	NOT NULL,\n"
		"	tax2name	varchar(20)	NOT NULL,\n"
		"	tax1percent	numeric(9,3)	NOT NULL DEFAULT '0.080',\n"
		"	tax2percent	numeric(9,3)	NOT NULL DEFAULT '0.070'\n"
		")"
	},
	{
		"gw_accounting_accounts",
		"accountid",
		"accaid_seq",
		"CREATE TABLE gw_accounting_accounts (\n"
		"	accountid	int4		NOT NULL DEFAULT nextval('accaid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	accountname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (accountid)\n"
		")"
	},
	{
		"gw_accounting_journal",
		"journalentryid",
		"accjid_seq",
		"CREATE TABLE gw_accounting_journal (\n"
		"	journalentryid	int4		NOT NULL DEFAULT nextval('accjid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	entrydate	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	accountid	int4		NOT NULL DEFAULT 0,\n"
		"	debit		numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	credit		numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	details		varchar(250)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (journalentryid)\n"
		")"
	},
	{
		"gw_activity",
		"activityid",
		"actiid_seq",
		"CREATE TABLE gw_activity (\n"
		"	activityid	int4		NOT NULL DEFAULT nextval('actiid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	userid		int4		NOT NULL DEFAULT 0,\n"
		"	clientip	varchar(50)	NOT NULL DEFAULT '',\n"
		"	category	varchar(50)	NOT NULL DEFAULT '',\n"
		"	indexid		int4		NOT NULL DEFAULT 0,\n"
		"	action		varchar(50)	NOT NULL DEFAULT '',\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (activityid)\n"
		")"
	},
	{
		"gw_bookmarks",
		"bookmarkid",
		"bkmkid_seq",
		"CREATE TABLE gw_bookmarks (\n"
		"	bookmarkid	int4		NOT NULL DEFAULT nextval('bkmkid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	folderid	int4		NOT NULL DEFAULT 0,\n"
		"	bookmarkname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	bookmarkurl	varchar(255)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (bookmarkid)\n"
		")"
	},
	{
		"gw_bookmarkfolders",
		"folderid",
		"bfldid_seq",
		"CREATE TABLE gw_bookmarkfolders (\n"
		"	folderid	int4		NOT NULL DEFAULT nextval('bfldid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	parentid	int4		NOT NULL DEFAULT 0,\n"
		"	foldername	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (folderid)\n"
		")"
	},
	{
		"gw_calls",
		"callid",
		"callid_seq",
		"CREATE TABLE gw_calls (\n"
		"	callid		int4		NOT NULL DEFAULT nextval('callid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	assignedby	int4		NOT NULL DEFAULT 0,\n"
		"	assignedto	int4		NOT NULL DEFAULT 0,\n"
		"	callname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	callstart	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	callfinish	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	contactid	int4		NOT NULL DEFAULT 0,\n"
		"	action		int4		NOT NULL DEFAULT 0,\n"
		"	status		int4		NOT NULL DEFAULT 0,\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (callid)\n"
		")"
	},
	{
		"gw_callactions",
		"callactionid",
		"calaid_seq",
		"CREATE TABLE gw_callactions (\n"
		"	callactionid	int4		NOT NULL DEFAULT nextval('calaid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	actionname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (callactionid)\n"
		")"
	},
	{
		"gw_contacts",
		"contactid",
		"contid_seq",
		"CREATE TABLE gw_contacts (\n"
		"	contactid	int4		NOT NULL DEFAULT nextval('contid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	folderid	int4		NOT NULL DEFAULT 0,\n"
		"	username	varchar(50)	NOT NULL DEFAULT '',\n"
		"	password	varchar(50)	NOT NULL DEFAULT '',\n"
		"	enabled		int4		NOT NULL DEFAULT 0,\n"
		"	geozone		int4		NOT NULL DEFAULT 0,\n"
		"	timezone	int4		NOT NULL DEFAULT 0,\n"
		"	surname		varchar(50)	NOT NULL DEFAULT '',\n"
		"	givenname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	salutation	varchar(50)	NOT NULL DEFAULT '',\n"
		"	contacttype	varchar(50)	NOT NULL DEFAULT '',\n"
		"	referredby	varchar(50)	NOT NULL DEFAULT '',\n"
		"	altcontact	varchar(50)	NOT NULL DEFAULT '',\n"
		"	prefbilling	varchar(50)	NOT NULL DEFAULT '',\n"
		"	website		varchar(250)	NOT NULL DEFAULT '',\n"
		"	email		varchar(250)	NOT NULL DEFAULT '',\n"
		"	homenumber	varchar(25)	NOT NULL DEFAULT '',\n"
		"	worknumber	varchar(25)	NOT NULL DEFAULT '',\n"
		"	faxnumber	varchar(25)	NOT NULL DEFAULT '',\n"
		"	mobilenumber	varchar(25)	NOT NULL DEFAULT '',\n"
		"	jobtitle	varchar(50)	NOT NULL DEFAULT '',\n"
		"	organization	varchar(50)	NOT NULL DEFAULT '',\n"
		"	homeaddress	varchar(50)	NOT NULL DEFAULT '',\n"
		"	homelocality	varchar(50)	NOT NULL DEFAULT '',\n"
		"	homeregion	varchar(50)	NOT NULL DEFAULT '',\n"
		"	homecountry	varchar(50)	NOT NULL DEFAULT '',\n"
		"	homepostalcode	varchar(10)	NOT NULL DEFAULT '',\n"
		"	workaddress	varchar(50)	NOT NULL DEFAULT '',\n"
		"	worklocality	varchar(50)	NOT NULL DEFAULT '',\n"
		"	workregion	varchar(50)	NOT NULL DEFAULT '',\n"
		"	workcountry	varchar(50)	NOT NULL DEFAULT '',\n"
		"	workpostalcode	varchar(10)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (contactid)\n"
		")"
	},
	{
		"gw_contactsessions",
		"sessionid",
		"csesid_seq",
		"CREATE TABLE gw_contactsessions (\n"
		"	sessionid	int4		NOT NULL DEFAULT nextval('csesid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	remoteip	varchar(20)	NOT NULL DEFAULT '0.0.0.0',\n"
		"	token		varchar(50)	NOT NULL DEFAULT '',\n"
		"	contactid	int4		NOT NULL DEFAULT 0,\n"
		"	domainid	int4		NOT NULL DEFAULT 0,\n"
		"	PRIMARY KEY (sessionid)\n"
		")"
	},
	{
		"gw_domains",
		"domainid",
		"domaid_seq",
		"CREATE TABLE gw_domains (\n"
		"	domainid	int4		NOT NULL DEFAULT nextval('domaid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	domainname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (domainid)\n"
		")"
	},
	{
		"gw_domainaliases",
		"domainaliasid",
		"domlid_seq",
		"CREATE TABLE gw_domainaliases (\n"
		"	domainaliasid	int4		NOT NULL DEFAULT nextval('domlid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	domainid	int4		NOT NULL DEFAULT 0,\n"
		"	domainname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (domainaliasid)\n"
		")"
	},
	{
		"gw_events",
		"eventid",
		"evenid_seq",
		"CREATE TABLE gw_events (\n"
		"	eventid		int4		NOT NULL DEFAULT nextval('evenid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	assignedby	int4		NOT NULL DEFAULT 0,\n"
		"	assignedto	int4		NOT NULL DEFAULT 0,\n"
		"	eventname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	eventtype	int4		NOT NULL DEFAULT 0,\n"
		"	contactid	int4		NOT NULL DEFAULT 0,\n"
		"	projectid	int4		NOT NULL DEFAULT 0,\n"
		"	priority	int4		NOT NULL DEFAULT 0,\n"
		"	reminder	int4		NOT NULL DEFAULT 0,\n"
		"	eventstart	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	eventfinish	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	busy		int4		NOT NULL DEFAULT 0,\n"
		"	status		int4		NOT NULL DEFAULT 0,\n"
		"	closingstatus	int4		NOT NULL DEFAULT 0,\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (eventid)\n"
		")"
	},
	{
		"gw_eventclosings",
		"eventclosingid",
		"ecloid_seq",
		"CREATE TABLE gw_eventclosings (\n"
		"	eventclosingid	int4		NOT NULL DEFAULT nextval('ecloid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	closingname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (eventclosingid)\n"
		")"
	},
	{
		"gw_eventtypes",
		"eventtypeid",
		"etypid_seq",
		"CREATE TABLE gw_eventtypes (\n"
		"	eventtypeid	int4		NOT NULL DEFAULT nextval('etypid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	eventtypename	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (eventtypeid)\n"
		")"
	},
	{
		"gw_files",
		"fileid",
		"fileid_seq",
		"CREATE TABLE gw_files (\n"
		"	fileid		int4		NOT NULL DEFAULT nextval('fileid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	filepath	varchar(255)	NOT NULL DEFAULT '',\n"
		"	filename	varchar(255)	NOT NULL DEFAULT '',\n"
		"	filetype	varchar(10)	NOT NULL DEFAULT '',\n"
		"	uldate		timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	lastdldate	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	numdownloads	int4		NOT NULL DEFAULT 0,\n"
		"	description	text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (fileid)\n"
		")"
	},
	{
		"gw_forums",
		"forumid",
		"foruid_seq",
		"CREATE TABLE gw_forums (\n"
		"	forumid		int4		NOT NULL DEFAULT nextval('foruid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	forumgroupid	int4		NOT NULL DEFAULT 0,\n"
		"	postername	varchar(50)	NOT NULL DEFAULT '',\n"
		"	posttime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	subject		varchar(50)	NOT NULL DEFAULT '',\n"
		"	message		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (forumid)\n"
		")"
	},
	{
		"gw_forumgroups",
		"forumgroupid",
		"forgid_seq",
		"CREATE TABLE gw_forumgroups (\n"
		"	forumgroupid	int4		NOT NULL DEFAULT nextval('forgid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	title		varchar(50)	NOT NULL DEFAULT '',\n"
		"	description	text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (forumgroupid)\n"
		")"
	},
	{
		"gw_forumposts",
		"messageid",
		NULL,
		"CREATE TABLE gw_forumposts (\n"
		"	messageid	int4		NOT NULL DEFAULT 0,\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	forumid		int4		NOT NULL DEFAULT 0,\n"
		"	referenceid	int4		NOT NULL DEFAULT 0,\n"
		"	postername	varchar(50)	NOT NULL DEFAULT '',\n"
		"	posttime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	subject		varchar(50)	NOT NULL DEFAULT '',\n"
		"	message		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (messageid, forumid)\n"
		")"
	},
	{
		"gw_groups",
		"groupid",
		"grouid_seq",
		"CREATE TABLE gw_groups (\n"
		"	groupid		int4		NOT NULL DEFAULT nextval('grouid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	groupname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	availability	varchar(170)	NOT NULL DEFAULT '',\n"
		"	motd		text		NOT NULL DEFAULT '',\n"
		"	members		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (groupid)\n"
		")"
	},
	{
		"gw_groupmembers",
		"groupmemberid",
		"grpmid_seq",
		"CREATE TABLE gw_groupmembers (\n"
		"	groupmemberid	int4		NOT NULL DEFAULT nextval('grpmid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	userid		int4		NOT NULL DEFAULT 0,\n"
		"	groupid		int4		NOT NULL DEFAULT 0,\n"
		"	PRIMARY KEY (groupmemberid)\n"
		")"
	},
	{
		"gw_mailaccounts",
		"mailaccountid",
		"mailid_seq",
		"CREATE TABLE gw_mailaccounts (\n"
		"	mailaccountid	int4		NOT NULL DEFAULT nextval('mailid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	accountname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	realname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	organization	varchar(50)	NOT NULL DEFAULT '',\n"
		"	address		varchar(50)	NOT NULL DEFAULT '',\n"
		"	replyto		varchar(50)	NOT NULL DEFAULT '',\n"
		"	hosttype	varchar(12)	NOT NULL DEFAULT '',\n"
		"	pophost		varchar(50)	NOT NULL DEFAULT '',\n"
		"	popport		int4		NOT NULL DEFAULT 110,\n"
		"	smtphost	varchar(50)	NOT NULL DEFAULT '',\n"
		"	smtpport	int4		NOT NULL DEFAULT 25,\n"
		"	popusername	varchar(50)	NOT NULL DEFAULT '',\n"
		"	poppassword	varchar(50)	NOT NULL DEFAULT '',\n"
		"	smtpauth	varchar(10)	NOT NULL DEFAULT 'n',\n"
		"	lastcount	int4		NOT NULL DEFAULT 0,\n"
		"	notify		int4		NOT NULL DEFAULT 0,\n"
		"	remove		int4		NOT NULL DEFAULT 0,\n"
		"	lastcheck	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	showdebug	varchar(10)	NOT NULL DEFAULT 'n',\n"
		"	signature	text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (mailaccountid)\n"
		")"
	},
	{
		"gw_mailfilters",
		"mailfilterid",
		"mfilid_seq",
		"CREATE TABLE gw_mailfilters (\n"
		"	mailfilterid	int4		NOT NULL DEFAULT nextval('mfilid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	accountid	int4		NOT NULL DEFAULT 0,\n"
		"	filtername	varchar(50)	NOT NULL DEFAULT '',\n"
		"	header		varchar(50)	NOT NULL DEFAULT '',\n"
		"	string		varchar(50)	NOT NULL DEFAULT '',\n"
		"	rule		varchar(10)	NOT NULL DEFAULT '',\n"
		"	action		varchar(10)	NOT NULL DEFAULT '',\n"
		"	dstfolderid	int4		NOT NULL DEFAULT 1,\n"
		"	PRIMARY KEY (mailfilterid, accountid)\n"
		")"
	},
	{
		"gw_mailfolders",
		"mailfolderid",
		"mfdrid_seq",
		"CREATE TABLE gw_mailfolders (\n"
		"	mailfolderid	int4		NOT NULL DEFAULT nextval('mfdrid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	accountid	int4		NOT NULL DEFAULT 0,\n"
		"	parentfolderid	int4		NOT NULL DEFAULT 0,\n"
		"	foldername	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (mailfolderid, accountid)\n"
		")"
	},
	{
		"gw_mailheaders",
		"mailheaderid",
		"mhdrid_seq",
		"CREATE TABLE gw_mailheaders (\n"
		"	mailheaderid	int4		NOT NULL DEFAULT nextval('mhdrid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	accountid	int4		NOT NULL DEFAULT 0,\n"
		"	folder		int4		NOT NULL DEFAULT 1,\n"
		"	status		varchar(10)	NOT NULL DEFAULT 'n',\n"
		"	size		int4		NOT NULL DEFAULT -1,\n"
		"	uidl		varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_from	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_replyto	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_to		text		NOT NULL DEFAULT '',\n"
		"	hdr_cc		text		NOT NULL DEFAULT '',\n"
		"	hdr_bcc		text		NOT NULL DEFAULT '',\n"
		"	hdr_subject	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_date	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	hdr_messageid	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_inreplyto	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_contenttype	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_boundary	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_encoding	varchar(100)	NOT NULL DEFAULT '',\n"
		"	hdr_scanresult	varchar(100)	NOT NULL DEFAULT '',\n"
		"	msg_text	text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (mailheaderid, accountid)\n"
		")"
	},
	{
		"gw_messages",
		"messageid",
		"messid_seq",
		"CREATE TABLE gw_messages (\n"
		"	messageid	int4		NOT NULL DEFAULT nextval('messid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	sender		int4		NOT NULL DEFAULT 0,\n"
		"	rcpt		int4		NOT NULL DEFAULT 0,\n"
		"	status		int4		NOT NULL DEFAULT 2,\n"
		"	message		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (messageid)\n"
		")"
	},
	{
		"gw_notes",
		"noteid",
		"noteid_seq",
		"CREATE TABLE gw_notes (\n"
		"	noteid		int4		NOT NULL DEFAULT nextval('noteid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	tablename	varchar(50)	NOT NULL DEFAULT '',\n"
		"	tableindex	int4		NOT NULL DEFAULT 0,\n"
		"	notetitle	varchar(50)	NOT NULL DEFAULT '',\n"
		"	notetext	text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (noteid)\n"
		")"
	},
	{
		"gw_orders",
		"orderid",
		"ordeid_seq",
		"CREATE TABLE gw_orders (\n"
		"	orderid		int4		NOT NULL DEFAULT nextval('ordeid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	contactid	int4		NOT NULL DEFAULT 0,\n"
		"	userid		int4		NOT NULL DEFAULT 0,\n"
		"	orderdate	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	ordertype	varchar(50)	NOT NULL DEFAULT '',\n"
		"	paymentmethod	varchar(50)	NOT NULL DEFAULT '',\n"
		"	paymentdue	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	paymentreceived	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	status		int4		NOT NULL DEFAULT 0,\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (orderid)\n"
		")"
	},
	{
		"gw_orderitems",
		"orderitemid",
		"ordiid_seq",
		"CREATE TABLE gw_orderitems (\n"
		"	orderitemid	int4		NOT NULL DEFAULT nextval('ordiid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	orderid		int4		NOT NULL DEFAULT 0,\n"
		"	productid	int4		NOT NULL DEFAULT 0,\n"
		"	quantity	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	discount	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	unitprice	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	internalcost	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	tax1		numeric(9,3)	NOT NULL DEFAULT '0.08',\n"
		"	tax2		numeric(9,3)	NOT NULL DEFAULT '0.07',\n"
		"	PRIMARY KEY (orderitemid)\n"
		")"
	},
	{
		"gw_products",
		"productid",
		"prodid_seq",
		"CREATE TABLE gw_products (\n"
		"	productid	int4		NOT NULL DEFAULT nextval('prodid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	productname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	category	varchar(50)	NOT NULL DEFAULT '',\n"
		"	discount	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	unitprice	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	internalcost	numeric(9,2)	NOT NULL DEFAULT '0.00',\n"
		"	tax1		numeric(9,3)	NOT NULL DEFAULT '0.08',\n"
		"	tax2		numeric(9,3)	NOT NULL DEFAULT '0.07',\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (productid)\n"
		")"
	},
	{
		"gw_projects",
		"projectid",
		"projid_seq",
		"CREATE TABLE gw_projects (\n"
		"	projectid	int4		NOT NULL DEFAULT nextval('projid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	projectname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	projectadmin	int4		NOT NULL DEFAULT 0,\n"
		"	projectstart	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	projectfinish	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	status		int4		NOT NULL DEFAULT 0,\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (projectid)\n"
		")"
	},
	{
		"gw_queries",
		"queryid",
		"querid_seq",
		"CREATE TABLE gw_queries (\n"
		"	queryid		int4		NOT NULL DEFAULT nextval('querid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	queryname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	query		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (queryid)\n"
		")"
	},
	{
		"gw_smtp_relayrules",
		"relayruleid",
		"smtrid_seq",
		"CREATE TABLE gw_smtp_relayrules (\n"
		"	relayruleid	int4		NOT NULL DEFAULT nextval('smtrid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	persistence	varchar(8)	NOT NULL DEFAULT '',\n"
		"	ipaddress	varchar(16)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (relayruleid)\n"
		")"
	},
	{
		"gw_tasks",
		"taskid",
		"taskid_seq",
		"CREATE TABLE gw_tasks (\n"
		"	taskid		int4		NOT NULL DEFAULT nextval('taskid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	assignedby	int4		NOT NULL DEFAULT 0,\n"
		"	assignedto	int4		NOT NULL DEFAULT 0,\n"
		"	taskname	varchar(50)	NOT NULL DEFAULT '',\n"
		"	contactid	int4		NOT NULL DEFAULT 0,\n"
		"	projectid	int4		NOT NULL DEFAULT 0,\n"
		"	duedate		timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	priority	int4		NOT NULL DEFAULT 0,\n"
		"	reminder	int4		NOT NULL DEFAULT 0,\n"
		"	status		int4		NOT NULL DEFAULT 0,\n"
		"	details		text		NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (taskid)\n"
		")"
	},
	{
		"gw_users",
		"userid",
		"userid_seq",
		"CREATE TABLE gw_users (\n"
		"	userid		int4		NOT NULL DEFAULT nextval('userid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	username	varchar(50)	NOT NULL DEFAULT '',\n"
		"	password	varchar(50)	NOT NULL DEFAULT '',\n"
		"	groupid		int4		NOT NULL DEFAULT 0,\n"
		"	domainid	int4		NOT NULL DEFAULT 0,\n"
		"	enabled		int4		NOT NULL DEFAULT 0,\n"
		"	authdomainadmin	int4		NOT NULL DEFAULT 0,\n"
		"	authadmin	int4		NOT NULL DEFAULT 0,\n"
		"	authbookmarks	int4		NOT NULL DEFAULT 0,\n"
		"	authcalendar	int4		NOT NULL DEFAULT 0,\n"
		"	authcalls	int4		NOT NULL DEFAULT 0,\n"
		"	authcontacts	int4		NOT NULL DEFAULT 0,\n"
		"	authfiles	int4		NOT NULL DEFAULT 0,\n"
		"	authforums	int4		NOT NULL DEFAULT 0,\n"
		"	authmessages	int4		NOT NULL DEFAULT 0,\n"
		"	authorders	int4		NOT NULL DEFAULT 0,\n"
		"	authprofile	int4		NOT NULL DEFAULT 0,\n"
		"	authprojects	int4		NOT NULL DEFAULT 0,\n"
		"	authquery	int4		NOT NULL DEFAULT 0,\n"
		"	authwebmail	int4		NOT NULL DEFAULT 0,\n"
		"	prefdaystart	int4		NOT NULL DEFAULT 0,\n"
		"	prefdaylength	int4		NOT NULL DEFAULT 0,\n"
		"	prefmailcurrent	int4		NOT NULL DEFAULT 0,\n"
		"	prefmaildefault	int4		NOT NULL DEFAULT 0,\n"
		"	prefmaxlist	int4		NOT NULL DEFAULT 0,\n"
		"	prefmenustyle	int4		NOT NULL DEFAULT 1,\n"
		"	preftimezone	int4		NOT NULL DEFAULT 0,\n"
		"	prefgeozone	int4		NOT NULL DEFAULT 0,\n"
		"	preflanguage	varchar(4)	NOT NULL DEFAULT 'en',\n"
		"	preftheme	varchar(40)	NOT NULL DEFAULT 'default',\n"
		"	availability	varchar(170)	NOT NULL DEFAULT '',\n"
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
		"	birthdate	date		NOT NULL DEFAULT '1970-01-01',\n"
		"	hiredate	date		NOT NULL DEFAULT '1970-01-01',\n"
		"	sin		varchar(15),\n"
		"	isactive	varchar(10),\n"
		"	PRIMARY KEY (userid)\n"
		")"
	},
	{
		"gw_usersessions",
		"sessionid",
		"usesid_seq",
		"CREATE TABLE gw_usersessions (\n"
		"	sessionid	int4		NOT NULL DEFAULT nextval('usesid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	remoteip	varchar(20)	NOT NULL DEFAULT '0.0.0.0',\n"
		"	token		varchar(50)	NOT NULL DEFAULT '',\n"
		"	userid		int4		NOT NULL DEFAULT 0,\n"
		"	domainid	int4		NOT NULL DEFAULT 0,\n"
		"	PRIMARY KEY (sessionid)\n"
		")"
	},
	{
		"gw_zones",
		"zoneid",
		"zoneid_seq",
		"CREATE TABLE gw_zones (\n"
		"	zoneid		int4		NOT NULL DEFAULT nextval('zoneid_seq'::text),\n"
		"	obj_ctime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_mtime	timestamp	NOT NULL DEFAULT '1970-01-01 00:00:00',\n"
		"	obj_uid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gid		int4		NOT NULL DEFAULT 0,\n"
		"	obj_did		int4		NOT NULL DEFAULT 0,\n"
		"	obj_gperm	int4		NOT NULL DEFAULT 0,\n"
		"	obj_operm	int4		NOT NULL DEFAULT 0,\n"
		"	zonename	varchar(50)	NOT NULL DEFAULT '',\n"
		"	PRIMARY KEY (zoneid)\n"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};
