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
#include "../xmlsync/xmlsync.h"
#ifdef WIN32
#pragma comment(lib, "../bin/xmlsync.lib")
#endif

#define NUMFIELDS 40

static char *field_mappings[][2]={
	{ "contactid",      ""                     },
	{ "obj_ctime",      ""                     },
	{ "obj_mtime",      ""                     },
	{ "obj_uid",        ""                     },
	{ "obj_gid",        ""                     },
	{ "obj_did",        ""                     },
	{ "obj_gperm",      ""                     },
	{ "obj_operm",      ""                     },
	{ "loginip",        ""                     },
	{ "logintime",      ""                     },
	{ "logintoken",     ""                     },
	{ "username",       "Nickname"             },
	{ "password",       ""                     },
	{ "enabled",        ""                     },
	{ "geozone",        ""                     },
	{ "timezone",       ""                     },
	{ "surname",        "Last Name"            },
	{ "givenname",      "First Name"           },
	{ "salutation",     "Title"                },
	{ "contacttype",    ""                     },
	{ "referredby",     "Referred By"          },
	{ "altcontact",     ""                     },
	{ "prefbilling",    ""                     },
	{ "email",          "E-mail Address"       },
	{ "homenumber",     "Home Phone"           },
	{ "worknumber",     "Business Phone"       },
	{ "faxnumber",      "Business Fax"         },
	{ "mobilenumber",   "Mobile Phone"         },
	{ "jobtitle",       "Job Title"            },
	{ "organization",   "Company"              },
	{ "homeaddress",    "Home Street"          },
	{ "homelocality",   "Home City"            },
	{ "homeregion",     "Home State"           },
	{ "homecountry",    "Home Country"         },
	{ "homepostalcode", "Home Postal Code"     },
	{ "workaddress",    "Business Street"      },
	{ "worklocality",   "Business City"        },
	{ "workregion",     "Business State"       },
	{ "workcountry",    "Business Country"     },
	{ "workpostalcode", "Business Postal Code" },
	{ NULL, NULL }
};
CONFIG config;
RECLIST_CONTACT contacts;

char *field_defined(char *name)
{
	int i;

	i=0;
	while (field_mappings[i][0]!=NULL) {
		if (strcasecmp(name, field_mappings[i][0])==0) {
			if (strlen(field_mappings[i][1])>0) return field_mappings[i][1];
		}
		i++;
	}
	return NULL;
}

char *r_field_defined(char *name)
{
	int i;

	i=0;
	while (field_mappings[i][1]!=NULL) {
		if (strcasecmp(name, field_mappings[i][1])==0) {
			if (strlen(field_mappings[i][0])>0) return field_mappings[i][0];
		}
		i++;
	}
	return NULL;
}

void usage(char *arg0)
{
	char *progname;

	printf("\r\nNullLogic Groupware CSV Sync [1.3.17]\r\n");
#ifdef WIN32
	progname=strrchr(arg0, '\\');
#else
	progname=strrchr(arg0, '/');
#endif
	if (progname==NULL) {
		progname=arg0;
	} else {
		progname++;
	}
	printf("Usage: %s command parameter\r\n\r\n", progname);
	printf("  command  Must be one of the following\r\n\r\n");
	printf("    DUMP     Dumps the data from the current database to a file\r\n");
	printf("    RESTORE  Restores the current database from a file\r\n");
	printf("\r\nThe DUMP command is used to export the contents of the current database\r\n");
	printf("to a CSV text file.  The parameter for this function is the name of the file\r\n");
	printf("to which the data is to be saved.\r\n");
	printf("\r\nThe RESTORE command is used to restore a CSV dump to the current\r\n");
	printf("database.  The parameter for this function is the name of the file\r\n");
	printf("from which the data is to be restored.\r\n");
	return;
}

