# Microsoft Developer Studio Generated NMAKE File, Based on mod_searches.dsp
!IF "$(CFG)" == ""
CFG=mod_searches - Win32 Release
!MESSAGE mod_searches - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_searches - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_searches.mak" CFG="mod_searches - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_searches - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=..\..\..\..\obj\modules\ghttpd\mod_searches
INTDIR=..\..\..\..\obj\modules\ghttpd\mod_searches

ALL : "..\..\..\..\distrib\lib\ghttpd\mod_searches.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_searches_contacts.obj"
	-@erase "$(INTDIR)\mod_searches_db.obj"
	-@erase "$(INTDIR)\mod_searches_lang.obj"
	-@erase "$(INTDIR)\mod_searches_main.obj"
	-@erase "$(INTDIR)\mod_searches_sql.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_searches.exp"
	-@erase "$(OUTDIR)\mod_searches.lib"
	-@erase "..\..\..\..\distrib\lib\ghttpd\mod_searches.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_searches.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\mod_searches.pdb" /machine:I386 /def:".\mod_searches.def" /out:"..\..\..\..\distrib\lib\ghttpd\mod_searches.dll" /implib:"$(OUTDIR)\mod_searches.lib" 
DEF_FILE= \
	".\mod_searches.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_searches_contacts.obj" \
	"$(INTDIR)\mod_searches_db.obj" \
	"$(INTDIR)\mod_searches_lang.obj" \
	"$(INTDIR)\mod_searches_main.obj" \
	"$(INTDIR)\mod_searches_sql.obj"

"..\..\..\..\distrib\lib\ghttpd\mod_searches.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "mod_searches - Win32 Release"
SOURCE=.\mod_searches_contacts.c

"$(INTDIR)\mod_searches_contacts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_searches_db.c

"$(INTDIR)\mod_searches_db.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_searches_lang.c

"$(INTDIR)\mod_searches_lang.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_searches_main.c

"$(INTDIR)\mod_searches_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_searches_sql.c

"$(INTDIR)\mod_searches_sql.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

