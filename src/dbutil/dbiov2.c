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
#include "main.h"

static int dump_write(FILE *fp, const char *str)
{
	int len;

	for (len=0;str[len];len++) {
		switch (str[len]) {
		case '\\': fprintf(fp, "\\\\"); break;
		case '\"': fprintf(fp, "\\\""); break;
		case '\'': fprintf(fp, "\\''"); break;
		case '\r': break;
		case '\n': fprintf(fp, "\\n"); break;
		default: fprintf(fp, "%c", str[len]); break;
		}
	}
	return len;
}

static int dump_sub(nsp_state *N, FILE *fp, obj_t *tobj, char *pclass)
{
	int ent=0;

	for (tobj=tobj->val->d.table.f;tobj;tobj=tobj->next) {
		if (strcmp(tobj->name, "obj_uid")==0) continue;
		if (strcmp(pclass, "bookmark")==0) {
			if (strcmp(tobj->name, "folderid")==0) continue;
		} else if (strcmp(pclass, "bookmarkfolder")==0) {
			if (strcmp(tobj->name, "folderid")==0) continue;
			if (strcmp(tobj->name, "parentid")==0) continue;
		} else if (strcmp(pclass, "emailaccount")==0) {
			if (strcmp(tobj->name, "mailaccountid")==0) continue;
		} else if (strcmp(pclass, "emailheader")==0) {
			if (strcmp(tobj->name, "msg_text")==0) continue;
		} else if (strcmp(pclass, "webloggroup")==0) {
			if (strcmp(tobj->name, "groupid")==0) continue;
		} else if (strcmp(pclass, "weblogentry")==0) {
			if (strcmp(tobj->name, "entrygroupid")==0) continue;
			if (strcmp(tobj->name, "entryid")==0) continue;
		} else if (strcmp(pclass, "person")==0) {
			if (strcmp(tobj->name, "userid")==0) continue;
		}
		if (ent) fprintf(fp, ", ");
		if (tobj->val->type==NT_TABLE) {
			if (isdigit(tobj->name[0])) {
				fprintf(fp, "[%s]={ ", tobj->name);
			} else {
				fprintf(fp, "%s={ ", tobj->name);
			}
			dump_sub(N, fp, tobj, pclass);
			fprintf(fp, " }");
			ent++;
		} else {
			if (isdigit(tobj->name[0])) {
				fprintf(fp, "[%s]=''", tobj->name);
			} else {
				fprintf(fp, "%s=''", tobj->name);
			}
			dump_write(fp, nsp_tostr(N, tobj));
			fprintf(fp, "''");
			ent++;
		}
	}
	return 0;
}

static int dump_rec(nsp_state *N, FILE *fp, obj_t *tobj)
{
	int id=(int)nsp_getnum(N, tobj, "id");
	int pid=(int)nsp_getnum(N, tobj, "pid");
	int did=(int)nsp_getnum(N, tobj, "did");
	char *pclass=nsp_getstr(N, tobj, "class");
	char *pname=nsp_getstr(N, tobj, "name");

	if (strcmp(pclass, "emailfolder")==0) return 0;
	fprintf(fp, "INSERT INTO nullgs_entries (id, pid, did, ctime, mtime, class, name, data) VALUES (");
	fprintf(fp, "%4d,%4d,%4d, '%s', '%s', '%s', '", id, pid, did, nsp_getstr(N, tobj, "ctime"), nsp_getstr(N, tobj, "mtime"), pclass);
	sqlfprintf(fp, "%s", pname);
	fprintf(fp, "', '{ ");
	tobj=nsp_getobj(N, tobj, "_data");
	dump_sub(N, fp, tobj, pclass);
	fprintf(fp, " }');\n");
	return 0;
}

