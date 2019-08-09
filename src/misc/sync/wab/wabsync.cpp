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
#include "../xmlsync/xmlsync.h"
#include <wab.h>

#ifdef WIN32
#pragma comment(lib, "../bin/xmlsync.lib")
#endif

typedef HRESULT (WINAPI *fWABOpen)(LPADRBOOK*, LPWABOBJECT*, LPWAB_PARAM, DWORD);

enum {
	ieidPR_DISPLAY_NAME = 0,
	ieidPR_ENTRYID,
	ieidPR_OBJECT_TYPE,
	ieidMax
};
static const SizedSPropTagArray(ieidMax, ptaEid)={
	ieidMax,{
		PR_DISPLAY_NAME,
		PR_ENTRYID,
		PR_OBJECT_TYPE,
	}
};

int config_read(CONFIG *config)
{
	FILE *fp=NULL;
	char line[512];
	char *pVar;
	char *pVal;
	short int founddir=0;
	int i;

	_snprintf(config->host,     sizeof(config->host)-1,     "localhost");
	config->port=4110;
	_snprintf(config->uri,      sizeof(config->uri)-1,      "/xml-rpc/");
	_snprintf(config->username, sizeof(config->username)-1, "administrator");
	_snprintf(config->password, sizeof(config->password)-1, "visual");
	/* try to open the config file */
	fp=fopen("xmlsync.cfg", "r");
	/* if config file couldn't be opened, try to write one */
	if (fp==NULL) {
		fp=fopen("xmlsync.cfg", "w");
		if (fp==NULL) return -1;
		fprintf(fp, "HOST     = \"%s\"\n", config->host);
		fprintf(fp, "PORT     = \"%d\"\n", config->port);
		fprintf(fp, "URI      = \"%s\"\n", config->uri);
		fprintf(fp, "USERNAME = \"%s\"\n", config->username);
		fprintf(fp, "PASSWORD = \"%s\"\n", config->password);
		fclose(fp);
		MessageBox(NULL, "No configuration file found.  Creating...\nPlease edit xmlsync.cfg","Outlook Error",MB_OK);
		exit(0);
	}
	/* else if config file does exist, read it */
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
				_snprintf(config->host,     sizeof(config->host)-1,     "%s", pVal);
			} else if (strcmp(pVar, "PORT")==0) {
				config->port=atoi(pVal);
			} else if (strcmp(pVar, "URI")==0) {
				_snprintf(config->uri,      sizeof(config->uri)-1,      "%s", pVal);
			} else if (strcmp(pVar, "USERNAME")==0) {
				_snprintf(config->username, sizeof(config->username)-1, "%s", pVal);
			} else if (strcmp(pVar, "PASSWORD")==0) {
				_snprintf(config->password, sizeof(config->password)-1, "%s", pVal);
			}
			*pVal='\0';
			*pVar='\0';
		}
	}
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	HRESULT hRes;
	LPADRBOOK lpAdrBook;
	LPWABOBJECT lpWABObject;
	LPWAB_PARAM lpWABParam = NULL;
	DWORD Reserved2 = NULL;
	HINSTANCE hinstLib;
	fWABOpen procWABOpen;
	ULONG lpcbEntryID;
	ENTRYID *lpEntryID;
	ULONG ulObjType = NULL;
	LPABCONT lpContainer = NULL;
	LPMAPITABLE lpTable = NULL;
	LPSRowSet lpRowAB = NULL;
	int cNumRows = 0;
	CONFIG config;

	RECLIST_CONTACT contacts;
	REC_CONTACT contact;
	REC_CONTACT contact_new;
	time_t t;
	char timebuf[40];
	char ctemp[160];
	unsigned int i;
	short int found;
	short int recs_new=0;
	short int recs_upd=0;
	short int recs_out=0;

	if (config_read(&config)<0) return -1;
	t=time(NULL);
	strftime(timebuf, 30, "%Y-%m-%d %H:%M:%S", gmtime(&t));
	if (xmlrpc_contact_listopen(config.host, config.port, config.uri, config.username, config.password, &contacts)<0) {
		printf("Error connecting to server.\r\n");
		return -1;
	}
	memset((char *)&contact_new, 0, sizeof(REC_CONTACT));
	if (xmlrpc_contact_read(config.host, config.port, config.uri, config.username, config.password, 0, &contact_new)<0) {
		printf("Error connecting to server.\r\n");
		return -1;
	}
	if ((hinstLib=LoadLibrary("C:\\Program Files\\Common Files\\System\\wab32"))==NULL) return 0;
	if ((procWABOpen=(fWABOpen)GetProcAddress(hinstLib, "WABOpen"))==NULL) goto cleanup;
	if ((hRes=(procWABOpen)(&lpAdrBook, &lpWABObject, NULL, Reserved2))!=S_OK) goto cleanup;
	if ((hRes=lpAdrBook->GetPAB(&lpcbEntryID, &lpEntryID))!=S_OK) goto cleanup;
	hRes=lpAdrBook->OpenEntry(lpcbEntryID, lpEntryID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpContainer);
	lpWABObject->FreeBuffer(lpEntryID);
	hRes=lpContainer->GetContentsTable(0, &lpTable);
	hRes=lpTable->SetColumns((LPSPropTagArray)&ptaEid, 0);
	hRes=lpTable->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
	do {
		memset((char *)&contact, 0, sizeof(REC_CONTACT));
		hRes=lpTable->QueryRows(1, 0, &lpRowAB);
		if (lpRowAB) {
			cNumRows=lpRowAB->cRows;
			if (cNumRows) {
				LPTSTR lpsz = lpRowAB->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.lpszA;
				LPENTRYID lpEID = (LPENTRYID) lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
				ULONG cbEID = lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;
				LPMAILUSER   lpMailUser = NULL;
				// Cache Proparray of currently selected item in the list view
				LPSPropValue m_lpPropArray;
				ULONG        m_ulcValues;

				hRes=lpAdrBook->OpenEntry(cbEID, lpEID, NULL, 0, &ulObjType, (LPUNKNOWN *)&lpMailUser);
				lpMailUser->GetProps(NULL, 0, &m_ulcValues, &m_lpPropArray);
				for (i=0;i<m_ulcValues;i++) {
					switch (m_lpPropArray[i].ulPropTag) {
					case PR_NICKNAME:
						snprintf(contact.username,       sizeof(contact.username)-1,       "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_SURNAME:
						snprintf(contact.surname,        sizeof(contact.surname)-1,        "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_GIVEN_NAME:
						snprintf(contact.givenname,      sizeof(contact.givenname)-1,      "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_DISPLAY_NAME_PREFIX:
						snprintf(contact.salutation,     sizeof(contact.salutation)-1,     "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_PREFERRED_BY_NAME:
						snprintf(contact.referredby,     sizeof(contact.referredby)-1,     "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_EMAIL_ADDRESS:
						snprintf(contact.email,          sizeof(contact.email)-1,          "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_HOME_TELEPHONE_NUMBER:
						snprintf(contact.homenumber,     sizeof(contact.homenumber)-1,     "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_OFFICE_TELEPHONE_NUMBER:
						snprintf(contact.worknumber,     sizeof(contact.worknumber)-1,     "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_BUSINESS_FAX_NUMBER:
						snprintf(contact.faxnumber,      sizeof(contact.faxnumber)-1,      "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_MOBILE_TELEPHONE_NUMBER:
						snprintf(contact.mobilenumber,   sizeof(contact.mobilenumber)-1,   "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_TITLE:
						snprintf(contact.jobtitle,       sizeof(contact.jobtitle)-1,       "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_COMPANY_NAME:
						snprintf(contact.organization,   sizeof(contact.organization)-1,   "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_HOME_ADDRESS_STREET:
						snprintf(contact.homeaddress,    sizeof(contact.homeaddress)-1,    "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_HOME_ADDRESS_CITY:
						snprintf(contact.homelocality,   sizeof(contact.homelocality)-1,   "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
						snprintf(contact.homeregion,     sizeof(contact.homeregion)-1,     "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_HOME_ADDRESS_COUNTRY:
						snprintf(contact.homecountry,    sizeof(contact.homecountry)-1,    "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_HOME_ADDRESS_POSTAL_CODE:
						snprintf(contact.homepostalcode, sizeof(contact.homepostalcode)-1, "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_BUSINESS_ADDRESS_STREET:
						snprintf(contact.workaddress,    sizeof(contact.workaddress)-1,    "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_BUSINESS_ADDRESS_CITY:
						snprintf(contact.worklocality,   sizeof(contact.worklocality)-1,   "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE:
						snprintf(contact.workregion,     sizeof(contact.workregion)-1,     "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_BUSINESS_ADDRESS_COUNTRY:
						snprintf(contact.workcountry,    sizeof(contact.workcountry)-1,    "%s", m_lpPropArray[i].Value.lpszA); break;
					case PR_BUSINESS_ADDRESS_POSTAL_CODE:
						snprintf(contact.workpostalcode, sizeof(contact.workpostalcode)-1, "%s", m_lpPropArray[i].Value.lpszA); break;
					default:
						break;
					}
				}
				found=-1;
				for (i=0;i<contacts.records;i++) {
//					if ((!pContact->GetLastName())||(!pContact->GetFirstName())) break;
					if ((stricmp(contact.surname, contacts.contact[i]->surname)!=0)||(stricmp(contact.givenname, contacts.contact[i]->givenname)!=0)) continue;
					_snprintf(ctemp, sizeof(ctemp)-1, "U %s, %s <%s>", contacts.contact[i]->surname, contacts.contact[i]->givenname, contacts.contact[i]->email);
//					m_ContactList.AddString(ctemp);
					recs_upd++;
//					pContact->PutNickName(contacts.contact[i]->username);				// username
//					pContact->PutLastName(contacts.contact[i]->surname);				// surname
//					pContact->PutFirstName(contacts.contact[i]->givenname);				// givenname
//					pContact->PutTitle(contacts.contact[i]->salutation);				// salutation
//					pContact->PutReferredBy(contacts.contact[i]->referredby);			// referredby
//					pContact->PutEmail1Address(contacts.contact[i]->email);				// email
//					pContact->PutHomeTelephoneNumber(contacts.contact[i]->homenumber);		// homenumber
//					pContact->PutBusinessTelephoneNumber(contacts.contact[i]->worknumber);		// worknumber
//					pContact->PutBusinessFaxNumber(contacts.contact[i]->faxnumber);			// faxnumber
//					pContact->PutMobileTelephoneNumber(contacts.contact[i]->mobilenumber);		// mobilenumber
//					pContact->PutJobTitle(contacts.contact[i]->jobtitle);				// jobtitle
//					pContact->PutCompanyName(contacts.contact[i]->organization);			// organization
//					pContact->PutHomeAddressStreet(contacts.contact[i]->homeaddress);		// homeaddress
//					pContact->PutHomeAddressCity(contacts.contact[i]->homelocality);		// homelocality
//					pContact->PutHomeAddressState(contacts.contact[i]->homeregion);			// homeregion
//					pContact->PutHomeAddressCountry(contacts.contact[i]->homecountry);		// homecountry
//					pContact->PutHomeAddressPostalCode(contacts.contact[i]->homepostalcode);	// homepostalcode
//					pContact->PutBusinessAddressStreet(contacts.contact[i]->workaddress);		// workaddress
//					pContact->PutBusinessAddressCity(contacts.contact[i]->worklocality);		// worklocality
//					pContact->PutBusinessAddressState(contacts.contact[i]->workregion);		// workregion
//					pContact->PutBusinessAddressCountry(contacts.contact[i]->workcountry);		// workcountry
//					pContact->PutBusinessAddressPostalCode(contacts.contact[i]->workpostalcode);	// workpostalcode
//					pContact->Save();
					contacts.contact[i]->contactid=-1;
					found=i;
				}
				if (found==-1) {
					_snprintf(ctemp, sizeof(ctemp)-1, "O %s, %s <%s>", contact.surname, contact.givenname, contact.email);
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
				lpWABObject->FreeBuffer(lpEID);
			}
		}
		lpWABObject->FreeBuffer(lpRowAB);
	} while (SUCCEEDED(hRes)&&cNumRows&&lpRowAB);
	for (i=0;i<contacts.records;i++) {
		if (contacts.contact[i]->contactid>0) {
			_snprintf(ctemp, sizeof(ctemp)-1, "N %s, %s <%s>", contacts.contact[i]->surname, contacts.contact[i]->givenname, contacts.contact[i]->email);
//			m_ContactList.AddString(ctemp);
//			pContact=pItems->Add();
//			pContact->PutNickName(contacts.contact[i]->username);				// username
//			pContact->PutLastName(contacts.contact[i]->surname);				// surname
//			pContact->PutFirstName(contacts.contact[i]->givenname);				// givenname
//			pContact->PutTitle(contacts.contact[i]->salutation);				// salutation
//			pContact->PutReferredBy(contacts.contact[i]->referredby);			// referredby
//			pContact->PutEmail1Address(contacts.contact[i]->email);				// email
//			pContact->PutHomeTelephoneNumber(contacts.contact[i]->homenumber);		// homenumber
//			pContact->PutBusinessTelephoneNumber(contacts.contact[i]->worknumber);		// worknumber
//			pContact->PutBusinessFaxNumber(contacts.contact[i]->faxnumber);			// faxnumber
//			pContact->PutMobileTelephoneNumber(contacts.contact[i]->mobilenumber);		// mobilenumber
//			pContact->PutJobTitle(contacts.contact[i]->jobtitle);				// jobtitle
//			pContact->PutCompanyName(contacts.contact[i]->organization);			// organization
//			pContact->PutHomeAddressStreet(contacts.contact[i]->homeaddress);		// homeaddress
//			pContact->PutHomeAddressCity(contacts.contact[i]->homelocality);		// homelocality
//			pContact->PutHomeAddressState(contacts.contact[i]->homeregion);			// homeregion
//			pContact->PutHomeAddressCountry(contacts.contact[i]->homecountry);		// homecountry
//			pContact->PutHomeAddressPostalCode(contacts.contact[i]->homepostalcode);	// homepostalcode
//			pContact->PutBusinessAddressStreet(contacts.contact[i]->workaddress);		// workaddress
//			pContact->PutBusinessAddressCity(contacts.contact[i]->worklocality);		// worklocality
//			pContact->PutBusinessAddressState(contacts.contact[i]->workregion);		// workregion
//			pContact->PutBusinessAddressCountry(contacts.contact[i]->workcountry);		// workcountry
//			pContact->PutBusinessAddressPostalCode(contacts.contact[i]->workpostalcode);	// workpostalcode
//			pContact->Save();
			recs_new++;
		}
	}
	lpWABObject->FreeBuffer(lpTable);
cleanup:
	FreeLibrary(hinstLib);
	_snprintf(ctemp, sizeof(ctemp)-1, "%d contacts\n%d imported\n%d updated\n%d exported\n", contacts.records, recs_new, recs_upd, recs_out);
	xmlrpc_contact_listclose(&contacts);
	return 0;
}
