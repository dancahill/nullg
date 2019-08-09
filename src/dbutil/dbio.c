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
#include "nullgw/sql/schema/data.h"
#include "nullgw/sql/schema/fbsql.h"
#include "nullgw/sql/schema/mysql.h"
#include "nullgw/sql/schema/odbc.h"
#include "nullgw/sql/schema/pgsql.h"
#include "nullgw/sql/schema/sqlite.h"

int entryid=0;
char lasto[255];
char ntime[40];

int domainid=0;
int userpid=0;
int contpid=0;
int miscpid=0;

char *ltime(char *sqltime)
{
	unsigned int i, j;

	for (i=0,j=0;i<strlen(sqltime);i++) {
		if (isdigit(sqltime[i])) { ntime[j]=sqltime[i]; j++; }
	}
	ntime[j]='Z';
	ntime[j+1]='\0';
	return ntime;
}

int dump_table(FILE *fp, char *table, char *index)
{
	char query[100];
	char *ptemp;
	int i;
	int j;
	SQLRES sqr;

	if (strcmp(table, "gw_dbinfo")==0) {
		snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY %s ASC", table, index);
	} else {
		snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY obj_did ASC, %s ASC", table, index);
	}
	if (sql_query(&sqr, query)<0) {
		printf("\r\nError dumping %s\r\n", table);
		return -1;
	}
	for (i=0;i<sql_numtuples(&sqr);i++) {
		fprintf(fp, "INSERT INTO %s (", table);
		for (j=0;j<sql_numfields(&sqr);j++) {
			ptemp=sql_getname(&sqr, j);
			if ((strcmp(table, "gw_contacts")==0)||(strcmp(table, "gw_users")==0)) {
				if ((strcmp(ptemp, "loginip")==0)||(strcmp(ptemp, "logintime")==0)||(strcmp(ptemp, "logintoken")==0)) {
					continue;
				}
			}
			fprintf(fp, "%s", sql_getname(&sqr, j));
			if (j<sql_numfields(&sqr)-1) fprintf(fp, ", ");
		}
		fprintf(fp, ") VALUES (");
		for (j=0;j<sql_numfields(&sqr);j++) {
			ptemp=sql_getname(&sqr, j);
			if ((strcmp(table, "gw_contacts")==0)||(strcmp(table, "gw_users")==0)) {
				if ((strcmp(ptemp, "loginip")==0)||(strcmp(ptemp, "logintime")==0)||(strcmp(ptemp, "logintoken")==0)) {
					continue;
				}
			}
			fprintf(fp, "'");
			sqlfprintf(fp, "%s", sql_getvalue(&sqr, i, j));
			fprintf(fp, "'");
			if (j<sql_numfields(&sqr)-1) fprintf(fp, ", ");
		}
		fprintf(fp, ");\n");
	}
	sql_freeresult(&sqr);
	return 0;
}
/*
int dump_table_ldif(FILE *fp, char *table, char *index)
{
	char query[100];
	char dn[255];
	char *ptemp1;
	//char *ptemp2;
	char *ptemp3;
	char *pval;
	int i;
	int j;
	SQLRES sqr;
	//int did=1;

	if (strcmp(table, "gw_users")==0) entryid=99;
	if (strcmp(table, "gw_dbinfo")==0) {
		snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY %s ASC", table, index);
	} else {
		snprintf(query, sizeof(query)-1, "SELECT * FROM %s ORDER BY obj_did ASC, %s ASC", table, index);
	}
	if (sql_query(&sqr, query)<0) {
		printf("\r\nError dumping %s\r\n", table);
		return -1;
	}
	for (i=0;i<sql_numtuples(&sqr);i++) {
		if (strncmp(table, "gw_bookmarks", 12)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=bookmarks,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_calls", 8)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=calls,ou=OtherData,%s", query, lasto);
		} else if (strcmp(table, "gw_contacts")==0) {
			ptemp1="person";
			snprintf(query, sizeof(query)-1, "cn=%s %s", sql_getvaluebyname(&sqr, i, "givenname"), sql_getvaluebyname(&sqr, i, "surname"));
			snprintf(dn, sizeof(dn)-1, "%s,ou=Contacts,ou=People,%s", query, lasto);
		} else if (strcmp(table, "gw_domains")==0) {
			ptemp1="organization";
			snprintf(query, sizeof(query)-1, "o=%s", sql_getvaluebyname(&sqr, i, "domainname"));
			snprintf(lasto, sizeof(lasto)-1, "o=%s", sql_getvaluebyname(&sqr, i, "domainname"));
			snprintf(dn, sizeof(dn)-1, "%s", lasto);
		} else if (strncmp(table, "gw_email", 8)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=email,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_events", 9)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=events,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_files", 8)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=files,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_finance", 10)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=finances,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_forums", 9)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=forums,ou=OtherData,%s", query, lasto);
		} else if (strcmp(table, "gw_groups")==0) {
			ptemp1="groupOfNames";
			snprintf(query, sizeof(query)-1, "cn=%s", sql_getvaluebyname(&sqr, i, "groupname"));
			snprintf(dn, sizeof(dn)-1, "%s,ou=UserGroups,ou=People,%s", query, lasto);
		} else if (strcmp(table, "gw_notes")==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=notes,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_projects", 11)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=projects,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_queries", 10)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=queries,ou=OtherData,%s", query, lasto);
		} else if (strncmp(table, "gw_tasks", 8)==0) {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=tasks,ou=OtherData,%s", query, lasto);
		} else if (strcmp(table, "gw_users")==0) {
			ptemp1="person";
			snprintf(query, sizeof(query)-1, "cn=%s %s", sql_getvaluebyname(&sqr, i, "givenname"), sql_getvaluebyname(&sqr, i, "surname"));
//			snprintf(query, sizeof(query)-1, "cn=%s", sql_getvaluebyname(&sqr, i, "username"));
			snprintf(dn, sizeof(dn)-1, "%s,ou=Users,ou=People,%s", query, lasto);
		} else if (strcmp(table, "gw_zones")==0) {
			ptemp1="locality";
			snprintf(query, sizeof(query)-1, "cn=%s", sql_getvaluebyname(&sqr, i, "zonename"));
			snprintf(dn, sizeof(dn)-1, "%s,ou=zones,ou=OtherData,%s", query, lasto);
		} else {
			ptemp1="organizationalRole";
			snprintf(query, sizeof(query)-1, "cn=%s.%d", table, entryid);
			snprintf(dn, sizeof(dn)-1, "%s,ou=OtherData,%s", query, lasto);
		}
		//ptemp2=query;
		//did=atoi(sql_getvaluebyname(&sqr, i, "obj_did"));
		if (strcmp(table, "gw_domains_aliases")==0) {
		} else {
			fprintf(fp, "\n");
			entryid++;
			fprintf(fp, "dn: %s\n", dn);
			if (strcmp(table, "gw_domains")==0) {
				fprintf(fp, "objectClass: top\n");
			}
			if (ptemp1!=NULL) fprintf(fp, "objectClass: %s\n", ptemp1);
			if (strcmp(table, "gw_domains")==0) {
				fprintf(fp, "objectClass: domainRelatedObject\n");
			}
			fprintf(fp, "objectClass: extensibleObject\n");
		}
		for (j=0;j<sql_numfields(&sqr);j++) {
			ptemp3=sql_getname(&sqr, j);
			if (strcmp(ptemp3, "obj_uid")==0) continue;
			if (strcmp(ptemp3, "obj_gid")==0) continue;
			if (strcmp(ptemp3, "obj_did")==0) continue;
			if (strcmp(ptemp3, "obj_gperm")==0) continue;
			if (strcmp(ptemp3, "obj_operm")==0) continue;
			if (strcmp(ptemp3, "obj_ctime")==0) continue;//ptemp3="createTimestamp";
			if (strcmp(ptemp3, "obj_mtime")==0) continue;//ptemp3="modifyTimestamp";// 20001128175739Z
//			if (strcmp(ptemp3, "obj_uid")==0)   ptemp3="creatorsName";// modifiersName
			if ((strcmp(table, "gw_bookmarks_folders")==0)&&(strcmp(ptemp3, "folderid")==0))          fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_bookmarks")==0)&&(strcmp(ptemp3, "folderid")==0))             fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_calls_actions")==0)&&(strcmp(ptemp3, "actionid")==0))         fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_contacts")==0)&&(strcmp(ptemp3, "username")==0))              fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_email_accounts")==0)&&(strcmp(ptemp3, "mailaccountid")==0))   fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_email_folders")==0)&&(strcmp(ptemp3, "mailfolderid")==0))     fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_email_headers")==0)&&(strcmp(ptemp3, "mailheaderid")==0))     fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_events_closings")==0)&&(strcmp(ptemp3, "eventclosingid")==0)) fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_events_types")==0)&&(strcmp(ptemp3, "eventtypeid")==0))       fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_files")==0)&&(strcmp(ptemp3, "fileid")==0))                   fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_finance_accounts")==0)&&(strcmp(ptemp3, "accountid")==0))     fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_finance_inventory")==0)&&(strcmp(ptemp3, "inventoryid")==0))  fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_finance_journal")==0)&&(strcmp(ptemp3, "entryid")==0))        fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_forums_groups")==0)&&(strcmp(ptemp3, "forumgroupid")==0))     fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_forums_posts")==0)&&(strcmp(ptemp3, "messageid")==0))         fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_forums")==0)&&(strcmp(ptemp3, "forumid")==0))                 fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_notes")==0)&&(strcmp(ptemp3, "noteid")==0))                   fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_projects")==0)&&(strcmp(ptemp3, "projectid")==0))             fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_queries")==0)&&(strcmp(ptemp3, "queryid")==0))                fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_smtp_relayrules")==0)&&(strcmp(ptemp3, "relayruleid")==0))    fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_tasks")==0)&&(strcmp(ptemp3, "taskid")==0))                   fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_weblog_groups")==0)&&(strcmp(ptemp3, "groupid")==0))          fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_weblog_entries")==0)&&(strcmp(ptemp3, "entryid")==0))         fprintf(fp, "cn: %s\n", query+3);
			else if ((strcmp(table, "gw_users")==0)&&(strcmp(ptemp3, "username")==0))                 fprintf(fp, "cn: %s\n", query+3);
			if (strcmp(table, "gw_bookmarks")==0) {
				if (strcmp(ptemp3, "bookmarkid")==0) continue;
				else if (strcmp(ptemp3, "bookmarkname")==0) ptemp3="cn";
				else if (strcmp(ptemp3, "bookmarkurl")==0) ptemp3="labeledURI";
				else {
					pval=sql_getvalue(&sqr, i, j);
					fprintf(fp, "description: %s: ", ptemp3);
					sqlfprintf(fp, "%s", pval);
					fprintf(fp, "\n");
					continue;
				}
			} else if (strcmp(table, "gw_contacts")==0) {
				if (strcmp(ptemp3, "folderid")==0) continue;
				else if (strcmp(ptemp3, "surname")==0) ptemp3="sn";
				else if (strcmp(ptemp3, "givenname")==0) ptemp3="gn";
				else if (strcmp(ptemp3, "username")==0) ptemp3="uid";
				else if (strcmp(ptemp3, "password")==0) ptemp3="userPassword";
				else if (strcmp(ptemp3, "salutation")==0) ptemp3="personalTitle";
				else if (strcmp(ptemp3, "homenumber")==0) ptemp3="homeTelephoneNumber";
				else if (strcmp(ptemp3, "faxnumber")==0) ptemp3="facsimileTelephoneNumber";
				else if (strcmp(ptemp3, "cellnumber")==0) ptemp3="mobileTelephoneNumber";
				else if (strcmp(ptemp3, "mobilenumber")==0) ptemp3="mobileTelephoneNumber";
				else if (strcmp(ptemp3, "pagernumber")==0) ptemp3="pagerTelephoneNumber";
				else if (strcmp(ptemp3, "email")==0) ptemp3="mail";
				else if (strcmp(ptemp3, "website")==0) ptemp3="labeledURI";
				else if (strcmp(ptemp3, "organization")==0) ptemp3="o";
				else if (strcmp(ptemp3, "jobtitle")==0) ptemp3="title";
				else if (strcmp(ptemp3, "address")==0) ptemp3="streetAddress";
				else if (strcmp(ptemp3, "locality")==0) ptemp3="localityName";
				else if (strcmp(ptemp3, "region")==0) ptemp3="stateOrProvinceName";
				else if (strcmp(ptemp3, "country")==0) ptemp3="friendlyCountryName";
				else if (strcmp(ptemp3, "postalcode")==0) ptemp3="postalCode";
				else {
					pval=sql_getvalue(&sqr, i, j);
					fprintf(fp, "description: %s: ", ptemp3);
					sqlfprintf(fp, "%s", pval);
					fprintf(fp, "\n");
					continue;
				}
			} else if (strcmp(table, "gw_domains")==0) {
				if (strcmp(ptemp3, "domainid")==0) continue;
				else if (strcmp(ptemp3, "domainname")==0) ptemp3="o";
			} else if (strcmp(table, "gw_domains_aliases")==0) {
				if (strcmp(ptemp3, "domainaliasid")==0) continue;
				else if (strcmp(ptemp3, "createTimestamp")==0) continue;
				else if (strcmp(ptemp3, "modifyTimestamp")==0) continue;
				else if (strcmp(ptemp3, "domainid")==0) continue;
				else if (strcmp(ptemp3, "domainname")==0) ptemp3="associatedDomain";
			} else if (strcmp(table, "gw_groups")==0) {
				if (strcmp(ptemp3, "groupname")==0) ptemp3="cn";
				else continue;
			} else if (strcmp(table, "gw_users")==0) {
				if (strcmp(ptemp3, "surname")==0) ptemp3="sn";
				else if (strcmp(ptemp3, "givenname")==0) ptemp3="gn";
				else if (strcmp(ptemp3, "username")==0) ptemp3="uid";
				else if (strcmp(ptemp3, "jobtitle")==0) ptemp3="title";
				else if (strcmp(ptemp3, "password")==0) ptemp3="userPassword";
				else if (strcmp(ptemp3, "preflanguage")==0) ptemp3="preferredLanguage";
				else if (strcmp(ptemp3, "homenumber")==0) ptemp3="homeTelephoneNumber";
				else if (strcmp(ptemp3, "faxnumber")==0) ptemp3="facsimileTelephoneNumber";
				else if (strcmp(ptemp3, "cellnumber")==0) ptemp3="mobileTelephoneNumber";
				else if (strcmp(ptemp3, "pagernumber")==0) ptemp3="pagerTelephoneNumber";
				else if (strcmp(ptemp3, "email")==0) ptemp3="mail";
				else if (strcmp(ptemp3, "address")==0) ptemp3="streetAddress";
				else if (strcmp(ptemp3, "locality")==0) ptemp3="localityName";
				else if (strcmp(ptemp3, "region")==0) ptemp3="stateOrProvinceName";
				else if (strcmp(ptemp3, "country")==0) ptemp3="friendlyCountryName";
				else if (strcmp(ptemp3, "postalcode")==0) ptemp3="postalCode";
				else if (strcmp(ptemp3, "userid")==0) ptemp3="uidNumber";
//				else if (strcmp(ptemp3, "groupid")==0) continue;
//				else if (strcmp(ptemp3, "domainid")==0) continue;
//				else if (strcmp(ptemp3, "enabled")==0) continue;
//				else if (strcmp(ptemp3, "birthdate")==0) continue;
//				else if (strcmp(ptemp3, "hiredate")==0) continue;
//				else if (strcmp(ptemp3, "sin")==0) continue;
//				else if (strcmp(ptemp3, "isactive")==0) continue;
//				else if (strcmp(ptemp3, "division")==0) continue;
//				else if (strcmp(ptemp3, "supervisor")==0) continue;
//				else if (strncmp(ptemp3, "auth", 4)==0) continue;
//				else if (strncmp(ptemp3, "pref", 4)==0) continue;
//				else if (strcmp(ptemp3, "availability")==0) continue;
				else {
					pval=sql_getvalue(&sqr, i, j);
					fprintf(fp, "description: %s: ", ptemp3);
					sqlfprintf(fp, "%s", pval);
					fprintf(fp, "\n");
					continue;
				}
				// postalAddress: Example, Inc. $ 535 W. William St. $ Anytown, MI 48109 $ US
			} else if (strcmp(table, "gw_zones")==0) {
				if (strcmp(ptemp3, "zoneid")==0) continue;
				else if (strcmp(ptemp3, "zonename")==0) ptemp3="cn";
			} else {
					pval=sql_getvalue(&sqr, i, j);
					fprintf(fp, "description: %s: ", ptemp3);
					sqlfprintf(fp, "%s", pval);
					fprintf(fp, "\n");
					continue;
			}
			pval=sql_getvalue(&sqr, i, j);
			if ((strcmp(ptemp3, "createTimestamp")==0)||(strcmp(ptemp3, "modifyTimestamp")==0)) {
				pval=ltime(pval);
			}
			if (strlen(pval)==0) {
//				fprintf(fp, "%s: \"\"\n", ptemp3);
			} else {
				fprintf(fp, "%s: ", ptemp3);
				sqlfprintf(fp, "%s", pval);
				fprintf(fp, "\n");
			}
			if ((strcmp(table, "gw_domains")==0)&&(strcmp(ptemp3, "o")==0)) {
				fprintf(fp, "associatedDomain: %s\n", pval);
			}
			if ((strcmp(table, "gw_groups")==0)&&(strcmp(ptemp3, "cn")==0)) {
				// just a dummy entry for now
				fprintf(fp, "member: cn=Dan Cahill,ou=Users,ou=People,o=gw.nulllogic.lan\n");
			}
		}
	}
	if (strcmp(table, "gw_domains_aliases")==0) {
		fprintf(fp, "\ndn: ou=People,%s\nobjectClass: organizationalUnit\nou: People\n", lasto);
		fprintf(fp, "\ndn: ou=Contacts,ou=People,%s\nobjectClass: organizationalUnit\nou: Contacts\n", lasto);
		fprintf(fp, "\ndn: ou=ContactGroups,ou=People,%s\nobjectClass: organizationalUnit\nou: ContactGroups\n", lasto);
		fprintf(fp, "\ndn: ou=Users,ou=People,%s\nobjectClass: organizationalUnit\nou: Users\n", lasto);
		fprintf(fp, "\ndn: ou=UserGroups,ou=People,%s\nobjectClass: organizationalUnit\nou: UserGroups\n", lasto);
		fprintf(fp, "\ndn: ou=OtherData,%s\nobjectClass: organizationalUnit\nou: OtherData\n", lasto);

		fprintf(fp, "\ndn: ou=bookmarks,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: bookmarks\n", lasto);
		fprintf(fp, "\ndn: ou=calls,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: calls\n", lasto);
		fprintf(fp, "\ndn: ou=email,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: email\n", lasto);
		fprintf(fp, "\ndn: ou=events,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: events\n", lasto);
		fprintf(fp, "\ndn: ou=files,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: files\n", lasto);
		fprintf(fp, "\ndn: ou=finances,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: finances\n", lasto);
		fprintf(fp, "\ndn: ou=forums,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: forums\n", lasto);
		fprintf(fp, "\ndn: ou=notes,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: notes\n", lasto);
		fprintf(fp, "\ndn: ou=projects,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: projects\n", lasto);
		fprintf(fp, "\ndn: ou=queries,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: queries\n", lasto);
		fprintf(fp, "\ndn: ou=tasks,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: tasks\n", lasto);
		fprintf(fp, "\ndn: ou=zones,ou=OtherData,%s\nobjectClass: organizationalUnit\nou: zones\n", lasto);
		entryid+=4;
	}
	sql_freeresult(&sqr);
	return 0;
}
*/
int init_table(char *query, char *tablename)
{
	if (strcasecmp(config.sql_type, "FBSQL")==0) {
		sql_updatef(0, "DROP TABLE %s;", tablename);
	} else if (strcasecmp(config.sql_type, "MYSQL")==0) {
		sql_updatef(0, "DROP TABLE IF EXISTS %s;", tablename);
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		sql_updatef(0, "DROP TABLE %s;", tablename);
	} else if (strncasecmp(config.sql_type, "SQLITE", 6)==0) {
		sql_updatef(0, "DROP TABLE %s;", tablename);
	}
	if (sql_update(1, query)<0) {
		printf("\r\nError inserting %s\r\n", tablename);
		return -1;
	}
	return 0;
}

