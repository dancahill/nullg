# Microsoft Developer Studio Generated NMAKE File, Based on mod_admin.dsp
!IF "$(CFG)" == ""
CFG=mod_admin - Win32 Release
!MESSAGE No configuration specified. Defaulting to mod_admin - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "mod_admin - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mod_admin.mak" CFG="mod_admin - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mod_admin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\obj\mod_admin
INTDIR=.\..\..\obj\mod_admin

ALL : "..\..\distrib\lib\mod_admin.dll"


CLEAN :
	-@erase "$(INTDIR)\mod_admin_config.obj"
	-@erase "$(INTDIR)\mod_admin_db.obj"
	-@erase "$(INTDIR)\mod_admin_groups.obj"
	-@erase "$(INTDIR)\mod_admin_logs.obj"
	-@erase "$(INTDIR)\mod_admin_main.obj"
	-@erase "$(INTDIR)\mod_admin_syscheck.obj"
	-@erase "$(INTDIR)\mod_admin_users.obj"
	-@erase "$(INTDIR)\mod_admin_zones.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mod_admin.exp"
	-@erase "..\..\distrib\lib\mod_admin.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../include" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mod_admin.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\mod_admin.pdb" /machine:I386 /def:".\mod_admin.def" /out:"..\..\distrib\lib\mod_admin.dll" /implib:"$(OUTDIR)\mod_admin.lib" 
DEF_FILE= \
	".\mod_admin.def"
LINK32_OBJS= \
	"$(INTDIR)\mod_admin_config.obj" \
	"$(INTDIR)\mod_admin_db.obj" \
	"$(INTDIR)\mod_admin_groups.obj" \
	"$(INTDIR)\mod_admin_logs.obj" \
	"$(INTDIR)\mod_admin_main.obj" \
	"$(INTDIR)\mod_admin_syscheck.obj" \
	"$(INTDIR)\mod_admin_users.obj" \
	"$(INTDIR)\mod_admin_zones.obj"

"..\..\distrib\lib\mod_admin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("mod_admin.dep")
!INCLUDE "mod_admin.dep"
!ELSE 
!MESSAGE Warning: cannot find "mod_admin.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mod_admin - Win32 Release"
SOURCE=.\mod_admin_config.c

"$(INTDIR)\mod_admin_config.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_db.c

"$(INTDIR)\mod_admin_db.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_groups.c

"$(INTDIR)\mod_admin_groups.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_logs.c

"$(INTDIR)\mod_admin_logs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_main.c

"$(INTDIR)\mod_admin_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_syscheck.c

"$(INTDIR)\mod_admin_syscheck.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_users.c

"$(INTDIR)\mod_admin_users.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mod_admin_zones.c

"$(INTDIR)\mod_admin_zones.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

