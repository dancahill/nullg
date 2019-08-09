/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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

static const tabledef fbsqldb_tables[] = {
	{
		"gw_dbinfo",
		"dbversion",
		NULL,
		"CREATE TABLE gw_dbinfo (\n"
		"	dbversion	varchar(10)	NOT NULL,\n"
		"	tax1name	varchar(20)	NOT NULL,\n"
		"	tax2name	varchar(20)	NOT NULL,\n"
		"	tax1percent	numeric(9,3)	DEFAULT '0.080' NOT NULL,\n"
		"	tax2percent	numeric(9,3)	DEFAULT '0.070' NOT NULL\n"
		")"
	},
	{
		"gw_activity",
		"activityid",
		"actiid_seq",
		"CREATE TABLE gw_activity (\n"
		"	activityid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	userid		integer		DEFAULT 0 NOT NULL,\n"
		"	clientip	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	category	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	indexid		integer		DEFAULT 0 NOT NULL,\n"
		"	action		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (activityid)\n"
		")"
	},
	{
		"gw_bookmarks",
		"bookmarkid",
		"bkmkid_seq",
		"CREATE TABLE gw_bookmarks (\n"
		"	bookmarkid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	folderid	integer		DEFAULT 0 NOT NULL,\n"
		"	bookmarkname	varchar(100)	DEFAULT '' NOT NULL,\n"
		"	bookmarkurl	varchar(255)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (bookmarkid)\n"
		")"
	},
	{
		"gw_bookmarks_folders",
		"folderid",
		"bfldid_seq",
		"CREATE TABLE gw_bookmarks_folders (\n"
		"	folderid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	parentid	integer		DEFAULT 0 NOT NULL,\n"
		"	foldername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (folderid)\n"
		")"
	},
	{
		"gw_calls",
		"callid",
		"callid_seq",
		"CREATE TABLE gw_calls (\n"
		"	callid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	assignedby	integer		DEFAULT 0 NOT NULL,\n"
		"	assignedto	integer		DEFAULT 0 NOT NULL,\n"
		"	callname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	callstart	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	callfinish	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	contactid	integer		DEFAULT 0 NOT NULL,\n"
		"	action		integer		DEFAULT 0 NOT NULL,\n"
		"	status		integer		DEFAULT 0 NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (callid)\n"
		")"
	},
	{
		"gw_calls_actions",
		"actionid",
		"calaid_seq",
		"CREATE TABLE gw_calls_actions (\n"
		"	actionid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	actionname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (actionid)\n"
		")"
	},
	{
		"gw_contacts",
		"contactid",
		"contid_seq",
		"CREATE TABLE gw_contacts (\n"
		"	contactid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	folderid	integer		DEFAULT 0 NOT NULL,\n"
		"	username	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	password	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	enabled		integer		DEFAULT 0 NOT NULL,\n"
		"	geozone		integer		DEFAULT 0 NOT NULL,\n"
		"	timezone	integer		DEFAULT 0 NOT NULL,\n"
		"	surname		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	givenname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	salutation	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	contacttype	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	referredby	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	altcontact	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	prefbilling	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	website		varchar(250)	DEFAULT '' NOT NULL,\n"
		"	email		varchar(250)	DEFAULT '' NOT NULL,\n"
		"	homenumber	varchar(25)	DEFAULT '' NOT NULL,\n"
		"	worknumber	varchar(25)	DEFAULT '' NOT NULL,\n"
		"	faxnumber	varchar(25)	DEFAULT '' NOT NULL,\n"
		"	mobilenumber	varchar(25)	DEFAULT '' NOT NULL,\n"
		"	jobtitle	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	organization	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	homeaddress	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	homelocality	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	homeregion	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	homecountry	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	homepostalcode	varchar(10)	DEFAULT '' NOT NULL,\n"
		"	workaddress	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	worklocality	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	workregion	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	workcountry	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	workpostalcode	varchar(10)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (contactid)\n"
		")"
	},
	{
		"gw_contacts_folders",
		"folderid",
		"cfldid_seq",
		"CREATE TABLE gw_contacts_folders (\n"
		"	folderid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	parentid	integer		DEFAULT 0 NOT NULL,\n"
		"	foldername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (folderid)\n"
		")"
	},
	{
		"gw_contacts_sessions",
		"sessionid",
		"csesid_seq",
		"CREATE TABLE gw_contacts_sessions (\n"
		"	sessionid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	remoteip	varchar(20)	DEFAULT '0.0.0.0' NOT NULL,\n"
		"	token		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	contactid	integer		DEFAULT 0 NOT NULL,\n"
		"	domainid	integer		DEFAULT 0 NOT NULL,\n"
		"	PRIMARY KEY (sessionid)\n"
		")"
	},
	{
		"gw_domains",
		"domainid",
		"domaid_seq",
		"CREATE TABLE gw_domains (\n"
		"	domainid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	domainname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (domainid)\n"
		")"
	},
	{
		"gw_domains_aliases",
		"domainaliasid",
		"domlid_seq",
		"CREATE TABLE gw_domains_aliases (\n"
		"	domainaliasid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	domainid	integer		DEFAULT 0 NOT NULL,\n"
		"	domainname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (domainaliasid)\n"
		")"
	},
	{
		"gw_email_accounts",
		"mailaccountid",
		"mailid_seq",
		"CREATE TABLE gw_email_accounts (\n"
		"	mailaccountid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	accountname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	realname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	organization	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	address		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	replyto		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	hosttype	varchar(12)	DEFAULT '' NOT NULL,\n"
		"	pophost		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	popport		integer		DEFAULT 110 NOT NULL,\n"
		"	popssl		integer		DEFAULT 0 NOT NULL,\n"
		"	smtphost	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	smtpport	integer		DEFAULT 25 NOT NULL,\n"
		"	smtpssl		integer		DEFAULT 0 NOT NULL,\n"
		"	popusername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	poppassword	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	smtpauth	varchar(10)	DEFAULT 'n' NOT NULL,\n"
		"	lastcount	integer		DEFAULT 0 NOT NULL,\n"
		"	notify		integer		DEFAULT 0 NOT NULL,\n"
		"	remove		integer		DEFAULT 0 NOT NULL,\n"
		"	lastcheck	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	showdebug	varchar(10)	DEFAULT 'n' NOT NULL,\n"
		"	signature	blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (mailaccountid)\n"
		")"
	},
	{
		"gw_email_filters",
		"mailfilterid",
		"mfilid_seq",
		"CREATE TABLE gw_email_filters (\n"
		"	mailfilterid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	accountid	integer		DEFAULT 0 NOT NULL,\n"
		"	filtername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	header		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	string		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	rule		varchar(10)	DEFAULT '' NOT NULL,\n"
		"	action		varchar(10)	DEFAULT '' NOT NULL,\n"
		"	dstfolderid	integer		DEFAULT 1 NOT NULL,\n"
		"	PRIMARY KEY (mailfilterid, accountid)\n"
		")"
	},
	{
		"gw_email_folders",
		"mailfolderid",
		"mfdrid_seq",
		"CREATE TABLE gw_email_folders (\n"
		"	mailfolderid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	accountid	integer		DEFAULT 0 NOT NULL,\n"
		"	parentfolderid	integer		DEFAULT 0 NOT NULL,\n"
		"	foldername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (mailfolderid, accountid)\n"
		")"
	},
	{
		"gw_email_headers",
		"mailheaderid",
		"mhdrid_seq",
		"CREATE TABLE gw_email_headers (\n"
		"	mailheaderid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	accountid	integer		DEFAULT 0 NOT NULL,\n"
		"	folder		integer		DEFAULT 1 NOT NULL,\n"
		"	status		varchar(10)	DEFAULT 'n' NOT NULL,\n"
		"	size		integer		DEFAULT -1 NOT NULL,\n"
		"	uidl		varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_from	varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_replyto	varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_to		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	hdr_cc		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	hdr_bcc		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	hdr_subject	varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_date	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	hdr_messageid	varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_inreplyto	varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_contenttype	varchar(200)	DEFAULT '' NOT NULL,\n"
		"	hdr_boundary	varchar(100)	DEFAULT '' NOT NULL,\n"
		"	hdr_encoding	varchar(100)	DEFAULT '' NOT NULL,\n"
		"	hdr_scanresult	varchar(100)	DEFAULT '' NOT NULL,\n"
		"	msg_text	blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (mailheaderid, accountid)\n"
		")"
	},
	{
		"gw_events",
		"eventid",
		"evenid_seq",
		"CREATE TABLE gw_events (\n"
		"	eventid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	assignedby	integer		DEFAULT 0 NOT NULL,\n"
		"	assignedto	integer		DEFAULT 0 NOT NULL,\n"
		"	eventname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	eventtype	integer		DEFAULT 0 NOT NULL,\n"
		"	contactid	integer		DEFAULT 0 NOT NULL,\n"
		"	projectid	integer		DEFAULT 0 NOT NULL,\n"
		"	priority	integer		DEFAULT 0 NOT NULL,\n"
		"	reminder	integer		DEFAULT 0 NOT NULL,\n"
		"	eventstart	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	eventfinish	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	busy		integer		DEFAULT 0 NOT NULL,\n"
		"	status		integer		DEFAULT 0 NOT NULL,\n"
		"	closingstatus	integer		DEFAULT 0 NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (eventid)\n"
		")"
	},
	{
		"gw_events_closings",
		"eventclosingid",
		"ecloid_seq",
		"CREATE TABLE gw_events_closings (\n"
		"	eventclosingid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	closingname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (eventclosingid)\n"
		")"
	},
	{
		"gw_events_types",
		"eventtypeid",
		"etypid_seq",
		"CREATE TABLE gw_events_types (\n"
		"	eventtypeid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	eventtypename	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (eventtypeid)\n"
		")"
	},
	{
		"gw_files",
		"fileid",
		"fileid_seq",
		"CREATE TABLE gw_files (\n"
		"	fileid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	filepath	varchar(255)	DEFAULT '' NOT NULL,\n"
		"	filename	varchar(255)	DEFAULT '' NOT NULL,\n"
		"	filetype	varchar(10)	DEFAULT '' NOT NULL,\n"
		"	uldate		timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	lastdldate	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	numdownloads	integer		DEFAULT 0 NOT NULL,\n"
		"	description	blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (fileid),\n"
		"	UNIQUE (obj_did, filepath, filename)\n"
		")"
	},
	{
		"gw_finance_accounts",
		"accountid",
		"accaid_seq",
		"CREATE TABLE gw_finance_accounts (\n"
		"	accountid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	accounttype	varchar(4)	DEFAULT '' NOT NULL,\n"
		"	accountname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (accountid)\n"
		")"
	},
	{
		"gw_finance_inventory",
		"inventoryid",
		"prodid_seq",
		"CREATE TABLE gw_finance_inventory (\n"
		"	inventoryid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	itemname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	category	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	discount	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	unitprice	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	internalcost	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	tax1		numeric(9,3)	DEFAULT '0.08' NOT NULL,\n"
		"	tax2		numeric(9,3)	DEFAULT '0.07' NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (inventoryid)\n"
		")"
	},
	{
		"gw_finance_invoices",
		"invoiceid",
		"ordeid_seq",
		"CREATE TABLE gw_finance_invoices (\n"
		"	invoiceid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	contactid	integer		DEFAULT 0 NOT NULL,\n"
		"	userid		integer		DEFAULT 0 NOT NULL,\n"
		"	invoicedate	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	invoicetype	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	paymentmethod	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	paymentdue	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	paymentreceived	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	status		integer		DEFAULT 0 NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (invoiceid)\n"
		")"
	},
	{
		"gw_finance_invoices_items",
		"invoiceitemid",
		"ordiid_seq",
		"CREATE TABLE gw_finance_invoices_items (\n"
		"	invoiceitemid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	invoiceid	integer		DEFAULT 0 NOT NULL,\n"
		"	inventoryid	integer		DEFAULT 0 NOT NULL,\n"
		"	quantity	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	discount	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	unitprice	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	internalcost	numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	tax1		numeric(9,3)	DEFAULT '0.08' NOT NULL,\n"
		"	tax2		numeric(9,3)	DEFAULT '0.07' NOT NULL,\n"
		"	PRIMARY KEY (invoiceitemid)\n"
		")"
	},
	{
		"gw_finance_journal",
		"entryid",
		"accjid_seq",
		"CREATE TABLE gw_finance_journal (\n"
		"	entryid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	entrydate	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	accountid	integer		DEFAULT 0 NOT NULL,\n"
		"	contactid	integer		DEFAULT 0 NOT NULL,\n"
		"	debit		numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	credit		numeric(9,2)	DEFAULT '0.00' NOT NULL,\n"
		"	details		varchar(250)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (entryid)\n"
		")"
	},
	{
		"gw_forums",
		"forumid",
		"foruid_seq",
		"CREATE TABLE gw_forums (\n"
		"	forumid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	forumgroupid	integer		DEFAULT 0 NOT NULL,\n"
		"	postername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	posttime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	subject		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	message		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (forumid)\n"
		")"
	},
	{
		"gw_forums_groups",
		"forumgroupid",
		"forgid_seq",
		"CREATE TABLE gw_forums_groups (\n"
		"	forumgroupid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	title		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	description	blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (forumgroupid)\n"
		")"
	},
	{
		"gw_forums_posts",
		"messageid",
		NULL,
		"CREATE TABLE gw_forums_posts (\n"
		"	messageid	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	forumid		integer		DEFAULT 0 NOT NULL,\n"
		"	referenceid	integer		DEFAULT 0 NOT NULL,\n"
		"	postername	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	posttime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	subject		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	message		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (messageid, forumid)\n"
		")"
	},
	{
		"gw_groups",
		"groupid",
		"grouid_seq",
		"CREATE TABLE gw_groups (\n"
		"	groupid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	groupname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	availability	varchar(170)	DEFAULT '' NOT NULL,\n"
		"	motd		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (groupid)\n"
		")"
	},
	{
		"gw_groups_members",
		"groupmemberid",
		"grpmid_seq",
		"CREATE TABLE gw_groups_members (\n"
		"	groupmemberid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	userid		integer		DEFAULT 0 NOT NULL,\n"
		"	groupid		integer		DEFAULT 0 NOT NULL,\n"
		"	PRIMARY KEY (groupmemberid)\n"
		")"
	},
	{
		"gw_messages",
		"messageid",
		"messid_seq",
		"CREATE TABLE gw_messages (\n"
		"	messageid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	sender		integer		DEFAULT 0 NOT NULL,\n"
		"	rcpt		integer		DEFAULT 0 NOT NULL,\n"
		"	status		integer		DEFAULT 2 NOT NULL,\n"
		"	message		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (messageid)\n"
		")"
	},
	{
		"gw_notes",
		"noteid",
		"noteid_seq",
		"CREATE TABLE gw_notes (\n"
		"	noteid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	tablename	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	tableindex	integer		DEFAULT 0 NOT NULL,\n"
		"	notetitle	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	notetext	blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (noteid)\n"
		")"
	},
	{
		"gw_projects",
		"projectid",
		"projid_seq",
		"CREATE TABLE gw_projects (\n"
		"	projectid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	projectname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	projectadmin	integer		DEFAULT 0 NOT NULL,\n"
		"	projectstart	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	projectfinish	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	status		integer		DEFAULT 0 NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (projectid)\n"
		")"
	},
	{
		"gw_queries",
		"queryid",
		"querid_seq",
		"CREATE TABLE gw_queries (\n"
		"	queryid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	queryname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	query		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (queryid)\n"
		")"
	},
	{
		"gw_smtp_relayrules",
		"relayruleid",
		"smtrid_seq",
		"CREATE TABLE gw_smtp_relayrules (\n"
		"	relayruleid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	persistence	varchar(8)	DEFAULT 'temp' NOT NULL,\n"
		"	ipaddress	varchar(16)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (relayruleid)\n"
		")"
	},
	{
		"gw_tasks",
		"taskid",
		"taskid_seq",
		"CREATE TABLE gw_tasks (\n"
		"	taskid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	assignedby	integer		DEFAULT 0 NOT NULL,\n"
		"	assignedto	integer		DEFAULT 0 NOT NULL,\n"
		"	taskname	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	contactid	integer		DEFAULT 0 NOT NULL,\n"
		"	projectid	integer		DEFAULT 0 NOT NULL,\n"
		"	duedate		timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	priority	integer		DEFAULT 0 NOT NULL,\n"
		"	reminder	integer		DEFAULT 0 NOT NULL,\n"
		"	status		integer		DEFAULT 0 NOT NULL,\n"
		"	details		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (taskid)\n"
		")"
	},
	{
		"gw_users",
		"userid",
		"userid_seq",
		"CREATE TABLE gw_users (\n"
		"	userid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	username	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	password	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	groupid		integer		DEFAULT 0 NOT NULL,\n"
		"	domainid	integer		DEFAULT 0 NOT NULL,\n"
		"	enabled		integer		DEFAULT 0 NOT NULL,\n"
		"	authdomainadmin	integer		DEFAULT 0 NOT NULL,\n"
		"	authadmin	integer		DEFAULT 0 NOT NULL,\n"
		"	authbookmarks	integer		DEFAULT 0 NOT NULL,\n"
		"	authcalendar	integer		DEFAULT 0 NOT NULL,\n"
		"	authcalls	integer		DEFAULT 0 NOT NULL,\n"
		"	authcontacts	integer		DEFAULT 0 NOT NULL,\n"
		"	authemail	integer		DEFAULT 0 NOT NULL,\n"
		"	authfiles	integer		DEFAULT 0 NOT NULL,\n"
		"	authfinance	integer		DEFAULT 0 NOT NULL,\n"
		"	authforums	integer		DEFAULT 0 NOT NULL,\n"
		"	authmessages	integer		DEFAULT 0 NOT NULL,\n"
		"	authprofile	integer		DEFAULT 0 NOT NULL,\n"
		"	authprojects	integer		DEFAULT 0 NOT NULL,\n"
		"	authquery	integer		DEFAULT 0 NOT NULL,\n"
		"	prefdaystart	integer		DEFAULT 0 NOT NULL,\n"
		"	prefdaylength	integer		DEFAULT 0 NOT NULL,\n"
		"	prefmailcurrent	integer		DEFAULT 0 NOT NULL,\n"
		"	prefmaildefault	integer		DEFAULT 0 NOT NULL,\n"
		"	prefmaxlist	integer		DEFAULT 0 NOT NULL,\n"
		"	prefmenustyle	integer		DEFAULT 1 NOT NULL,\n"
		"	preftimezone	integer		DEFAULT 0 NOT NULL,\n"
		"	prefgeozone	integer		DEFAULT 0 NOT NULL,\n"
		"	preflanguage	varchar(4)	DEFAULT 'en' NOT NULL,\n"
		"	preftheme	varchar(40)	DEFAULT 'default' NOT NULL,\n"
		"	availability	varchar(170)	DEFAULT '' NOT NULL,\n"
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
		"	birthdate	date		DEFAULT '1970-01-01' NOT NULL,\n"
		"	hiredate	date		DEFAULT '1970-01-01' NOT NULL,\n"
		"	sin		varchar(15),\n"
		"	isactive	varchar(10),\n"
		"	PRIMARY KEY (userid)\n"
		")"
	},
	{
		"gw_users_sessions",
		"sessionid",
		"usesid_seq",
		"CREATE TABLE gw_users_sessions (\n"
		"	sessionid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	remoteip	varchar(20)	DEFAULT '0.0.0.0' NOT NULL,\n"
		"	token		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	userid		integer		DEFAULT 0 NOT NULL,\n"
		"	domainid	integer		DEFAULT 0 NOT NULL,\n"
		"	PRIMARY KEY (sessionid)\n"
		")"
	},
	{
		"gw_weblog_comments",
		"commentid",
		"weblog_commentid_seq",
		"CREATE TABLE gw_weblog_comments (\n"
		"	commentid	integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	entryid		integer		DEFAULT 0 NOT NULL,\n"
		"	referenceid	integer		DEFAULT 0 NOT NULL,\n"
		"	subject		varchar(120)	DEFAULT '' NOT NULL,\n"
		"	message		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (commentid, entryid)\n"
		")"
	},
	{
		"gw_weblog_entries",
		"entryid",
		"weblog_entryid_seq",
		"CREATE TABLE gw_weblog_entries (\n"
		"	entryid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	entrygroupid	integer		DEFAULT 0 NOT NULL,\n"
		"	subject		varchar(120)	DEFAULT '' NOT NULL,\n"
		"	summary		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	message		blob sub_type 1	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (entryid)\n"
		")"
	},
	{
		"gw_weblog_groups",
		"groupid",
		"weblog_groupid_seq",
		"CREATE TABLE gw_weblog_groups (\n"
		"	groupid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	name		varchar(50)	DEFAULT '' NOT NULL,\n"
		"	description	varchar(240)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (groupid)\n"
		")"
	},
	{
		"gw_zones",
		"zoneid",
		"zoneid_seq",
		"CREATE TABLE gw_zones (\n"
		"	zoneid		integer		NOT NULL,\n"
		"	obj_ctime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_mtime	timestamp	DEFAULT '1970-01-01 00:00:00' NOT NULL,\n"
		"	obj_uid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gid		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_did		integer		DEFAULT 0 NOT NULL,\n"
		"	obj_gperm	integer		DEFAULT 0 NOT NULL,\n"
		"	obj_operm	integer		DEFAULT 0 NOT NULL,\n"
		"	zonename	varchar(50)	DEFAULT '' NOT NULL,\n"
		"	PRIMARY KEY (zoneid)\n"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};