int config_read(CONFIG *config)
{
	FILE *fp=NULL;
	char line[512];
	char *pVar;
	char *pVal;
	int i;

	snprintf(config->host,     sizeof(config->host)-1,     "localhost");
	config->port=4110;
	snprintf(config->uri,      sizeof(config->uri)-1,      "/xml-rpc/");
	snprintf(config->username, sizeof(config->username)-1, "administrator");
	snprintf(config->password, sizeof(config->password)-1, "visual");
	fp=fopen("xmlsync.cfg", "r");
	if (fp==NULL) {
		fp=fopen("xmlsync.cfg", "w");
		if (fp==NULL) return -1;
		fprintf(fp, "HOST     = \"%s\"\n", config->host);
		fprintf(fp, "PORT     = \"%d\"\n", config->port);
		fprintf(fp, "URI      = \"%s\"\n", config->uri);
		fprintf(fp, "USERNAME = \"%s\"\n", config->username);
		fprintf(fp, "PASSWORD = \"%s\"\n", config->password);
		fclose(fp);
		printf("No configuration file found.  Creating...\r\nPlease edit xmlsync.cfg\r\n");
		exit(0);
	}
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
		if (isalpha(line[0])) {
			pVar=line;
			pVal=line;
			while ((*pVal!='=')&&((char *)&pVal+1!='\0')) pVal++;
			*pVal='\0';
			pVal++;
			while (*pVar==' ') pVar++;
			while (pVar[strlen(pVar)-1]==' ') pVar[strlen(pVar)-1]='\0';
			while (*pVal==' ') pVal++;
			while (pVal[strlen(pVal)-1]==' ') pVal[strlen(pVal)-1]='\0';
			while (*pVal=='"') pVal++;
			while (pVal[strlen(pVal)-1]=='"') pVal[strlen(pVal)-1]='\0';
			if (strcmp(pVar, "HOST")==0) {
				snprintf(config->host,     sizeof(config->host)-1,     "%s", pVal);
			} else if (strcmp(pVar, "PORT")==0) {
				config->port=atoi(pVal);
			} else if (strcmp(pVar, "URI")==0) {
				snprintf(config->uri,      sizeof(config->uri)-1,      "%s", pVal);
			} else if (strcmp(pVar, "USERNAME")==0) {
				snprintf(config->username, sizeof(config->username)-1, "%s", pVal);
			} else if (strcmp(pVar, "PASSWORD")==0) {
				snprintf(config->password, sizeof(config->password)-1, "%s", pVal);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
	return 0;
}

int cond_writeval(FILE *fp, char *fieldname, char *fieldval, int colcount)
{
	if (field_defined(fieldname)!=NULL) {
		if (colcount>0) fprintf(fp, ",");
		fprintf(fp, "\"%s\"", fieldval);
		return 1;
	}
	return 0;
}

int dump_db(char *filename)
{
	FILE *fp;
	unsigned int i;
	short int colcount=0;

	printf("Dumping data to %s...\r\n", filename);
	fp=fopen(filename, "wa");
	if (fp==NULL) {
		printf("\r\nCould not create output file.\r\n");
		return -1;
	}
	for (i=0;i<NUMFIELDS;i++) {
		if (strlen(field_mappings[i][1])>0) {
			if (colcount>0) fprintf(fp, ",");
			fprintf(fp, "\"%s\"", field_mappings[i][1]);
			colcount++;
		}
	}
	fprintf(fp, "\n");
	for (i=0;i<contacts.records;i++) {
		colcount=0;
		printf(" - %s, %s <%s>\r\n", contacts.contact[i]->surname, contacts.contact[i]->givenname, contacts.contact[i]->email);
		if (cond_writeval(fp, "username",       contacts.contact[i]->username,       colcount)>0) colcount++;
		if (cond_writeval(fp, "surname",        contacts.contact[i]->surname,        colcount)>0) colcount++;
		if (cond_writeval(fp, "givenname",      contacts.contact[i]->givenname,      colcount)>0) colcount++;
		if (cond_writeval(fp, "salutation",     contacts.contact[i]->salutation,     colcount)>0) colcount++;
		if (cond_writeval(fp, "referredby",     contacts.contact[i]->referredby,     colcount)>0) colcount++;
		if (cond_writeval(fp, "email",          contacts.contact[i]->email,          colcount)>0) colcount++;
		if (cond_writeval(fp, "homenumber",     contacts.contact[i]->homenumber,     colcount)>0) colcount++;
		if (cond_writeval(fp, "worknumber",     contacts.contact[i]->worknumber,     colcount)>0) colcount++;
		if (cond_writeval(fp, "faxnumber",      contacts.contact[i]->faxnumber,      colcount)>0) colcount++;
		if (cond_writeval(fp, "mobilenumber",   contacts.contact[i]->mobilenumber,   colcount)>0) colcount++;
		if (cond_writeval(fp, "jobtitle",       contacts.contact[i]->jobtitle,       colcount)>0) colcount++;
		if (cond_writeval(fp, "organization",   contacts.contact[i]->organization,   colcount)>0) colcount++;
		if (cond_writeval(fp, "homeaddress",    contacts.contact[i]->homeaddress,    colcount)>0) colcount++;
		if (cond_writeval(fp, "homelocality",   contacts.contact[i]->homelocality,   colcount)>0) colcount++;
		if (cond_writeval(fp, "homeregion",     contacts.contact[i]->homeregion,     colcount)>0) colcount++;
		if (cond_writeval(fp, "homecountry",    contacts.contact[i]->homecountry,    colcount)>0) colcount++;
		if (cond_writeval(fp, "homepostalcode", contacts.contact[i]->homepostalcode, colcount)>0) colcount++;
		if (cond_writeval(fp, "workaddress",    contacts.contact[i]->workaddress,    colcount)>0) colcount++;
		if (cond_writeval(fp, "worklocality",   contacts.contact[i]->worklocality,   colcount)>0) colcount++;
		if (cond_writeval(fp, "workregion",     contacts.contact[i]->workregion,     colcount)>0) colcount++;
		if (cond_writeval(fp, "workcountry",    contacts.contact[i]->workcountry,    colcount)>0) colcount++;
		if (cond_writeval(fp, "workpostalcode", contacts.contact[i]->workpostalcode, colcount)>0) colcount++;
		fprintf(fp, "\n");
	}
	printf("%d contacts\r\n", contacts.records);
	fclose(fp);
	return 0;
}

char *getfieldname(char *fields, int fieldnumber, int maxfields)
{
	char *column=NULL;
	int i;

	if ((fieldnumber<0)||(fieldnumber+1>maxfields)) return NULL;
	column=fields;
	for (i=0;i<fieldnumber;i++) {
		column+=strlen(column)+1;
	}
	return column;
}

int restore_db(char *filename)
{
	FILE *fp;
	char line[12288];
	char fields[2048];
	char tmpval[128];
	short int fieldcount;
	short int col;
	char *column;
	char *ptemp;
	char *col_rmap;
	char timebuf[40];
	REC_CONTACT contact;
	REC_CONTACT contact_new;
	unsigned int i;
	short int found;
	short int recs_new=0;
	short int recs_upd=0;
	short int recs_out=0;
	time_t t=time(NULL);

	strftime(timebuf, 30, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	printf("Restoring data from %s...\r\n", filename);
	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("Could not open source file.\r\n");
		return -1;
	}
	memset(fields, 0, sizeof(fields));
	fieldcount=0;
	if (fgets(line, sizeof(line)-1, fp)==NULL) return -1;
	column=fields;
	ptemp=line;
	while ((*ptemp)&&(*ptemp!='\r')&&(*ptemp!='\n')) {
		if (*ptemp==',') {
			fieldcount++;
			column++;
			ptemp++;
		} else if (*ptemp=='"') {
			ptemp++;
		} else {
			*column++=*ptemp++;
		}
	}
	memset((char *)&contact_new, 0, sizeof(REC_CONTACT));
	if (xmlrpc_contact_read(config.host, config.port, config.uri, config.username, config.password, 0, &contact_new)<0) {
		printf("Error connecting to server.\r\n");
		return -1;
	}
	while (1) {
		memset((char *)&contact, 0, sizeof(REC_CONTACT));
		if (fgets(line, sizeof(line)-1, fp)==NULL) break;
		while ((line[strlen(line)-1]=='\n')||(line[strlen(line)-1]=='\r')) {
			line[strlen(line)-1]='\0';
		}
		col=0;
		ptemp=line;
		memset(tmpval, 0, sizeof(tmpval));
		column=tmpval;
		while (*ptemp) {
			if (*ptemp==',') {
				if ((col_rmap=r_field_defined(getfieldname(fields, col, fieldcount)))!=NULL) {
					if (strcmp(col_rmap, "username")==0) {
						snprintf(contact.username, sizeof(contact.username)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "surname")==0) {
						snprintf(contact.surname, sizeof(contact.surname)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "givenname")==0) {
						snprintf(contact.givenname, sizeof(contact.givenname)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "salutation")==0) {
						snprintf(contact.salutation, sizeof(contact.salutation)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "referredby")==0) {
						snprintf(contact.referredby, sizeof(contact.referredby)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "email")==0) {
						snprintf(contact.email, sizeof(contact.email)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "homenumber")==0) {
						snprintf(contact.homenumber, sizeof(contact.homenumber)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "worknumber")==0) {
						snprintf(contact.worknumber, sizeof(contact.worknumber)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "faxnumber")==0) {
						snprintf(contact.faxnumber, sizeof(contact.faxnumber)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "mobilenumber")==0) {
						snprintf(contact.mobilenumber, sizeof(contact.mobilenumber)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "jobtitle")==0) {
						snprintf(contact.jobtitle, sizeof(contact.jobtitle)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "organization")==0) {
						snprintf(contact.organization, sizeof(contact.organization)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "homeaddress")==0) {
						snprintf(contact.homeaddress, sizeof(contact.homeaddress)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "homelocality")==0) {
						snprintf(contact.homelocality, sizeof(contact.homelocality)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "homeregion")==0) {
						snprintf(contact.homeregion, sizeof(contact.homeregion)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "homecountry")==0) {
						snprintf(contact.homecountry, sizeof(contact.homecountry)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "homepostalcode")==0) {
						snprintf(contact.homepostalcode, sizeof(contact.homepostalcode)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "workaddress")==0) {
						snprintf(contact.workaddress, sizeof(contact.workaddress)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "worklocality")==0) {
						snprintf(contact.worklocality, sizeof(contact.worklocality)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "workregion")==0) {
						snprintf(contact.workregion, sizeof(contact.workregion)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "workcountry")==0) {
						snprintf(contact.workcountry, sizeof(contact.workcountry)-1, "%s", tmpval);
					} else if (strcmp(col_rmap, "workpostalcode")==0) {
						snprintf(contact.workpostalcode, sizeof(contact.workpostalcode)-1, "%s", tmpval);
					}
				}
				memset(tmpval, 0, sizeof(tmpval));
				column=tmpval;
				col++;
				ptemp++;
			} else if (*ptemp=='"') {
				ptemp++;
			} else {
				*column++=*ptemp++;
			}
		}
		found=-1;
		for (i=0;i<contacts.records;i++) {
			if (!strlen(contact.surname)||!strlen(contact.givenname)) break;
			if ((strcasecmp(contact.surname, contacts.contact[i]->surname)!=0)||(strcasecmp(contact.givenname, contacts.contact[i]->givenname)!=0)) continue;
			contacts.contact[i]->contactid=-1;
			found=i;
		}
		if (found==-1) {
			printf("O %s, %s <%s>\r\n", contact.surname, contact.givenname, contact.email);
			contact.contactid=0;
			contact.obj_ctime=contact_new.obj_ctime;
			contact.obj_mtime=contact_new.obj_mtime;
			contact.obj_uid=contact_new.obj_uid;
			contact.obj_gid=contact_new.obj_gid;
			contact.obj_did=contact_new.obj_did;
			contact.obj_gperm=contact_new.obj_gperm;
			contact.obj_operm=contact_new.obj_operm;
			contact.enabled=0;
			contact.geozone=contact_new.geozone;
			contact.timezone=contact_new.timezone;
			if (xmlrpc_contact_write(config.host, config.port, config.uri, config.username, config.password, 0, &contact)<0) {
				printf("Error connecting to server.\r\n");
				return -1;
			}
			recs_out++;
		}
	}
	printf("%d contacts\n%d imported\n%d updated\n%d exported\n", contacts.records, recs_new, recs_upd, recs_out);
	return 0;
}

int main(int argc, char *argv[])
{
	char function[16];
	char parameter[64];
	char *pTemp;

	setvbuf(stdout, NULL, _IONBF, 0);
	if (config_read(&config)<0) return 0;
	memset(function, 0, sizeof(function));
	memset(parameter, 0, sizeof(parameter));
	if (argc!=3) {
		usage(argv[0]);
		return 0;
	}
	strncpy(function, argv[1], sizeof(function)-1);
	pTemp=function;
	while (*pTemp) {
		*pTemp=tolower(*pTemp);
		pTemp++;
	}
	strncpy(parameter, argv[2], sizeof(parameter)-1);
	if (strcmp(function, "dump")==0) {
		if (xmlrpc_contact_listopen(config.host, config.port, config.uri, config.username, config.password, &contacts)<0) {
			printf("Error connecting to server.\r\n");
			return 0;
		}
		dump_db(parameter);
		xmlrpc_contact_listclose(&contacts);
		return 0;
	}
	if (strcmp(function, "restore")==0) {
		if (xmlrpc_contact_listopen(config.host, config.port, config.uri, config.username, config.password, &contacts)<0) {
			printf("Error connecting to server.\r\n");
			return 0;
		}
		restore_db(parameter);
		xmlrpc_contact_listclose(&contacts);
		return 0;
	}
	return 0;
}
