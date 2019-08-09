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

typedef struct {
	char *name;
	char *index;
	char *seqname;
	char *schema;
} tabledef;

char *sqldata_new[21] = {
	"INSERT INTO gw_dbinfo (dbversion, tax1name, tax2name, tax1percent, tax2percent) VALUES ('" PACKAGE_VERSION "', 'PST', 'GST', '0.080', '0.070');",
	"INSERT INTO gw_bookmarks (bookmarkid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, folderid, bookmarkname, bookmarkurl) VALUES ('1', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '0', 'NullLogic', 'http://nullgroupware.sourceforge.net/groupware/');",
	"INSERT INTO gw_bookmarks_folders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('1', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '0', 'users');",
	"INSERT INTO gw_bookmarks_folders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('2', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '0', 'groups');",
	"INSERT INTO gw_bookmarks_folders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('3', '2008-01-01', '2008-01-01', '0', '1', '1', '2', '1', '2', 'Administrators');",
	"INSERT INTO gw_bookmarks_folders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('4', '2008-01-01', '2008-01-01', '0', '2', '1', '2', '1', '2', 'Users');",
	"INSERT INTO gw_bookmarks_folders (folderid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, parentid, foldername) VALUES ('5', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '1', 'administrator');",
	"INSERT INTO gw_email_accounts (mailaccountid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, accountname, realname, organization, address, replyto, hosttype, pophost, popport, popssl, smtphost, smtpport, smtpssl, popusername, poppassword, smtpauth, lastcount, notify, remove, lastcheck, showdebug, signature) VALUES ('1', '2008-01-01', '2008-01-01', '1', '0', '1', '0', '0', 'Local Mail', 'Groupware Administrator', '', 'administrator@localhost', '', 'POP3', 'localhost', '110', '0', 'localhost', '25', '0', 'administrator@localhost', 'dmlzdWFs', 'n', '0', '60', '2', '2008-01-01', 'n', '');",
	"INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('1', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '/files/', 'users', 'dir', '2008-01-01', '2008-01-01', '0', '');",
	"INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('2', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '/files/', 'groups', 'dir', '2008-01-01', '2008-01-01', '0', '');",
	"INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('3', '2008-01-01', '2008-01-01', '0', '1', '1', '2', '1', '/files/groups/', 'Administrators', 'dir', '2008-01-01', '2008-01-01', '0', '');",
	"INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('4', '2008-01-01', '2008-01-01', '0', '2', '1', '2', '1', '/files/groups/', 'Users', 'dir', '2008-01-01', '2008-01-01', '0', '');",
	"INSERT INTO gw_files (fileid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, filepath, filename, filetype, uldate, lastdldate, numdownloads, description) VALUES ('5', '2008-01-01', '2008-01-01', '1', '1', '1', '1', '1', '/files/users/', 'administrator', 'dir', '2008-01-01', '2008-01-01', '0', '');",
	"INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', 'Administrators', '000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', '<BR>\r\n<DIV CLASS=JUSTIFY>\r\nWelcome to NullLogic Groupware.  If you are a new user, please take the time to read the online help.  It is also recommended that you edit your profile and configure your e-mail account information if you haven''t already done so.\n<BR><BR>\r\nADMINISTRATORS: This message can, and should be customized to meet the needs of your users.\n</DIV>');",
	"INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd) VALUES ('2', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', 'Users', '000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', '<BR>\r\n<DIV CLASS=JUSTIFY>\r\nWelcome to NullLogic Groupware.  If you are a new user, please take the time to read the online help.  It is also recommended that you edit your profile and configure your e-mail account information if you haven''t already done so.\n</DIV>');",
	"INSERT INTO gw_groups_members (groupmemberid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, userid, groupid) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', '1', '1');",
	"INSERT INTO gw_groups_members (groupmemberid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, userid, groupid) VALUES ('2', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', '1', '2');",
	"INSERT INTO gw_smtp_relayrules (relayruleid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, persistence, ipaddress) VALUES ('1', '2008-01-01', '2008-01-01', '1', '0', '1', '0', '0', 'perm', '127.0.0.1');",
	"INSERT INTO gw_users (userid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, username, password, groupid, domainid, enabled, authdomainadmin, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authemail, authfiles, authfinance, authforums, authmessages, authprofile, authprojects, authquery, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone, prefgeozone, preflanguage, preftheme, availability, surname, givenname, jobtitle, division, supervisor, address, locality, region, country, postalcode, homenumber, worknumber, faxnumber, cellnumber, pagernumber, email, birthdate, hiredate, sin, isactive) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', 'administrator', '$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/', '1', '1', '1', '31', '31', '31', '31', '31', '31', '15', '31', '31', '21', '13', '3', '31', '17', '0', '24', '1', '0', '25', '1', '10', '1', 'en', 'default', '000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', 'administrator@localhost', '1900-01-01', '1900-01-01', '', '');",
	"INSERT INTO gw_zones (zoneid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, zonename) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', 'default');",
	NULL
};

char *sqldata_callactions[7] = {
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Phoned');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('2', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Returned your call');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('3', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Please call');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('4', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Came to see you');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('5', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Wants to see you');",
	"INSERT INTO gw_calls_actions (actionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('6', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Will call again');",
	NULL
};

char *sqldata_domains[2] = {
	"INSERT INTO gw_domains (domainid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, domainname) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '1', '0', '0', 'localhost');",
	NULL
};

char *sqldata_eventclosings[4] = {
	"INSERT INTO gw_events_closings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Completed');",
	"INSERT INTO gw_events_closings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('2', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Cancelled');",
	"INSERT INTO gw_events_closings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('3', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Rescheduled');",
	NULL
};

char *sqldata_eventtypes[7] = {
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('1', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'All Day Event');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('2', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Anniversary');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('3', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Appointment');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('4', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Meeting');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('5', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Reminder');",
	"INSERT INTO gw_events_types (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('6', '2008-01-01', '2008-01-01', '0', '0', '0', '0', '0', 'Service Call');",
	NULL
};