int fbsql_generator_sync(void)
{
	int max;
	SQLRES sqr;
	int i;

	//printf("fbsql_generator_sync started;\r\n");
	for (i=0;;i++) {
		if (fbsqldb_tables[i].name==NULL) break;
		if (fbsqldb_tables[i].seqname==NULL) continue;
		if (sql_queryf(&sqr, "SELECT MAX(%s) FROM %s", fbsqldb_tables[i].index, fbsqldb_tables[i].name)<0) return -1;
		if (sql_numtuples(&sqr)!=1) {
			sql_freeresult(&sqr);
			printf("sql error;\r\n");
			return -1;
		}
		max=atoi(sql_getvalue(&sqr, 0, 0));
		sql_freeresult(&sqr);
		//printf("SET GENERATOR GEN_%s TO %d;\r\n", fbsqldb_tables[i].name, max);
		sql_updatef(0, "SET GENERATOR GEN_%s TO %d;", fbsqldb_tables[i].name, max);
	}
	//printf("fbsql_generator_sync done;\r\n");
	return 0;
}

int pgsql_sequence_sync(void)
{
	int max;
	int seq;
	SQLRES sqr;
	int i;

	for (i=0;;i++) {
		if (pgsqldb_tables[i].seqname==NULL) break;
		if (sql_queryf(&sqr, "SELECT max(%s) FROM %s", pgsqldb_tables[i].index, pgsqldb_tables[i].name)<0) return -1;
		if (sql_numtuples(&sqr)!=1) {
			sql_freeresult(&sqr);
			return -1;
		}
		max=atoi(sql_getvalue(&sqr, 0, 0))+1;
		sql_freeresult(&sqr);
		if (sql_queryf(&sqr, "SELECT last_value FROM %s", pgsqldb_tables[i].seqname)<0) return -1;
		if (sql_numtuples(&sqr)!=1) {
			sql_freeresult(&sqr);
			return -1;
		}
		seq=atoi(sql_getvalue(&sqr, 0, 0));
		sql_freeresult(&sqr);
		if (seq<max) seq=max;
		if (sql_queryf(&sqr, "SELECT setval ('\"%s\"', %d, false);", pgsqldb_tables[i].seqname, seq)<0) return -1;
		if (sql_numtuples(&sqr)!=1) {
			sql_freeresult(&sqr);
			return -1;
		}
		sql_freeresult(&sqr);
	}
	return 0;
}

