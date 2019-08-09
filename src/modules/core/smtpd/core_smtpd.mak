# Microsoft Developer Studio Generated NMAKE File, Based on core_smtpd.dsp
!IF "$(CFG)" == ""
CFG=core_smtpd - Win32 Release
!MESSAGE core_smtpd - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "core_smtpd - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core_smtpd.mak" CFG="core_smtpd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core_smtpd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=..\..\..\..\obj\modules\core\smtpd
INTDIR=..\..\..\..\obj\modules\core\smtpd

ALL : "..\..\..\..\distrib\lib\core\smtpd.dll"


CLEAN :
	-@erase "$(INTDIR)\smtpd_auth.obj"
	-@erase "$(INTDIR)\smtpd_bounce.obj"
	-@erase "$(INTDIR)\smtpd_client.obj"
	-@erase "$(INTDIR)\smtpd_conf.obj"
	-@erase "$(INTDIR)\smtpd_filter.obj"
	-@erase "$(INTDIR)\smtpd_main.obj"
	-@erase "$(INTDIR)\smtpd_server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\core_smtpd.exp"
	-@erase "..\..\..\..\distrib\lib\core\smtpd.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\core_smtpd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\core_smtpd.pdb" /machine:I386 /out:"..\..\..\..\distrib\lib\core\smtpd.dll" /implib:"$(OUTDIR)\core_smtpd.lib" 
LINK32_OBJS= \
	"$(INTDIR)\smtpd_auth.obj" \
	"$(INTDIR)\smtpd_bounce.obj" \
	"$(INTDIR)\smtpd_client.obj" \
	"$(INTDIR)\smtpd_conf.obj" \
	"$(INTDIR)\smtpd_filter.obj" \
	"$(INTDIR)\smtpd_main.obj" \
	"$(INTDIR)\smtpd_server.obj"

"..\..\..\..\distrib\lib\core\smtpd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "core_smtpd - Win32 Release"
SOURCE=.\smtpd_auth.c

"$(INTDIR)\smtpd_auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd_bounce.c

"$(INTDIR)\smtpd_bounce.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd_client.c

"$(INTDIR)\smtpd_client.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd_conf.c

"$(INTDIR)\smtpd_conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd_filter.c

"$(INTDIR)\smtpd_filter.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd_main.c

"$(INTDIR)\smtpd_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd_server.c

"$(INTDIR)\smtpd_server.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

