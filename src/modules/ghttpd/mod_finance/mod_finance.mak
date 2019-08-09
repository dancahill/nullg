# Microsoft Developer Studio Generated NMAKE File, Based on mod_finance.dsp
!IF "$(CFG)" == ""
CFG=mod_finance - Win32 Release
!MESSAGE mod_finance - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_finance - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_finance.mak" CFG="mod_finance - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_finance - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=..\..\..\..\obj\modules\ghttpd\mod_finance
INTDIR=..\..\..\..\obj\modules\ghttpd\mod_finance

ALL : "..\..\..\..\distrib\lib\ghttpd\mod_finance.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_finance_accounts.obj"
	-@erase "$(INTDIR)\mod_finance_db.obj"
	-@erase "$(INTDIR)\mod_finance_inventory.obj"
	-@erase "$(INTDIR)\mod_finance_invoices.obj"
	-@erase "$(INTDIR)\mod_finance_invoiceitems.obj"
	-@erase "$(INTDIR)\mod_finance_journal.obj"
	-@erase "$(INTDIR)\mod_finance_lang.obj"
	-@erase "$(INTDIR)\mod_finance_main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_finance.exp"
	-@erase "$(OUTDIR)\mod_finance.lib"
	-@erase "..\..\..\..\distrib\lib\ghttpd\mod_finance.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_finance.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\mod_finance.pdb" /machine:I386 /def:".\mod_finance.def" /out:"..\..\..\..\distrib\lib\ghttpd\mod_finance.dll" /implib:"$(OUTDIR)\mod_finance.lib" 
DEF_FILE= \
	".\mod_finance.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_finance_accounts.obj" \
	"$(INTDIR)\mod_finance_db.obj" \
	"$(INTDIR)\mod_finance_inventory.obj" \
	"$(INTDIR)\mod_finance_invoices.obj" \
	"$(INTDIR)\mod_finance_invoiceitems.obj" \
	"$(INTDIR)\mod_finance_journal.obj" \
	"$(INTDIR)\mod_finance_lang.obj" \
	"$(INTDIR)\mod_finance_main.obj"

"..\..\..\..\distrib\lib\ghttpd\mod_finance.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "mod_finance - Win32 Release"
SOURCE=.\mod_finance_accounts.c

"$(INTDIR)\mod_finance_accounts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_db.c

"$(INTDIR)\mod_finance_db.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_inventory.c

"$(INTDIR)\mod_finance_inventory.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_invoices.c

"$(INTDIR)\mod_finance_invoices.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_invoiceitems.c

"$(INTDIR)\mod_finance_invoiceitems.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_journal.c

"$(INTDIR)\mod_finance_journal.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_lang.c

"$(INTDIR)\mod_finance_lang.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_finance_main.c

"$(INTDIR)\mod_finance_main.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