static int add_ou(nsp_state *N, obj_t *robj, char *name, char *name2, int id, int pid, int did)
{
	obj_t *sobj, *tobj, *uobj;
	char tmp[16];

	sobj=nsp_settable(N, robj, "_rows");
	snprintf(tmp, sizeof(tmp)-1, "%d", id);
	tobj=nsp_settable(N, sobj, tmp);
	nsp_setnum(N, tobj, "id", id);
	nsp_setnum(N, tobj, "pid", pid);
	if (did) nsp_setnum(N, tobj, "did", did);
	nsp_setstr(N, tobj, "ctime", "2001-01-01 00:00:00", 19);
	nsp_setstr(N, tobj, "mtime", "2001-01-01 00:00:00", 19);
	nsp_setstr(N, tobj, "class", "organizationalunit", strlen("organizationalunit"));
	nsp_setstr(N, tobj, "name", name, strlen(name));
	uobj=nsp_settable(N, tobj, "_data");
	nsp_setstr(N, uobj, "ou", name, strlen(name));
	nsp_setnum(N, robj, name2, id);
	return id+1;
}

static int add_table(nsp_state *N, obj_t *robj, char *table, char *index, int did)
{
	obj_t *sobj, *tobj, *uobj, *vobj;
	char query[100];
	char name[100];
	char tmp[16];
	SQLRES sqr;
	int n=(int)nsp_getnum(N, robj, "_tuples");
	int ou_people  = (int)nsp_getnum(N, robj, "_ou_people");
	int ou_users   = (int)nsp_getnum(N, robj, "_ou_users");
	int ou_contacts= (int)nsp_getnum(N, robj, "_ou_contacts");
	int ou_other   = (int)nsp_getnum(N, robj, "_ou_other");
	int i, j;
	char *p;
	char *pname;
	char *pclass;
	int pid=0;

	if (n<1) n=1;
	if ((strcmp(table, "gw_users")==0)&&(n<100)) n=100;
	snprintf(query, sizeof(query)-1, "SELECT * FROM %s WHERE obj_did = %d ORDER BY %s ASC", table, did, index);
	if (sql_query(&sqr, query)<0) {
		printf("\r\nError dumping %s\r\n", table);
		return -1;
	}
	sobj=nsp_settable(N, robj, "_rows");
	for (i=0;i<sql_numtuples(&sqr);i++) {
		if (!ou_people) ou_people=did;
		if (!ou_users) ou_users=did;
		if (!ou_contacts) ou_contacts=did;
		if (!ou_other) ou_other=did;
		pid=ou_other;
		if (strcmp(table, "gw_bookmarks")==0) {
			pclass="bookmark";
//			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
			snprintf(name, sizeof(name)-1, "%s (%d)", sql_getvaluebyname(&sqr, i, "bookmarkname"), n);
		} else if (strcmp(table, "gw_bookmarks_folders")==0) {
			pclass="bookmarkfolder";
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "foldername"));
		} else if (strcmp(table, "gw_calls")==0) {
			pclass="organizationalRole";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_contacts")==0) {
			pid=ou_contacts;
			pclass="person";
			snprintf(name, sizeof(name)-1, "%s, %s", sql_getvaluebyname(&sqr, i, "surname"), sql_getvaluebyname(&sqr, i, "givenname"));
		} else if (strcmp(table, "gw_domains")==0) {
			pclass="organization";
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "domainname"));
		} else if (strcmp(table, "gw_domains_aliases")==0) {
			pclass="associateddomain";
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "domainname"));
		} else if (strcmp(table, "gw_email_accounts")==0) {
			pclass="emailaccount";
			snprintf(name, sizeof(name)-1, "%s (%d)", sql_getvaluebyname(&sqr, i, "accountname"), n);
		} else if (strcmp(table, "gw_email_folders")==0) {
			pclass="emailfolder";
			snprintf(name, sizeof(name)-1, "%s (%d)", sql_getvaluebyname(&sqr, i, "foldername"), n);
		} else if (strcmp(table, "gw_email_headers")==0) {
			pclass="emailheader";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_events")==0) {
			pclass="event";
			snprintf(name, sizeof(name)-1, "%s (%d)", sql_getvaluebyname(&sqr, i, "eventname"), n);
		} else if (strcmp(table, "gw_files")==0) {
			pclass="file";
			snprintf(name, sizeof(name)-1, "%s (%d)", sql_getvaluebyname(&sqr, i, "filename"), n);
		} else if (strcmp(table, "gw_finance_accounts")==0) {
			pclass="financeaccount";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_finance_inventory")==0) {
			pclass="financeitem";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_finance_journal")==0) {
			pclass="financejournal";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_forums")==0) {
			pclass="forum";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_forums_groups")==0) {
			pclass="forumgroup";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_forums_posts")==0) {
			pclass="forumpost";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_groups")==0) {
			pid=ou_users;
			pclass="groupofnames";
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "groupname"));
		} else if (strcmp(table, "gw_notes")==0) {
			pclass="note";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_projects")==0) {
			pclass="project";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_queries")==0) {
			pclass="dbquery";
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "queryname"));
		} else if (strcmp(table, "gw_smtp_relayrules")==0) {
			pclass="smtprule";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_tasks")==0) {
			pclass="task";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_users")==0) {
			pid=ou_users;
			pclass="person";
//			snprintf(name, sizeof(name)-1, "%s %s", sql_getvaluebyname(&sqr, i, "givenname"), sql_getvaluebyname(&sqr, i, "surname"));
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "username"));
		} else if (strcmp(table, "gw_weblog_groups")==0) {
			pclass="webloggroup";
			snprintf(name, sizeof(name)-1, "%s (%d)", sql_getvaluebyname(&sqr, i, "name"), n);
		} else if (strcmp(table, "gw_weblog_entries")==0) {
			pclass="weblogentry";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		} else if (strcmp(table, "gw_zones")==0) {
			pclass="locality";
			snprintf(name, sizeof(name)-1, "%s", sql_getvaluebyname(&sqr, i, "zonename"));
		} else {
			pclass="organizationalRole";
			snprintf(name, sizeof(name)-1, "%s.%d", table, n);
		}

		snprintf(tmp, sizeof(tmp)-1, "%d", n);
		tobj=nsp_settable(N, sobj, tmp);
		nsp_setnum(N, tobj, "id", n);
		nsp_setnum(N, tobj, "pid", pid);
		nsp_setnum(N, tobj, "did", did);
		p=sql_getvaluebyname(&sqr, i, "obj_ctime");
		nsp_setstr(N, tobj, "ctime", p, strlen(p));
		p=sql_getvaluebyname(&sqr, i, "obj_mtime");
		nsp_setstr(N, tobj, "mtime", p, strlen(p));
		nsp_setstr(N, tobj, "class", pclass, strlen(pclass));
		nsp_setstr(N, tobj, "name", name, strlen(name));
		uobj=nsp_settable(N, tobj, "_data");

		for (j=0;j<sql_numfields(&sqr);j++) {
			pname=sql_getname(&sqr, j);
//			if (strcmp(pname, "obj_uid")==0) continue;
			if (strcmp(pname, "obj_gid")==0) continue;
			if (strcmp(pname, "obj_did")==0) continue;
			if (strcmp(pname, "obj_gperm")==0) continue;
			if (strcmp(pname, "obj_operm")==0) continue;
			if (strcmp(pname, "obj_ctime")==0) continue;
			if (strcmp(pname, "obj_mtime")==0) continue;
			if (strcmp(table, "gw_bookmarks")==0) {
				if (strcmp(pname, "bookmarkid")==0) continue;
			} else if (strcmp(table, "gw_contacts")==0) {
				if (strcmp(pname, "folderid")==0) continue;
			} else if (strcmp(table, "gw_domains")==0) {
				if (strcmp(pname, "domainid")==0) continue;
			} else if (strcmp(table, "gw_domains_aliases")==0) {
				if (strcmp(pname, "domainaliasid")==0) continue;
				if (strcmp(pname, "domainid")==0) continue;
			} else if (strcmp(table, "gw_groups")==0) {
				if (strcmp(pname, "groupname")!=0) continue;
			} else if (strcmp(table, "gw_queries")==0) {
				if (strcmp(pname, "queryid")==0) continue;
			} else if (strcmp(table, "gw_users")==0) {
				if (strncmp(pname, "auth", 4)==0) continue;
				if (strncmp(pname, "pref", 4)==0) continue;
			}
			if (strcmp(table, "gw_bookmarks")==0) {
				if (strcmp(pname, "bookmarkname")==0)      pname="cn";
				else if (strcmp(pname, "bookmarkurl")==0)  pname="labeleduri";
			} else if (strcmp(table, "gw_bookmarks_folders")==0) {
				if (strcmp(pname, "foldername")==0)        pname="cn";
			} else if (strcmp(table, "gw_contacts")==0) {
				if (strcmp(pname, "contactid")==0) { nsp_setnum(N, uobj, "uidnumber", n); continue; }
				if (strcmp(pname, "surname")==0)           pname="sn";
				else if (strcmp(pname, "givenname")==0)    pname="gn";
				else if (strcmp(pname, "username")==0)     pname="uid";
				else if (strcmp(pname, "password")==0)     pname="userpassword";
				else if (strcmp(pname, "salutation")==0)   pname="personaltitle";
				else if (strcmp(pname, "homenumber")==0)   pname="hometelephonenumber";
				else if (strcmp(pname, "faxnumber")==0)    pname="facsimiletelephonenumber";
				else if (strcmp(pname, "cellnumber")==0)   pname="mobiletelephonenumber";
				else if (strcmp(pname, "mobilenumber")==0) pname="mobiletelephonenumber";
				else if (strcmp(pname, "pagernumber")==0)  pname="pagertelephonenumber";
				else if (strcmp(pname, "email")==0)        pname="mail";
				else if (strcmp(pname, "website")==0)      pname="labeleduri";
				else if (strcmp(pname, "organization")==0) pname="organization";
				else if (strcmp(pname, "jobtitle")==0)     pname="title";
				else if (strcmp(pname, "address")==0)      pname="streetaddress";
				else if (strcmp(pname, "locality")==0)     pname="localityname";
				else if (strcmp(pname, "region")==0)       pname="stateorprovincename";
				else if (strcmp(pname, "country")==0)      pname="friendlycountryname";
				else if (strcmp(pname, "postalcode")==0)   pname="postalcode";
				else if (strcmp(pname, "worknumber")==0)   pname="worktelephonenumber";
			} else if (strcmp(table, "gw_domains")==0) {
				if (strcmp(pname, "domainname")==0)        pname="o";
			} else if (strcmp(table, "gw_domains_aliases")==0) {
				if (strcmp(pname, "domainname")==0)        pname="associateddomain";
			} else if (strcmp(table, "gw_email_accounts")==0) {
				if (strcmp(pname, "accountname")==0)       pname="cn";
			} else if (strcmp(table, "gw_email_folders")==0) {
				if (strcmp(pname, "foldername")==0)        pname="cn";
				if (strcmp(pname, "mailfolderid")==0)      pname="folderid";
			} else if (strcmp(table, "gw_email_headers")==0) {
				if (strcmp(pname, "folder")==0)            pname="folderid";
			} else if (strcmp(table, "gw_groups")==0) {
				if (strcmp(pname, "groupname")==0)         pname="cn";
			} else if (strcmp(table, "gw_queries")==0) {
				if (strcmp(pname, "queryname")==0)         pname="cn";
			} else if (strcmp(table, "gw_weblog_groups")==0) {
				if (strcmp(pname, "name")==0)              pname="cn";
			} else if (strcmp(table, "gw_users")==0) {
//				if (strcmp(pname, "userid")==0) { nsp_setnum(N, uobj, "uidnumber", n); continue; }
				if (strcmp(pname, "userid")==0) { nsp_setnum(N, uobj, "uidnumber", n); }
				if (strcmp(pname, "surname")==0)           pname="sn";
				else if (strcmp(pname, "givenname")==0)    pname="gn";
				else if (strcmp(pname, "username")==0)     pname="uid";
				else if (strcmp(pname, "jobtitle")==0)     pname="title";
				else if (strcmp(pname, "password")==0)     pname="userpassword";
				else if (strcmp(pname, "preflanguage")==0) pname="preferredlanguage";
				else if (strcmp(pname, "homenumber")==0)   pname="hometelephonenumber";
				else if (strcmp(pname, "faxnumber")==0)    pname="facsimiletelephonenumber";
				else if (strcmp(pname, "cellnumber")==0)   pname="mobiletelephonenumber";
				else if (strcmp(pname, "pagernumber")==0)  pname="pagertelephonenumber";
				else if (strcmp(pname, "email")==0)        pname="mail";
				else if (strcmp(pname, "address")==0)      pname="streetaddress";
				else if (strcmp(pname, "locality")==0)     pname="localityname";
				else if (strcmp(pname, "region")==0)       pname="stateorprovincename";
				else if (strcmp(pname, "country")==0)      pname="friendlycountryname";
				else if (strcmp(pname, "postalcode")==0)   pname="postalcode";
			} else if (strcmp(table, "gw_zones")==0) {
				if (strcmp(pname, "zonename")==0)          pname="cn";
			}
			p=sql_getvalue(&sqr, i, j);
			nsp_setstr(N, uobj, pname, p, strlen(p));
		}
		if (strcmp(table, "gw_users")==0) {
			vobj=nsp_settable(N, uobj, "auth");
			for (j=0;j<sql_numfields(&sqr);j++) {
				pname=sql_getname(&sqr, j);
				if (strncmp(pname, "auth", 4)!=0) continue;
				pname+=4;
				p=sql_getvalue(&sqr, i, j);
				nsp_setstr(N, vobj, pname, p, strlen(p));
			}
			vobj=nsp_settable(N, uobj, "pref");
			for (j=0;j<sql_numfields(&sqr);j++) {
				pname=sql_getname(&sqr, j);
				if (strncmp(pname, "pref", 4)!=0) continue;
				pname+=4;
				p=sql_getvalue(&sqr, i, j);
				nsp_setstr(N, vobj, pname, p, strlen(p));
			}
		}
		n++;
		if (strcmp(table, "gw_domains")==0) {
			snprintf(tmp, sizeof(tmp)-1, "%d", n);
			tobj=nsp_settable(N, sobj, tmp);
			nsp_setnum(N, tobj, "id", n);
			nsp_setnum(N, tobj, "pid", did);
			nsp_setnum(N, tobj, "did", did);
			p=sql_getvaluebyname(&sqr, i, "obj_ctime");
			nsp_setstr(N, tobj, "ctime", p, strlen(p));
			p=sql_getvaluebyname(&sqr, i, "obj_mtime");
			nsp_setstr(N, tobj, "mtime", p, strlen(p));
			nsp_setstr(N, tobj, "class", "associateddomain", strlen("associateddomain"));
			nsp_setstr(N, tobj, "name", name, strlen(name));
			uobj=nsp_settable(N, tobj, "_data");
			p=sql_getvaluebyname(&sqr, i, "domainname");
			nsp_setstr(N, uobj, "associateddomain", p, strlen(p));
			n++;
		}
	}
	if (strcmp(table, "gw_domains_aliases")==0) {
		n=add_ou(N, robj, "People",    "_ou_people",   n, did, did);
		n=add_ou(N, robj, "Users",     "_ou_users",    n, n-1, did);
		n=add_ou(N, robj, "Contacts",  "_ou_contacts", n, n-2, did);
		n=add_ou(N, robj, "OtherData", "_ou_other",    n, did, did);
	}
	sql_freeresult(&sqr);
	nsp_setnum(N, robj, "_tuples", n);
	return 0;
}

