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
#include "main.h"
/****************************************************************************
 *	queryadd()
 *
 *	Purpose	: Display form for adding or editing queries
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void queryadd(int sid)
{
	char queryname[1024];
	char query[8192];
	int queryid=0;
	int sqr;

	if (auth(sid, "query")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	memset(queryname, 0, sizeof(queryname));
	memset(query, 0, sizeof(query));
	if (getgetenv(sid, "QUERYID")!=NULL)
		queryid=atoi(getgetenv(sid, "QUERYID"));
	if (queryid>0) {
		if ((sqr=sqlQueryf("SELECT queryname, query FROM gw_queries WHERE queryid = %d", queryid))<0) return;
		strncpy(queryname, sqlGetvalue(sqr, 0, 0), sizeof(queryname)-1);
		strncpy(query, sqlGetvalue(sqr, 0, 1), sizeof(query)-1);
		sqlFreeconnect(sqr);
	}
	prints("<SCRIPT LANGUAGE=\"JavaScript\">\n");
	prints("<!--\n");
	prints("function ConfirmDelete() {\n");
	prints("	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints("}\n");
	prints("// -->\n");
	prints("</SCRIPT>\n");
	prints("<CENTER>\n<FORM METHOD=POST ACTION=%s/querysave NAME=queryrun>\n", conn[sid].dat->in_ScriptName);
	prints("<INPUT TYPE=HIDDEN NAME=queryid VALUE=%d>\n", queryid);
	prints("<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (queryid!=0) {
		prints("<TR BGCOLOR=%s><TH>Query Number %d</TH></TR>\n", COLOR_TRIM, queryid);
	} else {
		prints("<TR BGCOLOR=%s><TH>New Query\n</TH></TR>", COLOR_TRIM);
	}
	prints("<TR BGCOLOR=%s><TD><B>Query Name</B></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD><INPUT TYPE=TEXT NAME=queryname VALUE='%s' SIZE=60></TD></TR>\n", COLOR_EDITFORM, queryname);
	prints("<TR BGCOLOR=%s><TD><B>Query</B></TD></TR>\n", COLOR_EDITFORM);
	prints("<TR BGCOLOR=%s><TD><TEXTAREA NAME=sqlquery ROWS=3 COLS=60>%s</TEXTAREA></TD></TR>\n", COLOR_EDITFORM, query);
	prints("</TABLE>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Save'>\n");
	prints("<INPUT TYPE=SUBMIT NAME=submit VALUE='Run'>\n");
	if (queryid!=0) {
		prints("<INPUT TYPE=SUBMIT NAME=Submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints("</FORM>\n</CENTER>\n");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.queryrun.queryname.focus();\n// -->\n</SCRIPT>\n");
	return;
}

/****************************************************************************
 *	queryrun()
 *
 *	Purpose	: Execute query and display results
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void queryrun(int sid)
{
	char query[8192];
	char queryname[1024];
	char querytype[10];
	char *pquerytype;
	int authlevel;
	int queryid=0;
	int offset=0;
	int i, j;
	int sqr;

	if (auth(sid, "query")<1) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if (auth(sid, "query")<4) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
	}
	memset(query, 0, sizeof(query));
	memset(queryname, 0, sizeof(queryname));
	memset(querytype, 0, sizeof(querytype));
	if ((strcmp(conn[sid].dat->in_RequestMethod, "POST")!=0)&&(getgetenv(sid, "QUERYID")==NULL)) {
		authlevel=auth(sid, "query");
		if ((sqr=sqlQuery("SELECT queryid, queryname, query FROM gw_queries ORDER BY queryname ASC"))<0) return;
		prints("<CENTER>\n");
		if (sqlNumtuples(sqr)>0) {
			prints("Saved queries<BR>\n");
			prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n<TR BGCOLOR=%s>\n", COLOR_TRIM);
			if (authlevel>3) {
				prints("<TH>&nbsp;</TH>");
			}
			prints("<TH WIDTH=200 NOWRAP>Query Name</TH></TR>\n");
			for (i=0;i<sqlNumtuples(sqr);i++) {
				prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
				if (authlevel>3) {
					prints("<TD NOWRAP><A HREF=%s/queryadd?queryid=%s>edit</A></TD>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
				}
				prints("<TD NOWRAP><A HREF=%s/queryrun?queryid=%s>", conn[sid].dat->in_ScriptName, sqlGetvalue(sqr, i, 0));
				prints("%s&nbsp;</A></TD>", sqlGetvalue(sqr, i, 1));
				prints("</TR>\n");
			}
			prints("</TABLE>\n");
		} else {
			prints("There are no saved queries<BR>\n");
		}
		sqlFreeconnect(sqr);
		if (auth(sid, "query")>3) {
			prints("<A HREF=%s/queryadd>New Query</A>\n", conn[sid].dat->in_ScriptName);
		}
		prints("</CENTER>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod, "POST")==0) {
		if (getpostenv(sid, "SQLQUERY")!=NULL) {
			strncpy(queryname, "SQL Query", sizeof(queryname)-1);
			strncpy(query, getpostenv(sid, "SQLQUERY"), sizeof(query)-1);
		}
	} else if (getgetenv(sid, "QUERYID")!=NULL) {
		queryid=atoi(getgetenv(sid, "QUERYID"));
		if ((sqr=sqlQueryf("SELECT query, queryname FROM gw_queries WHERE queryid = %d", queryid))<0) return;
		if (sqlNumtuples(sqr)<1) {
			prints("<CENTER>No query found.</CENTER>\n");
			return;
		}
		strncpy(queryname, sqlGetvalue(sqr, 0, 1), sizeof(queryname)-1);
		strncpy(query, sqlGetvalue(sqr, 0, 0), sizeof(query)-1);
		sqlFreeconnect(sqr);
	} else {
		return;
	}
	strncpy(querytype, query, 6);
	pquerytype=querytype;
	while (*pquerytype) {
		if (*pquerytype==' ') {
			*pquerytype='\0';
			break;
		}
		*pquerytype=toupper(*pquerytype);
		pquerytype++;
	}
	if (strcmp(querytype, "SELECT")==0) {
		if ((sqr=sqlQuery(query))<0) {
			prints("<CENTER>SQL query failed.</CENTER>\n");
			return;
		}
	} else if (strcmp(querytype, "SHOW")==0) {
		if ((sqr=sqlQuery(query))<0) {
			prints("<CENTER>SQL query failed.</CENTER>\n");
			return;
		}
	} else if (strcmp(querytype, "DELETE")==0) {
		if (sqlUpdate(query)<0) {
			prints("<CENTER>SQL delete failed.</CENTER>\n");
		} else {
			prints("<CENTER>SQL delete successful.</CENTER>\n");
		}
		return;
	} else if (strcmp(querytype, "INSERT")==0) {
		if (sqlUpdate(query)<0) {
			prints("<CENTER>SQL insert failed.</CENTER>\n");
		} else {
			prints("<CENTER>SQL insert successful.</CENTER>\n");
		}
		return;
	} else if (strcmp(querytype, "UPDATE")==0) {
		if (sqlUpdate(query)<0) {
			prints("<CENTER>SQL update failed.</CENTER>\n");
		} else {
			prints("<CENTER>SQL update successful.</CENTER>\n");
		}
		return;
	} else {
		prints("<CENTER>Unknown SQL query type.</CENTER>\n");
		return;
	}
	if (sqlNumtuples(sqr)<1) {
		prints("<CENTER>Query [%s] returned no results.</CENTER>\n", queryname);
		sqlFreeconnect(sqr);
		return;
	}
	prints("<CENTER>\n[%s]<BR>\n", queryname);
	prints("<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0>\n");
	prints("<TR BGCOLOR=%s>", COLOR_TRIM);
	if (getgetenv(sid, "OFFSET")!=NULL) offset=atoi(getgetenv(sid, "OFFSET"));
	for (i=0;i<sqlNumfields(sqr);i++) {
		prints("<TH>%s</TH>", sqlGetfieldname(sqr, i));
	}
	prints("\n</TR>\n");
	for (i=offset;(i<sqlNumtuples(sqr))&&(i<offset+50);i++) {
		prints("<TR BGCOLOR=%s>", COLOR_FTEXT);
		for (j=0;j<sqlNumfields(sqr);j++)
			prints("<TD NOWRAP>%s&nbsp;</TD>", sqlGetvalue(sqr, i, j));
		prints("</TR>\n");
	}
	prints("</TABLE></CENTER>\n");
	prints("<CENTER><TABLE><TR>\n");
	if (offset>49) {
		prints("<TD><FORM METHOD=POST ACTION=%s/queryrun?queryid=%d&offset=%d>\n", conn[sid].dat->in_ScriptName, queryid, offset-50);
		prints("<INPUT TYPE=HIDDEN NAME=sqlquery VALUE='%s'>\n", query);
		prints("<INPUT TYPE=SUBMIT VALUE='Previous 50'>\n");
		prints("</FORM></TD>\n");
	}
	if (offset+50<sqlNumtuples(sqr)) {
		prints("<TD><FORM METHOD=POST ACTION=%s/queryrun?queryid=%d&offset=%d>\n", conn[sid].dat->in_ScriptName, queryid, offset+50);
		prints("<INPUT TYPE=HIDDEN NAME=sqlquery VALUE='%s'>\n", query);
		prints("<INPUT TYPE=SUBMIT VALUE='Next 50'>\n");
		prints("</FORM></TD>\n");
	}
	prints("</TR></TABLE>\n");
	prints("</CENTER>\n");
	sqlFreeconnect(sqr);
	return;
}

/****************************************************************************
 *	querysave()
 *
 *	Purpose	: Update query or call queryrun() to run query
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void querysave(int sid)
{
	char creator[50];
	char query[8192];
	char queryname[1024];
	int queryid=0;
	char submit[10];
	char timebuffer[100];
	time_t t;
	int sqr;

	if (auth(sid, "query")<4) {
		prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
		return;
	}
	if (strcmp(conn[sid].dat->in_RequestMethod,"POST")!=0) return;
	memset(submit, 0, sizeof(submit));
	if (getpostenv(sid, "SUBMIT")!=NULL) {
		strncpy(submit, getpostenv(sid, "SUBMIT"), sizeof(submit)-1);
	}
	if (strcmp("Run", submit)==0) {
		queryrun(sid);
		return;
	}
	snprintf(creator, sizeof(creator)-1, "%s", conn[sid].dat->in_username);
	if (getpostenv(sid, "QUERYID")!=NULL) {
		queryid=atoi(getpostenv(sid, "QUERYID"));
	}
	if (getpostenv(sid, "QUERYNAME")!=NULL) {
		strncpy(queryname, getpostenv(sid, "QUERYNAME"), sizeof(queryname)-1);
	} else {
		strncpy(queryname, "Unnamed Query", sizeof(queryname)-1);
	}
	if (getpostenv(sid, "SQLQUERY")==NULL) {
		prints("cannot save null queries");
		return;
	}
	strncpy(query, str2sql(getpostenv(sid, "SQLQUERY")), sizeof(query)-1);
	t=time((time_t*)0);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
	if (queryid==0) {
		if ((sqr=sqlQuery("SELECT max(queryid) FROM gw_queries"))<0) return;
		queryid=atoi(sqlGetvalue(sqr, 0, 0))+1;
		if (queryid<1) queryid=1;
		sqlFreeconnect(sqr);
		if (sqlUpdatef("INSERT INTO gw_queries (queryid, cdatetime, mdatetime, creator, queryname, query) values ('%d', '%s', '%s', '%s', '%s', '%s')", queryid, timebuffer, timebuffer, creator, queryname, query)<0) return;
		prints("<CENTER>Query %d added successfully</CENTER><BR>\n", queryid);
		logaccess(1, "%s - %s added query %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, queryid);
	} else if (strcmp(submit, "Delete")==0) {
		if (auth(sid, "query")<4) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("DELETE FROM gw_queries WHERE queryid = %d", queryid)<0) return;
		prints("<CENTER>Query %d deleted successfully</CENTER><BR>\n", queryid);
		logaccess(1, "%s - %s deleted query %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, queryid);
	} else {
		if (auth(sid, "query")<4) {
			prints("<BR><CENTER>Access Denied</CENTER><BR>\n");
			return;
		}
		if (sqlUpdatef("UPDATE gw_queries SET mdatetime = '%s', queryname = '%s', query = '%s' WHERE queryid = %d", timebuffer, queryname, query, queryid)<0) return;
		prints("<CENTER>Query %d modified successfully</CENTER><BR>\n", queryid);
		logaccess(1, "%s - %s modified query %d", conn[sid].dat->in_ClientIP, conn[sid].dat->in_username, queryid);
	}
	prints("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/queryrun\">\n", conn[sid].dat->in_ScriptName);
	return;
}

/****************************************************************************
 *	querymain()
 *
 *	Purpose	: Execute appropriate function based on URI
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void querymain(int sid)
{
	send_header(sid, 0, 200, "OK", "1", "text/html", -1, -1);
	printheader(sid, "Sentinel Groupware Queries");
	prints("<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints("function ShowHelp()\n");
	prints("{\n");
	prints("	window.open('/sentinel/help/ch10-00.html','helpwin','toolbar=no,location=no,directories=no,alwaysRaised=yes,top=0,left=0,status=no,menubar=no,scrollbars=yes,resizable=yes,width=600,height=450');\n");
	prints("}\n");
	prints("// -->\n</SCRIPT>\n");
	prints("<CENTER>\n");
	prints("<TABLE BORDER=1 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%><TR><TD>\n");
	prints("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%%>\n");
	prints("<TR VALIGN=middle BGCOLOR=%s><TD ALIGN=left NOWRAP>&nbsp;\n", COLOR_TMENU);
	prints("<a class='TBAR' href=%s/queryrun>VIEW QUERIES</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<a class='TBAR' href=%s/queryadd>ADD QUERY</a>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=javascript:window.print()>PRINT</A>\n");
	prints("&nbsp;</TD><TD ALIGN=right NOWRAP>&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=javascript:ShowHelp()>HELP</A>&nbsp;&middot;&nbsp;\n");
	prints("<A CLASS='TBAR' HREF=%s/profileview TARGET=main>PROFILE</A>&nbsp;&middot;&nbsp;\n", conn[sid].dat->in_ScriptName);
	prints("<A CLASS='TBAR' HREF=%s/logout TARGET=_top>LOG OUT</A>\n", conn[sid].dat->in_ScriptName);
	prints("&nbsp;</TD></TR></TABLE>\n");
	prints("</TD></TR></TABLE>\n");
//	if (verifyimage("logo.gif")) {
//		prints("<BR><IMG SRC=/sentinel/images/logo.gif BORDER=0>\n");
//	}
	prints("</CENTER><BR>\n");
	flushbuffer(sid);
	if (strncmp(conn[sid].dat->in_RequestURI, "/queryadd", 9)==0)
		queryadd(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/queryrun", 9)==0)
		queryrun(sid);
	else if (strncmp(conn[sid].dat->in_RequestURI, "/querysave", 10)==0)
		querysave(sid);
	printfooter(sid);
	return;
}
