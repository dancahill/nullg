# Microsoft Developer Studio Generated NMAKE File, Based on mod_accounting.dsp
!IF "$(CFG)" == ""
CFG=mod_accounting - Win32 Release
!MESSAGE mod_accounting - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_accounting - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_accounting.mak" CFG="mod_accounting - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_accounting - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\..\obj\ghttpd_modules\mod_accounting
INTDIR=.\..\..\..\obj\ghttpd_modules\mod_accounting

ALL : "..\..\..\distrib\lib\ghttpd\mod_accounting.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_accounting_accounts.obj"
	-@erase "$(INTDIR)\mod_accounting_db.obj"
	-@erase "$(INTDIR)\mod_accounting_journal.obj"
	-@erase "$(INTDIR)\mod_accounting_lang.obj"
	-@erase "$(INTDIR)\mod_accounting_main.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_accounting.exp"
	-@erase "$(OUTDIR)\mod_accounting.lib"
	-@erase "..\..\..\distrib\lib\ghttpd\mod_accounting.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_accounting.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\mod_accounting.pdb" /machine:I386 /def:".\mod_accounting.def" /out:"..\..\..\distrib\lib\ghttpd\mod_accounting.dll" /implib:"$(OUTDIR)\mod_accounting.lib" 
DEF_FILE= \
	".\mod_accounting.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_accounting_accounts.obj" \
	"$(INTDIR)\mod_accounting_db.obj" \
	"$(INTDIR)\mod_accounting_journal.obj" \
	"$(INTDIR)\mod_accounting_lang.obj" \
	"$(INTDIR)\mod_accounting_main.obj"

"..\..\..\distrib\lib\ghttpd\mod_accounting.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "mod_accounting - Win32 Release"
SOURCE=.\mod_accounting_accounts.c

"$(INTDIR)\mod_accounting_accounts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_accounting_db.c

"$(INTDIR)\mod_accounting_db.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_accounting_journal.c

"$(INTDIR)\mod_accounting_journal.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_accounting_lang.c

"$(INTDIR)\mod_accounting_lang.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_accounting_main.c

"$(INTDIR)\mod_accounting_main.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