static int fix_refs(nsp_state *N, obj_t *robj)
{
	obj_t *sobj1, *tobj1;
	obj_t *sobj2, *tobj2;
	obj_t *tobj3, *tobj4;
	int n1, n2;
	int pid;

	sobj1=nsp_getobj(N, robj, "_rows");
	for (sobj1=sobj1->val->d.table.f;sobj1;sobj1=sobj1->next) {
		if (sobj1->val->type!=NT_TABLE) continue;
		pid=(int)nsp_getnum(N, sobj1, "id");
		if (strcmp(nsp_getstr(N, sobj1, "class"), "bookmarkfolder")==0) {
			tobj1=nsp_getobj(N, sobj1, "_data");
			n1=(int)nsp_getnum(N, tobj1, "folderid");
			if (n1<1) continue;
			/* fix bookmarkfolder parentid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "bookmarkfolder")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "parentid");
				if (n1==n2) nsp_setnum(N, sobj2, "pid", pid);
			}
			/* fix bookmark folderid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "bookmark")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "folderid");
				if (n1==n2) nsp_setnum(N, sobj2, "pid", pid);
			}
		} else if (strcmp(nsp_getstr(N, sobj1, "class"), "emailaccount")==0) {
			tobj1=nsp_getobj(N, sobj1, "_data");
			n1=(int)nsp_getnum(N, tobj1, "mailaccountid");
			if (n1<1) continue;
			/* fix emailfolder accountid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "emailfolder")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				tobj3=nsp_settable(N, tobj1, "folders");
				n2=(int)nsp_getnum(N, tobj2, "accountid");
				if (n1==n2) {
					nsp_setnum(N, sobj2, "pid", pid);

					tobj4=nsp_settable(N, tobj3, nsp_getstr(N, tobj2, "folderid"));
					nsp_setstr(N, tobj4, "cn", nsp_getstr(N, tobj2, "cn"), -1);
					nsp_setnum(N, tobj4, "folderid", nsp_getnum(N, tobj2, "folderid"));
					nsp_setnum(N, tobj4, "parentid", nsp_getnum(N, tobj2, "parentfolderid"));

//					nsp_setstr(N, sobj2, "class", "emailfolder-", -1);
//					nsp_freetable(N, sobj2);

				}
			}
			/* fix emailaccount pid */
			n1=(int)nsp_getnum(N, tobj1, "obj_uid");
			if (n1<1) continue;
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "person")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "userid");
				if (n1==n2) nsp_setnum(N, sobj1, "pid", (int)nsp_getnum(N, sobj2, "id"));
			}
			/* fix emailheader pid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "emailheader")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "accountid");
//				if (nsp_getnum(N, tobj1, "folderid")!=nsp_getnum(N, tobj2, "folderid")) continue;
	//			if (n1==n2) nsp_setnum(N, sobj2, "pid", pid);

				nsp_setstr(N, tobj2, "msg_text", "", 0);

			}
		} else if (strcmp(nsp_getstr(N, sobj1, "class"), "emailfolder")==0) {
			tobj1=nsp_getobj(N, sobj1, "_data");
			n1=(int)nsp_getnum(N, tobj1, "accountid");
			if (n1<1) continue;
			/* fix emailfolder parentfolderid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "emailfolder")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "parentfolderid");
				if (n1==n2) nsp_setnum(N, sobj2, "pid", pid);
			}
			/* fix emailheader folder */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "emailheader")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "accountid");
				if (nsp_getnum(N, tobj1, "folderid")!=nsp_getnum(N, tobj2, "folderid")) continue;
				if (n1==n2) {
					nsp_setnum(N, tobj2, "accountid", nsp_getnum(N, sobj1, "pid"));
//					nsp_setnum(N, tobj2, "folderid", pid);
					nsp_setnum(N, sobj2, "pid", nsp_getnum(N, sobj1, "pid"));
				}
			}
		} else if (strcmp(nsp_getstr(N, sobj1, "class"), "forumgroup")==0) {
			tobj1=nsp_getobj(N, sobj1, "_data");
			n1=(int)nsp_getnum(N, tobj1, "forumgroupid");
			if (n1<1) continue;
			/* fix forum forumgroupid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "forum")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "forumgroupid");
				if (n1!=n2) continue;
				nsp_setnum(N, sobj2, "pid", pid);
			}
		} else if (strcmp(nsp_getstr(N, sobj1, "class"), "forum")==0) {
			tobj1=nsp_getobj(N, sobj1, "_data");
			n1=(int)nsp_getnum(N, tobj1, "forumid");
			if (n1<1) continue;
			/* fix forumpost forumid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "forumpost")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "forumid");
				if (n1!=n2) continue;
				nsp_setnum(N, sobj2, "pid", pid);
			}
		} else if (strcmp(nsp_getstr(N, sobj1, "class"), "webloggroup")==0) {
			tobj1=nsp_getobj(N, sobj1, "_data");
			n1=(int)nsp_getnum(N, tobj1, "groupid");
			if (n1<1) continue;
			/* fix weblogentry entrygroupid */
			sobj2=nsp_getobj(N, robj, "_rows");
			for (sobj2=sobj2->val->d.table.f;sobj2;sobj2=sobj2->next) {
				if (sobj2->val->type!=NT_TABLE) continue;
				if (strcmp(nsp_getstr(N, sobj2, "class"), "weblogentry")!=0) continue;
				tobj2=nsp_getobj(N, sobj2, "_data");
				n2=(int)nsp_getnum(N, tobj2, "entrygroupid");
				if (n1!=n2) continue;
				nsp_setnum(N, sobj2, "pid", pid);
			}
		}
	}
	return 0;
}

