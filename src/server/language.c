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

typedef	void (*LANG_CALLBACK)(char *, char *);

int language_read(char *langcode, char *section, void *callback)
{
	obj_t *tobj=nsp_getobj(proc.N, &proc.N->g, "CONFIG");
	obj_t *cobj=nsp_getobj(proc.N, tobj, "var_path");
	LANG_CALLBACK language_callback=(LANG_CALLBACK)callback;
	FILE *fp=NULL;
	char file[256];
	char line[512];
	char *pVar;
	char *pVal;
	int i;
	short int match;

	if (cobj->val->type!=NT_STRING) return 0;
	snprintf(file, sizeof(file)-1, "%s/share/locale/%s/strings", cobj->val->d.str, langcode);
	if ((fp=fopen(file, "r"))==NULL) {
		log_error(proc.N, "core", __FILE__, __LINE__, 0, "ERROR: Language file '%s' does not exist.", file);
		return -1;
	}
	match=0;
	memset(line, 0, sizeof(line));
	while (fgets(line, sizeof(line)-1, fp)!=NULL) {
		while (1) {
			i=strlen(line);
			if (i<1) break;
			if (line[i-1]=='\r') { line[i-1]='\0'; continue; }
			if (line[i-1]=='\n') { line[i-1]='\0'; continue; }
			break;
		};
		if ((pVar=strchr(line, '#'))!=NULL) *pVar='\0';
		if ((pVar=strchr(line, ';'))!=NULL) *pVar='\0';
		pVar=line;
		pVal=strchr(line, ']');
		if ((pVar[0]=='[')&&(pVal!=NULL)) {
			match=0;
			pVar++;
			*pVal='\0';
			if (strcasecmp(pVar, "common")==0) match=1;
			if (strcasecmp(pVar, section)==0) match=1;
			continue;
		}
		if (match) {
			pVar=line;
			pVal=line;
			while ((pVal[0]!='=')&&(pVal[1]!='\0')) pVal++;
			*pVal='\0';
			pVal++;
			while (*pVar==' ') pVar++;
			while (pVar[strlen(pVar)-1]==' ') pVar[strlen(pVar)-1]='\0';
			while (*pVal==' ') pVal++;
			while (pVal[strlen(pVal)-1]==' ') pVal[strlen(pVal)-1]='\0';
			if ((pVal[0]=='"')&&(pVal[strlen(pVal)-1]=='"')) {
				pVal++;
				pVal[strlen(pVal)-1]='\0';
			}
			if (pVar[0]!='\0') {
				language_callback(pVar, pVal);
			}
		}
	}
	fclose(fp);
	return 0;
}
/*
static void lang_callback(char *var, char *val)
{
	return;
}
*/
int lang_read()
{
	return 0;
}

char *language_gets(nsp_state *N, char *lang, char *sect, char *label)
{
	return "[empty string]";
}
