/*
    NullLogic GroupServer - Copyright (C) 2000-2023 Dan Cahill

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
#include "stdafx.h"
#include "OutlookContacts.h"
#include "OutlookContactsDlg.h"
#include "../xmlsync/xmlsync.h"
#pragma comment(lib, "../bin/xmlsync.lib")

#ifdef _DEBUG
	#define new DEBUG_NEW
#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

#import "C:\Program Files\Microsoft Office\Office\mso9.dll" named_guids
#import "C:\Program Files\Microsoft Office\Office\MSOUTL9.OLB" no_namespace exclude("_IRecipientControl", "_DRecipientControl")
/*
#import "E:\Program Files\Common Files\Microsoft Shared\Office10\mso.dll" named_guids
#import "E:\Microsoft Office\Office10\MSOUTL.OLB" no_namespace exclude("_IRecipientControl", "_DRecipientControl")
*/

COutlookContactsDlg::COutlookContactsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COutlookContactsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COutlookContactsDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COutlookContactsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutlookContactsDlg)
	DDX_Control(pDX,IDC_CONTACTLIST,m_ContactList);
	DDX_Control(pDX,IDC_OPTION,m_Option);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COutlookContactsDlg, CDialog)
	//{{AFX_MSG_MAP(COutlookContactsDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COutlookContactsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COutlookContactsDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR COutlookContactsDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

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

