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

typedef struct {
	char *name;
	char *index;
	char *seqname;
	char *schema;
} tabledef;

char *sqldata_new[7] = {
	"INSERT INTO gw_dbinfo (dbversion, tax1name, tax2name, tax1percent, tax2percent) VALUES ('" PACKAGE_VERSION "', 'PST', 'GST', '0.080', '0.070');",
	"INSERT INTO gw_bookmarks (bookmarkid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, folderid, bookmarkname, bookmarkurl) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '1', '1', '1', '1', '1', '0', 'NullLogic', 'http://nullgroupware.sourceforge.net/groupware/');",
	"INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd, members) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '1', '0', '0', 'admin', '000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', '<BR>\n<DIV CLASS=JUSTIFY>\nWelcome to NullLogic Groupware.  If you are a new user, please take the time to read the online help.  It is also recommended that you edit your profile and configure your e-mail account information if you haven''t already done so.\n<BR><BR>\nADMINISTRATORS: This message can, and should be customized to meet the needs of your users.\n</DIV>', '');",
	"INSERT INTO gw_groups (groupid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, groupname, availability, motd, members) VALUES ('2', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '1', '0', '0', 'users', '000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', '<BR>\n<DIV CLASS=JUSTIFY>\nWelcome to NullLogic Groupware.  If you are a new user, please take the time to read the online help.  It is also recommended that you edit your profile and configure your e-mail account information if you haven''t already done so.\n</DIV>', '');",
	"INSERT INTO gw_users (userid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, username, password, groupid, domainid, enabled, authdomainadmin, authadmin, authbookmarks, authcalendar, authcalls, authcontacts, authfiles, authforums, authmessages, authorders, authprofile, authquery, authwebmail, prefdaystart, prefdaylength, prefmailcurrent, prefmaildefault, prefmaxlist, prefmenustyle, preftimezone, prefgeozone, availability, surname, givenname, jobtitle, division, supervisor, address, locality, region, country, postalcode, homenumber, worknumber, faxnumber, cellnumber, pagernumber, email, birthdate, hiredate, sin, isactive) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '1', '0', '0', 'administrator', '$1$0hs2u/6B$KxBgAAw59fBu/DyHA/Vor/', '1', '1', '1', '31', '31', '31', '31', '31', '31', '31', '31', '31', '31', '31', '31', '31', '0', '24', '0', '0', '25', '1', '10', '1', '000000000000000000000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000111111110000000000000000000000000000000', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '1900-01-01', '1900-01-01', '', '');",
	"INSERT INTO gw_zones (zoneid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, zonename) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '1', '0', '0', 'default');",
	NULL
};

char *sqldata_callactions[7] = {
	"INSERT INTO gw_callactions (callactionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Phoned');",
	"INSERT INTO gw_callactions (callactionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('2', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Returned your call');",
	"INSERT INTO gw_callactions (callactionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('3', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Please call');",
	"INSERT INTO gw_callactions (callactionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('4', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Came to see you');",
	"INSERT INTO gw_callactions (callactionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('5', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Wants to see you');",
	"INSERT INTO gw_callactions (callactionid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, actionname) VALUES ('6', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Will call again');",
	NULL
};

char *sqldata_domains[2] = {
	"INSERT INTO gw_domains (domainid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, domainname) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'default');",
	NULL
};

char *sqldata_eventclosings[4] = {
	"INSERT INTO gw_eventclosings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Completed');",
	"INSERT INTO gw_eventclosings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('2', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Cancelled');",
	"INSERT INTO gw_eventclosings (eventclosingid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, closingname) VALUES ('3', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Rescheduled');",
	NULL
};

char *sqldata_eventtypes[7] = {
	"INSERT INTO gw_eventtypes (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('1', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'All Day Event');",
	"INSERT INTO gw_eventtypes (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('2', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Anniversary');",
	"INSERT INTO gw_eventtypes (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('3', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Appointment');",
	"INSERT INTO gw_eventtypes (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('4', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Meeting');",
	"INSERT INTO gw_eventtypes (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('5', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Reminder');",
	"INSERT INTO gw_eventtypes (eventtypeid, obj_ctime, obj_mtime, obj_uid, obj_gid, obj_did, obj_gperm, obj_operm, eventtypename) VALUES ('6', '2004-01-01 00:00:00', '2004-01-01 00:00:00', '0', '0', '0', '0', '0', 'Service Call');",
	NULL
};
