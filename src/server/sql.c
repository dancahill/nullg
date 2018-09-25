/*
    NullLogic GroupServer - Copyright (C) 2000-2015 Dan Cahill

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

void sql_disconnect(nsp_state *N)
{
	pthread_mutex_lock(&Lock.SQL);
	_sql_disconnect(proc.N);
	pthread_mutex_unlock(&Lock.SQL);
	return;
}

void sql_freeresult(nsp_state *N, obj_t **qobj)
{
	_sql_freeresult(N, qobj);
	return;
}

int sql_update(nsp_state *N, obj_t **qobj, char *sqlquery)
{
	int rc = -1;

	pthread_mutex_lock(&Lock.SQL);
	rc = _sql_update(N, qobj, sqlquery);
	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_query(nsp_state *N, obj_t **qobj, char *query)
{
	int rc = -1;

	pthread_mutex_lock(&Lock.SQL);
	rc = _sql_query(N, qobj, query);
	pthread_mutex_unlock(&Lock.SQL);
	return rc;
}

int sql_updatef(nsp_state *N, obj_t **qobj, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery = calloc(8192, sizeof(char))) == NULL) {
		log_error(proc.N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc = sql_update(N, qobj, sqlquery);
	free(sqlquery);
	return rc;
}

int sql_queryf(nsp_state *N, obj_t **qobj, char *format, ...)
{
	char *sqlquery;
	va_list ap;
	int rc;

	if ((sqlquery = calloc(8192, sizeof(char))) == NULL) {
		log_error(proc.N, "sql", __FILE__, __LINE__, 0, "OUT OF MEMORY");
		return -1;
	}
	va_start(ap, format);
	vsnprintf(sqlquery, 8191, format, ap);
	va_end(ap);
	rc = sql_query(N, qobj, sqlquery);
	free(sqlquery);
	return rc;
}


int sql_getsequence(nsp_state *N, char *table)
{
	int rc = _sql_getsequence(N, table);
	return rc;
}
