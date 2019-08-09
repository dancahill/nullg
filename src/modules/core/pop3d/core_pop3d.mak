# Microsoft Developer Studio Generated NMAKE File, Based on core_pop3d.dsp
!IF "$(CFG)" == ""
CFG=core_pop3d - Win32 Release
!MESSAGE core_pop3d - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "core_pop3d - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core_pop3d.mak" CFG="core_pop3d - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core_pop3d - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=..\..\..\..\obj\modules\core\pop3d
INTDIR=..\..\..\..\obj\modules\core\pop3d

ALL : "..\..\..\..\distrib\lib\core\pop3d.dll"


CLEAN :
	-@erase "$(INTDIR)\pop3d_auth.obj"
	-@erase "$(INTDIR)\pop3d_conf.obj"
	-@erase "$(INTDIR)\pop3d_main.obj"
	-@erase "$(INTDIR)\pop3d_server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\core_pop3d.exp"
	-@erase "..\..\..\..\distrib\lib\core\pop3d.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\core_pop3d.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\core_pop3d.pdb" /machine:I386 /out:"..\..\..\..\distrib\lib\core\pop3d.dll" /implib:"$(OUTDIR)\core_pop3d.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pop3d_auth.obj" \
	"$(INTDIR)\pop3d_conf.obj" \
	"$(INTDIR)\pop3d_main.obj" \
	"$(INTDIR)\pop3d_server.obj"

"..\..\..\..\distrib\lib\core\pop3d.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "core_pop3d - Win32 Release"
SOURCE=.\pop3d_auth.c

"$(INTDIR)\pop3d_auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pop3d_conf.c

"$(INTDIR)\pop3d_conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pop3d_main.c

"$(INTDIR)\pop3d_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pop3d_server.c

"$(INTDIR)\pop3d_server.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

