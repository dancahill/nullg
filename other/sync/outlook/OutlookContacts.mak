# Microsoft Developer Studio Generated NMAKE File, Based on OutlookContacts.dsp
!IF "$(CFG)" == ""
CFG=OutlookContacts - Win32 Release
!MESSAGE OutlookContacts - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "OutlookContacts - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OutlookContacts.mak" CFG="OutlookContacts - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OutlookContacts - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\bin
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\..\bin
# End Custom Macros

ALL : "$(OUTDIR)\outlooksync.exe" ".\obj\OutlookContacts.pch"


CLEAN :
	-@erase "$(INTDIR)\OutlookContacts.obj"
	-@erase "$(INTDIR)\OutlookContacts.pch"
	-@erase "$(INTDIR)\OutlookContacts.res"
	-@erase "$(INTDIR)\OutlookContactsDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\outlooksync.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\OutlookContacts.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\OutlookContacts.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\outlooksync.exe" 
LINK32_OBJS= \
	"$(INTDIR)\OutlookContacts.obj" \
	"$(INTDIR)\OutlookContactsDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\OutlookContacts.res"

"$(OUTDIR)\outlooksync.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "OutlookContacts - Win32 Release"
SOURCE=.\OutlookContacts.cpp

"$(INTDIR)\OutlookContacts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OutlookContacts.rc

"$(INTDIR)\OutlookContacts.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\OutlookContactsDlg.cpp

"$(INTDIR)\OutlookContactsDlg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StdAfx.cpp
CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\OutlookContacts.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\OutlookContacts.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<



!ENDIF 

