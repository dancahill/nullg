# Microsoft Developer Studio Generated NMAKE File, Based on mod_calendar.dsp
!IF "$(CFG)" == ""
CFG=mod_calendar - Win32 Release
!MESSAGE mod_calendar - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_calendar - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_calendar.mak" CFG="mod_calendar - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_calendar - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\..\obj\httpd_modules\mod_calendar
INTDIR=.\..\..\..\obj\httpd_modules\mod_calendar

ALL : "..\..\..\distrib\lib\httpd\mod_calendar.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_calendar_assign.obj"
	-@erase "$(INTDIR)\mod_calendar_availmap.obj"
	-@erase "$(INTDIR)\mod_calendar_db.obj"
	-@erase "$(INTDIR)\mod_calendar_lang.obj"
	-@erase "$(INTDIR)\mod_calendar_listd.obj"
	-@erase "$(INTDIR)\mod_calendar_listm.obj"
	-@erase "$(INTDIR)\mod_calendar_listw.obj"
	-@erase "$(INTDIR)\mod_calendar_listy.obj"
	-@erase "$(INTDIR)\mod_calendar_main.obj"
	-@erase "$(INTDIR)\mod_calendar_mini.obj"
	-@erase "$(INTDIR)\mod_calendar_vcal.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_calendar.exp"
	-@erase "$(OUTDIR)\mod_calendar.lib"
	-@erase "..\..\..\distrib\lib\httpd\mod_calendar.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_calendar.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\mod_calendar.pdb" /machine:I386 /def:".\mod_calendar.def" /out:"..\..\..\distrib\lib\httpd\mod_calendar.dll" /implib:"$(OUTDIR)\mod_calendar.lib" 
DEF_FILE= \
	".\mod_calendar.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_calendar_assign.obj" \
	"$(INTDIR)\mod_calendar_availmap.obj" \
	"$(INTDIR)\mod_calendar_db.obj" \
	"$(INTDIR)\mod_calendar_lang.obj" \
	"$(INTDIR)\mod_calendar_listd.obj" \
	"$(INTDIR)\mod_calendar_listw.obj" \
	"$(INTDIR)\mod_calendar_listm.obj" \
	"$(INTDIR)\mod_calendar_listy.obj" \
	"$(INTDIR)\mod_calendar_main.obj" \
	"$(INTDIR)\mod_calendar_mini.obj" \
	"$(INTDIR)\mod_calendar_vcal.obj"

"..\..\..\distrib\lib\httpd\mod_calendar.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "mod_calendar - Win32 Release"
SOURCE=.\mod_calendar_assign.c

"$(INTDIR)\mod_calendar_assign.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_availmap.c

"$(INTDIR)\mod_calendar_availmap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_db.c

"$(INTDIR)\mod_calendar_db.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_lang.c

"$(INTDIR)\mod_calendar_lang.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_listd.c

"$(INTDIR)\mod_calendar_listd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_listm.c

"$(INTDIR)\mod_calendar_listm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_listw.c

"$(INTDIR)\mod_calendar_listw.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_listy.c

"$(INTDIR)\mod_calendar_listy.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_main.c

"$(INTDIR)\mod_calendar_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_mini.c

"$(INTDIR)\mod_calendar_mini.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_calendar_vcal.c

"$(INTDIR)\mod_calendar_vcal.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

