/*
    NullLogic Groupware - Copyright (C) 2000-2005 Dan Cahill

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
// OutlookContacts.h : main header file for the OUTLOOKCONTACTS application
//

#if !defined(AFX_OUTLOOKCONTACTS_H__668A0876_AC80_47B0_A1B8_2D510914D8E4__INCLUDED_)
#define AFX_OUTLOOKCONTACTS_H__668A0876_AC80_47B0_A1B8_2D510914D8E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// COutlookContactsApp:
// See OutlookContacts.cpp for the implementation of this class
//

class COutlookContactsApp : public CWinApp
{
public:
	COutlookContactsApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutlookContactsApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(COutlookContactsApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTLOOKCONTACTS_H__668A0876_AC80_47B0_A1B8_2D510914D8E4__INCLUDED_)
