/*
    NullLogic GroupServer - Copyright (C) 2000-2007 Dan Cahill

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

typedef struct {
	char *name;
	char *index;
	char *seqname;
	char *schema;
} tabledef;

static const tabledef mysqldb_tables[] = {
	{
		"nullgs_entries",
		"id",
		NULL,
		"CREATE TABLE nullgs_entries ( "
			"id int4 NOT NULL auto_increment, "
			"pid int4 NOT NULL DEFAULT 0, "
			"did int4 NOT NULL DEFAULT 0, "
			"ctime datetime	NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"mtime datetime	NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"class varchar(64) NOT NULL, "
			"name varchar(255) NOT NULL, "
			"data text NOT NULL DEFAULT '', "
			"PRIMARY KEY (id), "
			"UNIQUE (did, pid, class, name)"
		")"
	},
	{
		"nullgs_sessions",
		"id",
		NULL,
		"CREATE TABLE nullgs_sessions ( "
			"id int4 NOT NULL auto_increment, "
			"uid int4 NOT NULL DEFAULT 0, "
			"did int4 NOT NULL DEFAULT 0, "
			"ctime datetime	NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"mtime datetime	NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"token varchar(50) NOT NULL DEFAULT '', "
			"remoteaddr varchar(20) NOT NULL DEFAULT '0.0.0.0', "
			"data text NOT NULL DEFAULT '', "
			"PRIMARY KEY (id)"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};

static const tabledef mdb_tables[] = {
	{
		"nullgs_entries",
		"id",
		NULL,
		"CREATE TABLE nullgs_entries ( "
			"id autoincrement, "
			"pid integer, "
			"did integer, "
			"ctime datetime NOT NULL, "
			"mtime datetime NOT NULL, "
			"class varchar(255) NOT NULL "
			"name varchar(255) NOT NULL, "
			"data memo, "
			"PRIMARY KEY (id)"
		")"
	},
	{
		"nullgs_sessions",
		"id",
		NULL,
		"CREATE TABLE nullgs_sessions ( "
			"id autoincrement, "
			"uid integer, "
			"did integer, "
			"ctime datetime NOT NULL, "
			"mtime datetime NOT NULL, "
			"token varchar(50) NOT NULL, "
			"remoteaddr varchar(20) NOT NULL, "
			"data memo, "
			"PRIMARY KEY (id)"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};

static const tabledef pgsqldb_tables[] = {
	{
		"nullgs_entries",
		"id",
		"nullgs_entries_seq",
		"CREATE TABLE nullgs_entries ( "
			"id int4 NOT NULL DEFAULT nextval('nullgs_entries_seq'::text), "
			"pid int4 NOT NULL, "
			"did int4 NOT NULL, "
			"ctime timestamp NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"mtime timestamp NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"class varchar(255) NOT NULL, "
			"name varchar(255) NOT NULL, "
			"data text NOT NULL DEFAULT '', "
			"PRIMARY KEY (id)"
		")"
	},
	{
		"nullgs_sessions",
		"id",
		"nullgs_sessions_seq",
		"CREATE TABLE nullgs_sessions ( "
			"id int4 NOT NULL DEFAULT nextval('nullgs_sessions_seq'::text), "
			"uid int4 NOT NULL DEFAULT 0, "
			"did int4 NOT NULL DEFAULT 0, "
			"ctime timestamp NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"mtime timestamp NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"token varchar(50) NOT NULL DEFAULT '', "
			"remoteaddr varchar(20) NOT NULL DEFAULT '0.0.0.0', "
			"data text NOT NULL DEFAULT '', "
			"PRIMARY KEY (id)"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};

static const tabledef sqlitedb_tables[] = {
	{
		"nullgs_entries",
		"id",
		NULL,
		"CREATE TABLE nullgs_entries ( "
			"id INTEGER, "
			"pid int4 NOT NULL, "
			"did int4 NOT NULL, "
			"ctime datetime NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"mtime datetime NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"class varchar(64) NOT NULL, "
			"name varchar(255) NOT NULL, "
			"data text DEFAULT '', "
			"PRIMARY KEY (id), "
			"UNIQUE (did, pid, class, name)"
		")"
	},
	{
		"nullgs_sessions",
		"id",
		NULL,
		"CREATE TABLE nullgs_sessions ( "
			"id INTEGER, "
			"uid int4 NOT NULL, "
			"did int4 NOT NULL, "
			"ctime datetime	NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"mtime datetime	NOT NULL DEFAULT '1970-01-01 00:00:00', "
			"token varchar(50) NOT NULL, "
			"remoteaddr varchar(20) NOT NULL, "
			"data text DEFAULT '', "
			"PRIMARY KEY (id)"
		")"
	},
	{ NULL, NULL, NULL, NULL }
};

char *sqldata_new[] = {
	/* default domain + attributes */
	"INSERT INTO nullgs_entries (id, pid, did, class, name, data) VALUES (1, 0, 1, \"organization\", \"default domain\", \"{}\");",
	"INSERT INTO nullgs_entries (id, pid, did, class, name, data) VALUES (2, 1, 1, \"associateddomain\", \"localhost\", \"{}\");",
	/* 'Administrators' group attributes */
	"INSERT INTO nullgs_entries (id, pid, did, name, class, data) VALUES (3, 1, 1, \"groupofnames\", \"Administrators\", \"{ " \
		"availability='000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', " \
		"members={}, " \
		"motd='<BR />\n<DIV CLASS=JUSTIFY>\nWelcome to NullLogic GroupServer.  If you are a new user, please take the time to read the online help.  It is also recommended that you edit your profile and configure your e-mail account information if you have not already done so.\n<BR /><BR />\nADMINISTRATORS: This message can, and should be customized to meet the needs of your users.\n</DIV>' " \
	"}\" );",
	/* 'Users' group attributes */
	"INSERT INTO nullgs_entries (id, pid, did, name, class, data) VALUES (4, 1, 1, \"groupofnames\", \"Users\", \"{ " \
		"availability='000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', " \
		"members={}, " \
		"motd='<BR />\n<DIV CLASS=JUSTIFY>\nWelcome to NullLogic GroupServer.  If you are a new user, please take the time to read the online help.  It is also recommended that you edit your profile and configure your e-mail account information if you have not already done so.\n</DIV>' " \
	"}\" );",
	/* 'administrator' user attributes */
	"INSERT INTO nullgs_entries (id, pid, did, name, class, data) VALUES (100, 1, 1, \"person\", \"administrator\", \"{ " \
		"uid='administrator', " \
		"uidnumber=100, " \
		"userpassword='$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/', " \
		"domainid=1, " \
		"groupid=1, " \
		"availability='000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', " \
		"auth={ domainadmin=31, admin=31, bookmarks=31, calendar=31, calls=31, contacts=31, email=31, files=31, finance=31, forums=31, messages=31, profile=31, projects=31, query=17 }, " \
		"pref={ daystart=0, daylength=24, geozone=1, language='en', mailcurrent=1, maildefault=0, maxlist=25, menustyle=0, timezone=10, theme='default' } " \
	" }\" );",