int table_check()
{
	SQLRES sqr;
	int i;
	int x;

	/* CHECK gw_dbinfo TABLE */
	if (sql_query(&sqr, "SELECT count(*) FROM gw_dbinfo")<0) return -1;
	x=atoi(sql_getvalue(&sqr, 0, 0));
	sql_freeresult(&sqr);
	if (x==0) {
		for (i=0;sqldata_new[i]!=NULL;i++) {
			if (sql_update(1, sqldata_new[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_new[i]); return -1; }
		}
	}
	/* CHECK gw_calls_actions TABLE */
	if (sql_query(&sqr, "SELECT count(*) FROM gw_calls_actions")<0) return -1;
	x=atoi(sql_getvalue(&sqr, 0, 0));
	sql_freeresult(&sqr);
	if (x==0) {
		for (i=0;sqldata_callactions[i]!=NULL;i++) {
			if (sql_update(1, sqldata_callactions[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_callactions[i]); return -1; }
		}
	}
	/* CHECK gw_domains TABLE */
	if (sql_query(&sqr, "SELECT count(*) FROM gw_domains")<0) return -1;
	x=atoi(sql_getvalue(&sqr, 0, 0));
	sql_freeresult(&sqr);
	if (x==0) {
		for (i=0;sqldata_domains[i]!=NULL;i++) {
			if (sql_update(1, sqldata_domains[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_domains[i]); return -1; }
		}
	}
	/* CHECK gw_eventclosings TABLE */
	if (sql_query(&sqr, "SELECT count(*) FROM gw_events_closings")<0) return -1;
	x=atoi(sql_getvalue(&sqr, 0, 0));
	sql_freeresult(&sqr);
	if (x==0) {
		for (i=0;sqldata_eventclosings[i]!=NULL;i++) {
			if (sql_update(1, sqldata_eventclosings[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_eventclosings[i]); return -1; }
		}
	}
	/* CHECK gw_eventtypes TABLE */
	if (sql_query(&sqr, "SELECT count(*) FROM gw_events_types")<0) return -1;
	x=atoi(sql_getvalue(&sqr, 0, 0));
	sql_freeresult(&sqr);
	if (x==0) {
		for (i=0;sqldata_eventtypes[i]!=NULL;i++) {
			if (sql_update(1, sqldata_eventtypes[i])<0) { printf("\r\nError inserting '%s'\r\n", sqldata_eventtypes[i]); return -1; }
		}
	}
	/* Assign all orphaned records to domain 1 */
	sql_updatef(1, "UPDATE gw_users SET obj_did = 1, domainid = 1 WHERE domainid = 0");
	sql_updatef(1, "UPDATE gw_groups SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_contacts SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_activity SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_bookmarks SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_bookmarks_folders SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_calls SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_email_accounts SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_email_folders SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_email_headers SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_events SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_files SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_finance_invoices SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_notes SET obj_did = 1 WHERE obj_did = 0");
	sql_updatef(1, "UPDATE gw_tasks SET obj_did = 1 WHERE obj_did = 0");
	/* Update the db version */
	sql_updatef(1, "UPDATE gw_dbinfo SET dbversion = '%s'", PACKAGE_VERSION);
	return 0;
}

int dump_db_v2(char *filename, int dumptype)
{
/*
	FILE *fp;

	printf("Dumping %s database to %s...", config.sql_type, filename);
#ifndef WIN32
	umask(077);
#endif
	if (dumptype==DUMP_NGW2) {
		dump_db_version2(filename);
		return -1;
	}
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	if (dumptype==DUMP_LDIF) {
		fprintf(fp, "# this so-called ldif file is not really ldif-compatible.\n");
		dump_table_ldif(fp, "gw_domains",                "domainid");
		dump_table_ldif(fp, "gw_domains_aliases",        "domainaliasid");
		dump_table_ldif(fp, "gw_groups",                 "groupid");
//		dump_table_ldif(fp, "gw_groups_members",         "groupmemberid");
		dump_table_ldif(fp, "gw_zones",                  "zoneid");
		dump_table_ldif(fp, "gw_users",                  "userid");
		dump_table_ldif(fp, "gw_contacts",               "contactid");

		dump_table_ldif(fp, "gw_bookmarks_folders",      "folderid");
		dump_table_ldif(fp, "gw_bookmarks",              "folderid");
		dump_table_ldif(fp, "gw_calls_actions",          "actionid");
		dump_table_ldif(fp, "gw_calls",                  "callid");
		dump_table_ldif(fp, "gw_email_accounts",         "mailaccountid");
		dump_table_ldif(fp, "gw_email_filters",          "mailfilterid");
		dump_table_ldif(fp, "gw_email_folders",          "mailfolderid");
		dump_table_ldif(fp, "gw_email_headers",          "mailheaderid");
		dump_table_ldif(fp, "gw_events_closings",        "eventclosingid");
		dump_table_ldif(fp, "gw_events_types",           "eventtypeid");
		dump_table_ldif(fp, "gw_events",                 "eventid");
		dump_table_ldif(fp, "gw_files",                  "fileid");
		dump_table_ldif(fp, "gw_finance_accounts",       "accountid");
		dump_table_ldif(fp, "gw_finance_inventory",      "inventoryid");
		dump_table_ldif(fp, "gw_finance_invoices",       "invoiceid");
		dump_table_ldif(fp, "gw_finance_invoices_items", "invoiceitemid");
		dump_table_ldif(fp, "gw_finance_journal",        "entryid");
		dump_table_ldif(fp, "gw_forums_groups",          "forumgroupid");
		dump_table_ldif(fp, "gw_forums_posts",           "messageid");
		dump_table_ldif(fp, "gw_forums",                 "forumid");
		dump_table_ldif(fp, "gw_messages",               "messageid");
		dump_table_ldif(fp, "gw_notes",                  "noteid");
		dump_table_ldif(fp, "gw_projects",               "projectid");
		dump_table_ldif(fp, "gw_queries",                "queryid");
		dump_table_ldif(fp, "gw_smtp_relayrules",        "relayruleid");
		dump_table_ldif(fp, "gw_tasks",                  "taskid");
		dump_table_ldif(fp, "gw_weblog_groups",          "groupid");
//		dump_table_ldif(fp, "gw_weblog_entries",         "entryid");
		dump_table_ldif(fp, "gw_weblog_comments",        "commentid");
	}
	fclose(fp);
	printf("done.\r\n");
	sql_disconnect();
*/
	return -1;
}

int dump_db(char *filename)
{
	FILE *fp;

	printf("Dumping %s database to %s...", config.sql_type, filename);
#ifndef WIN32
	umask(077);
#endif
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	dump_table(fp, "gw_dbinfo",                 "dbversion");
	dump_table(fp, "gw_users",                  "userid");
	dump_table(fp, "gw_users_sessions",         "sessionid");
	dump_table(fp, "gw_activity",               "activityid");
	dump_table(fp, "gw_bookmarks",              "bookmarkid");
	dump_table(fp, "gw_bookmarks_folders",      "folderid");
	dump_table(fp, "gw_calls",                  "callid");
	dump_table(fp, "gw_calls_actions",          "actionid");
	dump_table(fp, "gw_contacts",               "contactid");
	dump_table(fp, "gw_contacts_folders",       "folderid");
	dump_table(fp, "gw_contacts_sessions",      "sessionid");
	dump_table(fp, "gw_domains",                "domainid");
	dump_table(fp, "gw_domains_aliases",        "domainaliasid");
	dump_table(fp, "gw_email_accounts",         "mailaccountid");
	dump_table(fp, "gw_email_filters",          "mailfilterid");
	dump_table(fp, "gw_email_folders",          "mailfolderid");
	dump_table(fp, "gw_email_headers",          "mailheaderid");
	dump_table(fp, "gw_events",                 "eventid");
	dump_table(fp, "gw_events_closings",        "eventclosingid");
	dump_table(fp, "gw_events_types",           "eventtypeid");
	dump_table(fp, "gw_files",                  "fileid");
	dump_table(fp, "gw_finance_accounts",       "accountid");
	dump_table(fp, "gw_finance_inventory",      "inventoryid");
	dump_table(fp, "gw_finance_invoices",       "invoiceid");
	dump_table(fp, "gw_finance_invoices_items", "invoiceitemid");
	dump_table(fp, "gw_finance_journal",        "entryid");
	dump_table(fp, "gw_forums",                 "forumid");
	dump_table(fp, "gw_forums_groups",          "forumgroupid");
	dump_table(fp, "gw_forums_posts",           "messageid");
	dump_table(fp, "gw_groups",                 "groupid");
	dump_table(fp, "gw_groups_members",         "groupmemberid");
	dump_table(fp, "gw_messages",               "messageid");
	dump_table(fp, "gw_notes",                  "noteid");
	dump_table(fp, "gw_projects",               "projectid");
	dump_table(fp, "gw_queries",                "queryid");
	dump_table(fp, "gw_smtp_relayrules",        "relayruleid");
	dump_table(fp, "gw_tasks",                  "taskid");
	dump_table(fp, "gw_weblog_comments",        "commentid");
	dump_table(fp, "gw_weblog_entries",         "entryid");
	dump_table(fp, "gw_weblog_groups",          "groupid");
	dump_table(fp, "gw_zones",                  "zoneid");
	fclose(fp);
	printf("done.\r\n");
	sql_disconnect();
	return -1;
}

static int init_fbsql_tables()
{
	int i;

#if defined(linux)
	printf("\r\n");
	for (i=0;;i++) {
		if (fbsqldb_tables[i].name==NULL) break;
		printf("-");
	}
	printf("\r");
#endif
	for (i=0;;i++) {
		if (fbsqldb_tables[i].name==NULL) break;
		/* drop any old triggers and generators */
		if (fbsqldb_tables[i].seqname!=NULL) {
			sql_updatef(0, "DROP TRIGGER %s_BI;", fbsqldb_tables[i].name);
			sql_updatef(0, "DROP GENERATOR GEN_%s;", fbsqldb_tables[i].name);
		}
		/* create the table */
		if (init_table(fbsqldb_tables[i].schema, fbsqldb_tables[i].name)!=0) return -1;
		/* create generators and triggers for this table */
		if (fbsqldb_tables[i].seqname!=NULL) {
			sql_updatef(0, "CREATE GENERATOR GEN_%s;", fbsqldb_tables[i].name);
			sql_updatef(0, "SET GENERATOR GEN_%s TO 0;", fbsqldb_tables[i].name);
			sql_updatef(0, "CREATE TRIGGER %s_BI FOR %s\n"
				"ACTIVE BEFORE INSERT POSITION 0\n"
				"AS\n"
				"BEGIN\n"
				"IF (NEW.%s IS NULL OR NEW.%s=0) THEN NEW.%s = GEN_ID(GEN_%s, 1);\n"
				"END\n"
				, fbsqldb_tables[i].name, fbsqldb_tables[i].name
				, fbsqldb_tables[i].index, fbsqldb_tables[i].index, fbsqldb_tables[i].index, fbsqldb_tables[i].name
			);
		}
#if defined(linux)
		printf("=");
#endif
	}
#if defined(linux)
	printf("\r\n");
#endif
	return 0;
}

int init_db(void)
{
	int i;

	if (strcasecmp(config.sql_type, "FBSQL")==0) {
		printf("Initialising FirebirdSQL database...");
		init_fbsql_tables();
	} else if (strcasecmp(config.sql_type, "MYSQL")==0) {
		printf("Initialising MySQL database...");
		for (i=0;;i++) {
			if (mysqldb_tables[i].name==NULL) break;
			if (init_table(mysqldb_tables[i].schema, mysqldb_tables[i].name)!=0) return -1;
		}
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
		printf("Initialising ODBC *.mdb database...");
		for (i=0;;i++) {
			if (mdb_tables[i].name==NULL) break;
			if (init_table(mdb_tables[i].schema, mdb_tables[i].name)!=0) return -1;
		}
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		printf("Initialising PostgreSQL database...");
		for (i=0;;i++) {
			if (pgsqldb_tables[i].seqname==NULL) break;
			sql_updatef(0, "DROP SEQUENCE %s;", pgsqldb_tables[i].seqname);
			sql_updatef(0, "CREATE SEQUENCE %s start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;", pgsqldb_tables[i].seqname);
		}
		for (i=0;;i++) {
			if (pgsqldb_tables[i].name==NULL) break;
			if (init_table(pgsqldb_tables[i].schema, pgsqldb_tables[i].name)!=0) return -1;
		}
	} else if (strncasecmp(config.sql_type, "SQLITE", 6)==0) {
		printf("Initialising SQLite database...");
		for (i=0;;i++) {
			if (sqlitedb_tables[i].name==NULL) break;
			if (init_table(sqlitedb_tables[i].schema, sqlitedb_tables[i].name)!=0) return -1;
		}
	}
	table_check();
	if (strcasecmp(config.sql_type, "FBSQL")==0) {
		fbsql_generator_sync();
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		pgsql_sequence_sync();
	}
	if (strlen(rootpass)>0) {
		if (sql_updatef(1, "UPDATE gw_users SET password = '%s' WHERE userid = 1", rootpass)<0) {
			printf("\r\nError setting root password\r\n");
			return -1;
		}
	}
	printf("done.\r\n");
	sql_disconnect();
	return 0;
}

int restore_db(char *filename)
{
	char line[32768];
	char *pTemp;
	FILE *fp;
	int i;

	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("Could not open source file.\r\n");
		return -1;
	}
	if (strcasecmp(config.sql_type, "FBSQL")==0) {
		printf("Restoring FirebirdSQL database from %s...", filename);
		init_fbsql_tables();
	} else if (strcasecmp(config.sql_type, "MYSQL")==0) {
		printf("Restoring MySQL database from %s...", filename);
		for (i=0;;i++) {
			if (mysqldb_tables[i].name==NULL) break;
			if (init_table(mysqldb_tables[i].schema, mysqldb_tables[i].name)!=0) return -1;
		}
#ifdef WIN32
	} else if (strcasecmp(config.sql_type, "ODBC")==0) {
		printf("Restoring ODBC database from %s...", filename);
		for (i=0;;i++) {
			if (mdb_tables[i].name==NULL) break;
			if (init_table(mdb_tables[i].schema, mdb_tables[i].name)!=0) return -1;
		}
#endif
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		printf("Restoring PostgreSQL database from %s...", filename);
		for (i=0;;i++) {
			if (pgsqldb_tables[i].seqname==NULL) break;
			sql_updatef(0, "DROP SEQUENCE %s;", pgsqldb_tables[i].seqname);
			sql_updatef(0, "CREATE SEQUENCE %s start 1 increment 1 maxvalue 2147483647 minvalue 1 cache 1;", pgsqldb_tables[i].seqname);
		}
		for (i=0;;i++) {
			if (pgsqldb_tables[i].name==NULL) break;
			if (init_table(pgsqldb_tables[i].schema, pgsqldb_tables[i].name)!=0) return -1;
		}
	} else if (strncasecmp(config.sql_type, "SQLITE", 6)==0) {
		printf("Restoring SQLite database from %s...", filename);
		for (i=0;;i++) {
			if (sqlitedb_tables[i].name==NULL) break;
			if (init_table(sqlitedb_tables[i].schema, sqlitedb_tables[i].name)!=0) return -1;
		}
	}
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) {
			line[strlen(line)-1]='\0';
		}
		if (strncasecmp(line, "INSERT", 6)==0) {
			pTemp=line;
			while (*pTemp) {
				if ((pTemp[0]=='\\')&&(pTemp[1]=='n')) {
					pTemp[0]=13;
					pTemp[1]=10;
				}
				pTemp++;
			}
			if (sql_update(1, line)<0) return -1;
		}
	}
	if (strcasecmp(config.sql_type, "FBSQL")==0) {
		fbsql_generator_sync();
	} else if (strcasecmp(config.sql_type, "PGSQL")==0) {
		pgsql_sequence_sync();
	}
	fclose(fp);
	table_check();
	printf("done.\r\n");
	sql_disconnect();
	return 0;
}