void COutlookContactsDlg::OnImport() 
{
	_ApplicationPtr pApp;
	_ItemsPtr pItems;
	MAPIFolderPtr pFolder;
	_ContactItemPtr pContact;
	HRESULT hr;

	time_t t;
	char timebuf[40];
	char ctemp[160];
	char *ptemp;
	RECLIST_CONTACT contacts;
	REC_CONTACT contact;
	unsigned int i;
	short int found;
	short int recs_new=0;
	short int recs_upd=0;
	short int recs_out=0;
	CONFIG config;

	try {
		if (config_read(&config)<0) return;
		hr=pApp.CreateInstance(__uuidof(Application));
		if (FAILED(hr)) {
			MessageBox("Unable to instantiate Outlook.","Outlook Error",MB_OK);
			return;
		}
		if (m_Option.GetCheck()) { //default outlook contacts folder
			pFolder=pApp->GetNamespace(_bstr_t("MAPI"))->GetDefaultFolder(olFolderContacts);
			if (pFolder==NULL) {
				MessageBox("Could not find default contacts folder.","Outlook Error");
				return;
			}
		} else { //display folder selection window
			pFolder=pApp->GetNamespace(_bstr_t("MAPI"))->PickFolder();
			if (pFolder==NULL) return;
			if (pFolder->GetDefaultItemType()!=olContactItem) {
				MessageBox("Select folder is not a Contact folder.","Outlook Contacts");
				return;
			}
		}
		pItems=pFolder->GetItems();
		if (pItems==NULL) {
			MessageBox("Unabel to get Contact Items.","Outlook Error");
			return;
		}
		pContact=pItems->GetFirst();
		m_ContactList.ResetContent();
		t=time(NULL);
		strftime(timebuf, 30, "%Y-%m-%d %H:%M:%S", gmtime(&t));
		if (xmlrpc_contact_listopen(config.host, config.port, config.uri, config.username, config.password, &contacts)<0) {
			MessageBox("Error connecting to server.");
			return;
		}
		while (1) {
			if (pContact==NULL) break;
			found=-1;
			for (i=0;i<contacts.records;i++) {
				if ((!pContact->GetLastName())||(!pContact->GetFirstName())) break;
				if ((stricmp(pContact->GetLastName(), contacts.contact[i]->surname)!=0)||(stricmp(pContact->GetFirstName(), contacts.contact[i]->givenname)!=0)) continue;
				_snprintf(ctemp, sizeof(ctemp)-1, "U %s, %s <%s>", contacts.contact[i]->surname, contacts.contact[i]->givenname, contacts.contact[i]->email);
				m_ContactList.AddString(ctemp);
				recs_upd++;
				pContact->PutNickName(contacts.contact[i]->username);				// username
				pContact->PutLastName(contacts.contact[i]->surname);				// surname
				pContact->PutFirstName(contacts.contact[i]->givenname);				// givenname
				pContact->PutTitle(contacts.contact[i]->salutation);				// salutation
				pContact->PutReferredBy(contacts.contact[i]->referredby);			// referredby
				pContact->PutEmail1Address(contacts.contact[i]->email);				// email
				pContact->PutHomeTelephoneNumber(contacts.contact[i]->homenumber);		// homenumber
				pContact->PutBusinessTelephoneNumber(contacts.contact[i]->worknumber);		// worknumber
				pContact->PutBusinessFaxNumber(contacts.contact[i]->faxnumber);			// faxnumber
				pContact->PutMobileTelephoneNumber(contacts.contact[i]->mobilenumber);		// mobilenumber
				pContact->PutJobTitle(contacts.contact[i]->jobtitle);				// jobtitle
				pContact->PutCompanyName(contacts.contact[i]->organization);			// organization
				pContact->PutHomeAddressStreet(contacts.contact[i]->homeaddress);		// homeaddress
				pContact->PutHomeAddressCity(contacts.contact[i]->homelocality);		// homelocality
				pContact->PutHomeAddressState(contacts.contact[i]->homeregion);			// homeregion
				pContact->PutHomeAddressCountry(contacts.contact[i]->homecountry);		// homecountry
				pContact->PutHomeAddressPostalCode(contacts.contact[i]->homepostalcode);	// homepostalcode
				pContact->PutBusinessAddressStreet(contacts.contact[i]->workaddress);		// workaddress
				pContact->PutBusinessAddressCity(contacts.contact[i]->worklocality);		// worklocality
				pContact->PutBusinessAddressState(contacts.contact[i]->workregion);		// workregion
				pContact->PutBusinessAddressCountry(contacts.contact[i]->workcountry);		// workcountry
				pContact->PutBusinessAddressPostalCode(contacts.contact[i]->workpostalcode);	// workpostalcode
				pContact->Save();
				contacts.contact[i]->contactid=-1;
				found=i;
			}
			if (found==-1) {
				_snprintf(ctemp, sizeof(ctemp)-1, "O %s, %s <%s>", (ptemp=pContact->GetLastName())!=NULL?ptemp:"", (ptemp=pContact->GetFirstName())!=NULL?ptemp:"", (ptemp=pContact->GetEmail1Address())!=NULL?ptemp:"");
				m_ContactList.AddString(ctemp);
				memset((char *)&contact, 0, sizeof(REC_CONTACT));
				if (xmlrpc_contact_read(config.host, config.port, config.uri, config.username, config.password, 0, &contact)<0) {
					MessageBox("Error connecting to server.");
					return;
				}
//				contact.contactid=atoi(val);
//				contact.obj_ctime=atoi(val);
//				contact.obj_mtime=atoi(val);
//				contact.obj_uid=atoi(val);
//				contact.obj_gid=atoi(val);
//				contact.obj_did=atoi(val);
//				contact.obj_gperm=atoi(val);
//				contact.obj_operm=atoi(val);
//				strncpy(contact.loginip, val, sizeof(contact.loginip)-1);
//				contact.logintime=atoi(val);
//				strncpy(contact.logintoken, val, sizeof(contact.logintoken)-1);
				strncpy(contact.username,       (ptemp=pContact->GetNickName())!=NULL?ptemp:"",                  sizeof(contact.username)-1);
//				strncpy(contact.password, val, sizeof(contact.password)-1);
//				contact.enabled=atoi(val);
//				contact.geozone=atoi(val);
//				contact.timezone=atoi(val);
				strncpy(contact.surname,        (ptemp=pContact->GetLastName())!=NULL?ptemp:"",                  sizeof(contact.surname)-1);
				strncpy(contact.givenname,      (ptemp=pContact->GetFirstName())!=NULL?ptemp:"",                 sizeof(contact.givenname)-1);
				strncpy(contact.salutation,     (ptemp=pContact->GetTitle())!=NULL?ptemp:"",                     sizeof(contact.salutation)-1);
//				strncpy(contact.contacttype,    val, sizeof(contact.contacttype)-1);
				strncpy(contact.referredby,     (ptemp=pContact->GetReferredBy())!=NULL?ptemp:"",                sizeof(contact.referredby)-1);
//				strncpy(contact.altcontact,     val, sizeof(contact.altcontact)-1);
//				strncpy(contact.prefbilling,    val, sizeof(contact.prefbilling)-1);
				strncpy(contact.email,          (ptemp=pContact->GetEmail1Address())!=NULL?ptemp:"",             sizeof(contact.email)-1);
				strncpy(contact.homenumber,     (ptemp=pContact->GetHomeTelephoneNumber())!=NULL?ptemp:"",       sizeof(contact.homenumber)-1);
				strncpy(contact.worknumber,     (ptemp=pContact->GetBusinessTelephoneNumber())!=NULL?ptemp:"",   sizeof(contact.worknumber)-1);
				strncpy(contact.faxnumber,      (ptemp=pContact->GetBusinessFaxNumber())!=NULL?ptemp:"",         sizeof(contact.faxnumber)-1);
				strncpy(contact.mobilenumber,   (ptemp=pContact->GetMobileTelephoneNumber())!=NULL?ptemp:"",     sizeof(contact.mobilenumber)-1);
				strncpy(contact.jobtitle,       (ptemp=pContact->GetJobTitle())!=NULL?ptemp:"",                  sizeof(contact.jobtitle)-1);
				strncpy(contact.organization,   (ptemp=pContact->GetCompanyName())!=NULL?ptemp:"",               sizeof(contact.organization)-1);
				strncpy(contact.homeaddress,    (ptemp=pContact->GetHomeAddressStreet())!=NULL?ptemp:"",         sizeof(contact.homeaddress)-1);
				strncpy(contact.homelocality,   (ptemp=pContact->GetHomeAddressCity())!=NULL?ptemp:"",           sizeof(contact.homelocality)-1);
				strncpy(contact.homeregion,     (ptemp=pContact->GetHomeAddressState())!=NULL?ptemp:"",          sizeof(contact.homeregion)-1);
				strncpy(contact.homecountry,    (ptemp=pContact->GetHomeAddressCountry())!=NULL?ptemp:"",        sizeof(contact.homecountry)-1);
				strncpy(contact.homepostalcode, (ptemp=pContact->GetHomeAddressPostalCode())!=NULL?ptemp:"",     sizeof(contact.homepostalcode)-1);
				strncpy(contact.workaddress,    (ptemp=pContact->GetBusinessAddressStreet())!=NULL?ptemp:"",     sizeof(contact.workaddress)-1);
				strncpy(contact.worklocality,   (ptemp=pContact->GetBusinessAddressCity())!=NULL?ptemp:"",       sizeof(contact.worklocality)-1);
				strncpy(contact.workregion,     (ptemp=pContact->GetBusinessAddressState())!=NULL?ptemp:"",      sizeof(contact.workregion)-1);
				strncpy(contact.workcountry,    (ptemp=pContact->GetBusinessAddressCountry())!=NULL?ptemp:"",    sizeof(contact.workcountry)-1);
				strncpy(contact.workpostalcode, (ptemp=pContact->GetBusinessAddressPostalCode())!=NULL?ptemp:"", sizeof(contact.workpostalcode)-1);
				if (xmlrpc_contact_write(config.host, config.port, config.uri, config.username, config.password, 0, &contact)<0) {
					MessageBox("Error connecting to server.");
					return;
				}
				recs_out++;
			}
			pContact=pItems->GetNext();
		}
		for (i=0;i<contacts.records;i++) {
			if (contacts.contact[i]->contactid>0) {
				_snprintf(ctemp, sizeof(ctemp)-1, "N %s, %s <%s>", contacts.contact[i]->surname, contacts.contact[i]->givenname, contacts.contact[i]->email);
				m_ContactList.AddString(ctemp);
				pContact=pItems->Add();
				pContact->PutNickName(contacts.contact[i]->username);				// username
				pContact->PutLastName(contacts.contact[i]->surname);				// surname
				pContact->PutFirstName(contacts.contact[i]->givenname);				// givenname
				pContact->PutTitle(contacts.contact[i]->salutation);				// salutation
				pContact->PutReferredBy(contacts.contact[i]->referredby);			// referredby
				pContact->PutEmail1Address(contacts.contact[i]->email);				// email
				pContact->PutHomeTelephoneNumber(contacts.contact[i]->homenumber);		// homenumber
				pContact->PutBusinessTelephoneNumber(contacts.contact[i]->worknumber);		// worknumber
				pContact->PutBusinessFaxNumber(contacts.contact[i]->faxnumber);			// faxnumber
				pContact->PutMobileTelephoneNumber(contacts.contact[i]->mobilenumber);		// mobilenumber
				pContact->PutJobTitle(contacts.contact[i]->jobtitle);				// jobtitle
				pContact->PutCompanyName(contacts.contact[i]->organization);			// organization
				pContact->PutHomeAddressStreet(contacts.contact[i]->homeaddress);		// homeaddress
				pContact->PutHomeAddressCity(contacts.contact[i]->homelocality);		// homelocality
				pContact->PutHomeAddressState(contacts.contact[i]->homeregion);			// homeregion
				pContact->PutHomeAddressCountry(contacts.contact[i]->homecountry);		// homecountry
				pContact->PutHomeAddressPostalCode(contacts.contact[i]->homepostalcode);	// homepostalcode
				pContact->PutBusinessAddressStreet(contacts.contact[i]->workaddress);		// workaddress
				pContact->PutBusinessAddressCity(contacts.contact[i]->worklocality);		// worklocality
				pContact->PutBusinessAddressState(contacts.contact[i]->workregion);		// workregion
				pContact->PutBusinessAddressCountry(contacts.contact[i]->workcountry);		// workcountry
				pContact->PutBusinessAddressPostalCode(contacts.contact[i]->workpostalcode);	// workpostalcode
				pContact->Save();
				recs_new++;
			}
		}
		_snprintf(ctemp, sizeof(ctemp)-1, "%d contacts\n%d imported\n%d updated\n%d exported\n", contacts.records, recs_new, recs_upd, recs_out);
		MessageBox(ctemp);
		xmlrpc_contact_listclose(&contacts);
	}
	catch (_com_error &e) {
		MessageBox((char *)e.Description());
	}
}
