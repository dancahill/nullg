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
#include "http_mod.h"
#include "mod_searches.h"

/****************************************************************************
 *	queryadd()
 *
 *	Purpose	: Display form for adding or editing queries
 *	Args	: None
 *	Returns	: void
 *	Notes	: None
 ***************************************************************************/
void searchsqladd(CONN *sid)
{
	REC_QUERY query;
	char *ptemp;
	int queryid;

	if (!(auth_priv(sid, "query")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestURI, "/search/sqladd")==0) {
		queryid=0;
		dbread_query(sid, 2, 0, &query);
	} else {
		if ((ptemp=getgetenv(sid, "QUERYID"))==NULL) return;
		queryid=atoi(ptemp);
		if (dbread_query(sid, 2, queryid, &query)!=0) {
			prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", queryid);
			return;
		}
	}
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\n");
	prints(sid, "function ConfirmDelete() {\n");
	prints(sid, "	return confirm(\"Are you sure you want to delete this record?\");\n");
	prints(sid, "}\n");
	prints(sid, "// -->\n</SCRIPT>\n");
	prints(sid, "<CENTER>\n<FORM METHOD=POST ACTION=%s/search/sqlsave NAME=queryrun>\n", sid->dat->in_ScriptName);
	prints(sid, "<INPUT TYPE=HIDDEN NAME=queryid VALUE=%d>\n", queryid);
	prints(sid, "<TABLE BORDER=0 CELLPADDING=2 CELLSPACING=0>\n");
	if (queryid!=0) {
		prints(sid, "<TR><TH COLSPAN=2>Query Number %d</TH></TR>\n", query.queryid);
	} else {
		prints(sid, "<TR><TH COLSPAN=2>New Query</TH></TR>\n");
	}
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>&nbsp;Query Name&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><INPUT TYPE=TEXT NAME=queryname VALUE=\"%s\" SIZE=60></TD></TR>\n", str2html(sid, query.queryname));
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><B>&nbsp;Query&nbsp;</B></TD></TR>\n");
	prints(sid, "<TR CLASS=\"EDITFORM\"><TD COLSPAN=2><TEXTAREA NAME=query ROWS=5 COLS=60>%s</TEXTAREA></TD></TR>\n", query.query);
	if ((query.obj_uid==sid->dat->user_uid)||(auth_priv(sid, "query")&A_ADMIN)) {
		prints(sid, "<TR><TH ALIGN=center COLSPAN=2>Permissions</TH></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Owner&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_uid style='width:182px'%s>\n", (auth_priv(sid, "query")&A_ADMIN)?"":" DISABLED");
		htselect_user(sid, query.obj_uid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group&nbsp;</B></TD>");
		prints(sid, "<TD ALIGN=RIGHT STYLE='padding:0px'><SELECT NAME=obj_gid style='width:182px'%s>\n", (auth_priv(sid, "query")&A_ADMIN)?"":" DISABLED");
		htselect_group(sid, query.obj_gid);
		prints(sid, "</SELECT></TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Group Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"0\"%s>None\n", query.obj_gperm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"1\"%s>Read\n", query.obj_gperm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_gperm VALUE=\"2\"%s>Write\n", query.obj_gperm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
		prints(sid, "<TR CLASS=\"EDITFORM\"><TD STYLE='padding:0px'><B>&nbsp;Other Members&nbsp;</B></TD><TD ALIGN=RIGHT STYLE='padding:0px'>\n");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"0\"%s>None\n", query.obj_operm==0?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"1\"%s>Read\n", query.obj_operm==1?" CHECKED":"");
		prints(sid, "<INPUT TYPE=RADIO NAME=obj_operm VALUE=\"2\"%s>Write\n", query.obj_operm==2?" CHECKED":"");
		prints(sid, "</TD></TR>\n");
	}
	prints(sid, "</TABLE>\n");
	prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=submit VALUE='Save'>\n");
	if (queryid!=0) {
		prints(sid, "<INPUT TYPE=SUBMIT CLASS=frmButton NAME=Submit VALUE='Delete' onClick=\"return ConfirmDelete();\">\n");
	}
	prints(sid, "</FORM>\n</CENTER>\n");
	prints(sid, "<SCRIPT LANGUAGE=JavaScript>\n<!--\ndocument.queryrun.queryname.focus();\n// -->\n</SCRIPT>\n");
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
void searchsqlrun(CONN *sid)
{
	REC_QUERY query;
	char querytype[10];
	char *ptemp;
	int queryid;
	int offset;
	int i, j;
	int sqr;
	int min, max;

	if (!(auth_priv(sid, "query")&A_READ)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if ((ptemp=getgetenv(sid, "QUERYID"))!=NULL) {
		queryid=atoi(ptemp);
	} else {
		queryid=0;
	}
	if (queryid==0) {
		if (auth_priv(sid, "query")&A_ADMIN) {
			if ((sqr=sql_queryf("SELECT queryid, queryname, query FROM gw_queries ORDER BY queryname ASC"))<0) return;
		} else {
			if ((sqr=sql_queryf("SELECT queryid, queryname, query FROM gw_queries where (obj_uid = %d or (obj_gid = %d and obj_gperm>=1) or obj_operm>=1) ORDER BY queryname ASC", sid->dat->user_uid, sid->dat->user_gid))<0) return;
		}
		prints(sid, "<CENTER>\n");
		if (sql_numtuples(sqr)>0) {
			prints(sid, "Saved queries<BR>\n");
			prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n<TR>\n");
			if (auth_priv(sid, "query")&A_ADMIN) {
				prints(sid, "<TH ALIGN=LEFT STYLE='border-style:solid'>&nbsp;</TH>");
			}
			prints(sid, "<TH ALIGN=LEFT COLSPAN=2 WIDTH=200 NOWRAP STYLE='border-style:solid'>Query Name</TH></TR>\n");
			for (i=0;i<sql_numtuples(sqr);i++) {
				prints(sid, "<TR CLASS=\"FIELDVAL\">");
				if (auth_priv(sid, "query")&A_ADMIN) {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/search/sqladd?queryid=%s>edit</A></TD>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
				}
				prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/search/sqlrun?queryid=%s>", sid->dat->in_ScriptName, sql_getvalue(sqr, i, 0));
				prints(sid, "%s</A>&nbsp;</TD>\n", str2html(sid, sql_getvalue(sqr, i, 1)));

				if (strncasecmp("SELECT", sql_getvalue(sqr, i, 2), 6)==0) {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'><A HREF=%s/search/sqlrun?queryid=%d&format=csv>CSV</A></TD>\n", sid->dat->in_ScriptName, atoi(sql_getvalue(sqr, i, 0)));
				} else {
					prints(sid, "<TD NOWRAP STYLE='border-style:solid'>&nbsp;</TD>\n");
				}
				prints(sid, "</TR>\n");
			}
			prints(sid, "</TABLE>\n");
		} else {
			prints(sid, "There are no saved queries<BR>\n");
		}
		sql_freeresult(sqr);
		if (auth_priv(sid, "query")&A_ADMIN) {
			prints(sid, "<A HREF=%s/search/sqladd>New Query</A>\n", sid->dat->in_ScriptName);
		}
		prints(sid, "</CENTER>\n");
		return;
	}
	if (dbread_query(sid, 1, queryid, &query)!=0) {
		prints(sid, "<CENTER>No matching record found for %d</CENTER>\n", queryid);
		return;
	}
	memset(querytype, 0, sizeof(querytype));
	strncpy(querytype, query.query, 6);
	ptemp=querytype;
	while (*ptemp) {
		if (*ptemp==' ') {
			*ptemp='\0';
			break;
		}
		*ptemp=toupper(*ptemp);
		ptemp++;
	}
	if (strcmp(querytype, "SELECT")==0) {
		if ((sqr=sql_query(query.query))<0) {
			prints(sid, "<CENTER>SQL query failed.</CENTER>\n");
			return;
		}
	} else if (strcmp(querytype, "SHOW")==0) {
		if ((sqr=sql_query(query.query))<0) {
			prints(sid, "<CENTER>SQL query failed.</CENTER>\n");
			return;
		}
	} else if (strcmp(querytype, "DELETE")==0) {
		if (sql_update(query.query)<0) {
			prints(sid, "<CENTER>SQL delete failed.</CENTER>\n");
		} else {
			prints(sid, "<CENTER>SQL delete successful.</CENTER>\n");
		}
		return;
	} else if (strcmp(querytype, "INSERT")==0) {
		if (sql_update(query.query)<0) {
			prints(sid, "<CENTER>SQL insert failed.</CENTER>\n");
		} else {
			prints(sid, "<CENTER>SQL insert successful.</CENTER>\n");
		}
		return;
	} else if (strcmp(querytype, "UPDATE")==0) {
		if (sql_update(query.query)<0) {
			prints(sid, "<CENTER>SQL update failed.</CENTER>\n");
		} else {
			prints(sid, "<CENTER>SQL update successful.</CENTER>\n");
		}
		return;
	} else {
		prints(sid, "<CENTER>Unknown SQL query type.</CENTER>\n");
		return;
	}

	if ((ptemp=getgetenv(sid, "FORMAT"))!=NULL) {
		snprintf(sid->dat->out_ContentDisposition, sizeof(sid->dat->out_ContentDisposition)-1, "attachment; filename=query.csv");
		send_header(sid, 1, 200, "OK", "1", "application/octet-stream", -1, -1);
		flushbuffer(sid);
		for (i=0;i<sql_numfields(sqr);i++) {
			prints(sid, "\"%s\"", sql_getname(sqr, i));
			if (i<sql_numfields(sqr)-1) {
				prints(sid, ",");
			} else {
				prints(sid, "\r\n");
			}
		}
		for (i=0;i<sql_numtuples(sqr);i++) {
			for (j=0;j<sql_numfields(sqr);j++) {
				prints(sid, "\"%s\"", str2html(sid, sql_getvalue(sqr, i, j)));
				if (j<sql_numfields(sqr)-1) {
					prints(sid, ",");
				} else {
					prints(sid, "\r\n");
				}
			}
		}
		sql_freeresult(sqr);
		return;
	}

	if (sql_numtuples(sqr)<1) {
		prints(sid, "<CENTER>Query [%s] returned no results.</CENTER>\n", query.queryname);
		sql_freeresult(sqr);
		return;
	}
	prints(sid, "<CENTER>\n%s - %d results<BR>\n", query.queryname, sql_numtuples(sqr));
	prints(sid, "<TABLE BORDER=1 CELLPADDING=2 CELLSPACING=0 STYLE='border-style:solid'>\r\n");
	prints(sid, "<TR>");
	if ((ptemp=getgetenv(sid, "OFFSET"))!=NULL) {
		offset=atoi(ptemp);
	} else {
		offset=0;
	}
	for (i=0;i<sql_numfields(sqr);i++) {
		prints(sid, "<TH ALIGN=LEFT NOWRAP STYLE='border-style:solid'>%s</TH>", str2html(sid, sql_getname(sqr, i)));
	}
	prints(sid, "</TR>\n");
	for (i=offset;(i<sql_numtuples(sqr))&&(i<offset+sid->dat->user_maxlist);i++) {
		prints(sid, "<TR CLASS=\"FIELDVAL\">");
		for (j=0;j<sql_numfields(sqr);j++)
			prints(sid, "<TD NOWRAP STYLE='border-style:solid'>%s&nbsp;</TD>", str2html(sid, sql_getvalue(sqr, i, j)));
		prints(sid, "</TR>\n");
	}
	prints(sid, "</TABLE></CENTER>\n");
	prints(sid, "<CENTER>\n");
	if (sql_numtuples(sqr)>sid->dat->user_maxlist) {
		if (offset>(sid->dat->user_maxlist-1)) {
			prints(sid, "[<A HREF=%s/search/sqlrun?queryid=%d&offset=%d>", sid->dat->in_ScriptName, queryid, offset-sid->dat->user_maxlist);
			min=offset-sid->dat->user_maxlist+1;
			max=offset;
			if (min<0) min=0;
			if (min>sql_numtuples(sqr)) min=sql_numtuples(sqr);
			if (max<0) max=0;
			if (max>sql_numtuples(sqr)) max=sql_numtuples(sqr);
			prints(sid, "Previous (%d to %d)</A>]\n", min, max);
		} else {
			prints(sid, "[Previous]\n");
		}
		if (offset+sid->dat->user_maxlist<sql_numtuples(sqr)) {
			prints(sid, "[<A HREF=%s/search/sqlrun?queryid=%d&offset=%d>", sid->dat->in_ScriptName, queryid, offset+sid->dat->user_maxlist);
			min=offset+sid->dat->user_maxlist+1;
			max=offset+sid->dat->user_maxlist+sid->dat->user_maxlist;
			if (min<0) min=0;
			if (min>sql_numtuples(sqr)) min=sql_numtuples(sqr);
			if (max<0) max=0;
			if (max>sql_numtuples(sqr)) max=sql_numtuples(sqr);
			prints(sid, "Next (%d to %d)</A>]\n", min, max);
		} else {
			prints(sid, "[Next]\n");
		}
	}
	prints(sid, "</CENTER>\n");
	sql_freeresult(sqr);
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
void searchsqlsave(CONN *sid)
{
	REC_QUERY query;
	char *ptemp;
	int queryid;

	if (!(auth_priv(sid, "query")&A_ADMIN)) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (strcmp(sid->dat->in_RequestMethod,"POST")!=0) return;
	if ((ptemp=getpostenv(sid, "QUERYID"))==NULL) return;
	queryid=atoi(ptemp);
	if (dbread_query(sid, 2, queryid, &query)!=0) {
		prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
		return;
	}
	if (auth_priv(sid, "query")&A_ADMIN) {
		if ((ptemp=getpostenv(sid, "OBJ_UID"))!=NULL) query.obj_uid=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_GID"))!=NULL) query.obj_gid=atoi(ptemp);
	}
	if ((auth_priv(sid, "query")&A_ADMIN)||(query.obj_uid==sid->dat->user_uid)) {
		if ((ptemp=getpostenv(sid, "OBJ_GPERM"))!=NULL) query.obj_gperm=atoi(ptemp);
		if ((ptemp=getpostenv(sid, "OBJ_OPERM"))!=NULL) query.obj_operm=atoi(ptemp);
	}
	if ((ptemp=getpostenv(sid, "QUERYNAME"))!=NULL) snprintf(query.queryname, sizeof(query.queryname)-1, "%s", ptemp);
	if ((ptemp=getpostenv(sid, "QUERY"))!=NULL) snprintf(query.query, sizeof(query.query)-1, "%s", ptemp);
	if (strlen(query.queryname)==0) strncpy(query.queryname, "Unnamed Query", sizeof(query.queryname)-1);
	if (((ptemp=getpostenv(sid, "SUBMIT"))!=NULL)&&(strcmp(ptemp, "Delete")==0)) {
		if (!(auth_priv(sid, "query")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (sql_updatef("DELETE FROM gw_queries WHERE queryid = %d", queryid)<0) return;
		prints(sid, "<CENTER>Query %d deleted successfully</CENTER><BR>\n", queryid);
		db_log_activity(sid, 1, "queries", query.queryid, "delete", "%s - %s deleted query %d", sid->dat->in_RemoteAddr, sid->dat->user_username, query.queryid);
	} else if (query.queryid==0) {
		if (!(auth_priv(sid, "query")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if ((query.queryid=dbwrite_query(sid, 0, &query))<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Query %d added successfully</CENTER><BR>\n", query.queryid);
		db_log_activity(sid, 1, "queries", query.queryid, "insert", "%s - %s added query %d", sid->dat->in_RemoteAddr, sid->dat->user_username, query.queryid);
	} else {
		if (!(auth_priv(sid, "query")&A_ADMIN)) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		if (dbwrite_query(sid, queryid, &query)<1) {
			prints(sid, "<BR><CENTER>%s</CENTER><BR>\n", ERR_NOACCESS);
			return;
		}
		prints(sid, "<CENTER>Query %d modified successfully</CENTER><BR>\n", query.queryid);
		db_log_activity(sid, 1, "queries", query.queryid, "modify", "%s - %s modified query %d", sid->dat->in_RemoteAddr, sid->dat->user_username, query.queryid);
	}
	prints(sid, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL=%s/search/sqlrun\">\n", sid->dat->in_ScriptName);
	return;
}