/*	"INSERT INTO nullgs_entries (id, pid, did, name, class) VALUES (4, 1, 1, 'administrator', 'user');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'surname', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'givenname', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'jobtitle', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'division', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'supervisor', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'address', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'locality', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'region', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'country', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'postalcode', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'homenumber', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'worknumber', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'faxnumber', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'cellnumber', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'pagernumber', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'email', 'administrator@localhost');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'birthdate', '1900-01-01');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'hiredate', '1900-01-01');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'sin', '');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (4, 1, 'isactive', '');",
*/
	/* 'anonymous' guest user attributes */
/*	"INSERT INTO nullgs_entries (id, pid, did, name, class) VALUES (5, 1, 1, 'anonymous', 'user');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (5, 1, 'userid', '2');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (5, 1, 'username', 'anonymous');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (5, 1, 'password', '$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (5, 1, 'enabled', 'false');",
*/
/*	"INSERT INTO nullgs_entries (id, pid, did, name, class) VALUES (2, 1, 1, 'default zone', 'zone');",*/
	/* smtp rule for localhost attributes */
/*	"INSERT INTO nullgs_entries (id, pid, did, name, class) VALUES (5, 1, 1, '127.0.0.1', 'smtp_rule');",*/
/*	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (5, 1, 'persistence', 'permanent');",*/

	/* JUNK DATA FOR TESTING */
/*
	"INSERT INTO nullgs_entries (id, pid, did, name, class) VALUES (7, 1, 1, 'nulllogic', 'user');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (7, 1, 'userid', '7');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (7, 1, 'username', 'nulllogic');",
	"INSERT INTO nullgs_attributes (pid, did, name, value) VALUES (7, 1, 'password', '$1$xxxxxxxx$yyyyyyyyyyyyyyyyyyyyyy');",
*/
	NULL
};
/*
char *sqldata_callactions[7] = {
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('1', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Phoned');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('2', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Returned your call');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('3', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Please call');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('4', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Came to see you');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('5', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Wants to see you');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('6', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Will call again');",
	NULL
};

char *sqldata_domains[2] = {
	"INSERT INTO gw_domains (domainid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, domainname) VALUES ('1', '2007-01-01', '2007-01-01', '0', '0', '1', '0', '0', 'localhost');",
	NULL
};

char *sqldata_eventclosings[4] = {
	"INSERT INTO gw_events_closings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('1', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Completed');",
	"INSERT INTO gw_events_closings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('2', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Cancelled');",
	"INSERT INTO gw_events_closings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('3', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Rescheduled');",
	NULL
};

char *sqldata_eventtypes[7] = {
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('1', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'All Day Event');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('2', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Anniversary');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('3', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Appointment');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('4', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Meeting');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('5', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Reminder');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('6', '2007-01-01', '2007-01-01', '0', '0', '0', '0', '0', 'Service Call');",
	NULL
};
*/
