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

static FILE *fp;

int sqlfprintf(FILE *fp, const char *format, ...)
{
	unsigned char buffer[1024];
	int offset=0;
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	while (buffer[offset]) {
		if (buffer[offset]==13) {
//			fprintf(fp, "\\r");
		} else if (buffer[offset]==10) {
			fprintf(fp, "\\n");
		} else if (buffer[offset]=='\'') {
			fprintf(fp, "''");
		} else {
			fprintf(fp, "%c", buffer[offset]);
		}
		offset++;
	}
	return 0;
}

int dump_table(char *table)
{
	char query[100];
	int i;
	int j;
	int sqr;

	snprintf(query, sizeof(query)-1, "SELECT * from %s", table);
	if ((sqr=sqlQuery(query))<0) return -1;
	for (i=0;i<sqlNumtuples(sqr);i++) {
		fprintf(fp, "INSERT INTO %s (", table);
		for (j=0;j<sqlNumfields(sqr);j++) {
			fprintf(fp, "%s", sqlGetfieldname(sqr, j));
			if (j<sqlNumfields(sqr)-1) {
				fprintf(fp, ", ");
			}
		}
		fprintf(fp, ") VALUES (");
		for (j=0;j<sqlNumfields(sqr);j++) {
			fprintf(fp, "'");
			sqlfprintf(fp, "%s", sqlGetvalue(sqr, i, j));
			fprintf(fp, "'");
			if (j<sqlNumfields(sqr)-1) {
				fprintf(fp, ", ");
			}
		}
		fprintf(fp, ");\n");
	}
	sqlFreeconnect(sqr);
	return 0;
}

int dump_db(char *filename)
{
	printf("Dumping Sentinel Groupware database to %s...", filename);
	configread();
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	if (dump_table("gw_dbinfo")<0) return -1;
//	if (dump_table("gw_calls")<0) return -1;
	if (dump_table("gw_companies")<0) return -1;
	if (dump_table("gw_contacts")<0) return -1;
	if (dump_table("gw_events")<0) return -1;
	if (dump_table("gw_forums")<0) return -1;
	if (dump_table("gw_forumposts")<0) return -1;
	if (dump_table("gw_links")<0) return -1;
	if (dump_table("gw_messages")<0) return -1;
	if (dump_table("gw_orders")<0) return -1;
	if (dump_table("gw_orderdetails")<0) return -1;
	if (dump_table("gw_products")<0) return -1;
	if (dump_table("gw_queries")<0) return -1;
	if (dump_table("gw_tasks")<0) return -1;
	if (dump_table("gw_users")<0) return -1;
	printf("done.\r\n");
	fclose(fp);
	sqlDisconnect();
	return -1;
}