int dump_db_version2(char *filename)
{
	nsp_state *N;
	FILE *fp;
	obj_t *robj;
	int did=1;

	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	N=nsp_newstate();
	robj=nsp_settable(N, &N->g, "DATABASE");
	add_table(N, robj, "gw_domains",                "domainid",       did);
	add_table(N, robj, "gw_domains_aliases",        "domainaliasid",  did);

	add_table(N, robj, "gw_groups",                 "groupid",        did);
//	add_table(N, robj, "gw_groups_members",         "groupmemberid",  did);
	add_table(N, robj, "gw_zones",                  "zoneid",         did);
	add_table(N, robj, "gw_users",                  "userid",         did);
	add_table(N, robj, "gw_contacts",               "contactid",      did);

	add_table(N, robj, "gw_bookmarks_folders",      "folderid",       did);
	add_table(N, robj, "gw_bookmarks",              "folderid",       did);
	add_table(N, robj, "gw_calls_actions",          "actionid",       did);
	add_table(N, robj, "gw_calls",                  "callid",         did);
	add_table(N, robj, "gw_email_accounts",         "mailaccountid",  did);
	add_table(N, robj, "gw_email_filters",          "mailfilterid",   did);
	add_table(N, robj, "gw_email_folders",          "mailfolderid",   did);
	add_table(N, robj, "gw_email_headers",          "mailheaderid",   did);
	add_table(N, robj, "gw_events_closings",        "eventclosingid", did);
	add_table(N, robj, "gw_events_types",           "eventtypeid",    did);
	add_table(N, robj, "gw_events",                 "eventid",        did);
	add_table(N, robj, "gw_files",                  "fileid",         did);
	add_table(N, robj, "gw_finance_accounts",       "accountid",      did);
	add_table(N, robj, "gw_finance_inventory",      "inventoryid",    did);
	add_table(N, robj, "gw_finance_invoices",       "invoiceid",      did);
	add_table(N, robj, "gw_finance_invoices_items", "invoiceitemid",  did);
	add_table(N, robj, "gw_finance_journal",        "entryid",        did);
	add_table(N, robj, "gw_forums_groups",          "forumgroupid",   did);
	add_table(N, robj, "gw_forums",                 "forumid",        did);
	add_table(N, robj, "gw_forums_posts",           "messageid",      did);
	add_table(N, robj, "gw_messages",               "messageid",      did);
	add_table(N, robj, "gw_notes",                  "noteid",         did);
	add_table(N, robj, "gw_projects",               "projectid",      did);
	add_table(N, robj, "gw_queries",                "queryid",        did);
	add_table(N, robj, "gw_smtp_relayrules",        "relayruleid",    did);
	add_table(N, robj, "gw_tasks",                  "taskid",         did);
	add_table(N, robj, "gw_weblog_groups",          "groupid",        did);
	add_table(N, robj, "gw_weblog_entries",         "entryid",        did);
	add_table(N, robj, "gw_weblog_comments",        "commentid",      did);
	fix_refs(N, robj);
	/* fix note pids */
	robj=nsp_settable(N, robj, "_rows");
	for (robj=robj->val->d.table.f;robj;robj=robj->next) {
		if (robj->val->type==NT_TABLE) dump_rec(N, fp, robj);
	}
//	nsp_exec(N, "print(\"global _GLOBALS = \");printvar(_GLOBALS);");
	nsp_endstate(N);
	fclose(fp);
	sql_disconnect();
	printf("done.\r\n");
	return -1;
}
