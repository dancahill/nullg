# Microsoft Developer Studio Generated NMAKE File, Based on srv_httpd.dsp
!IF "$(CFG)" == ""
CFG=srv_httpd - Win32 Release
!MESSAGE srv_httpd - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "srv_httpd - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "srv_httpd.mak" CFG="srv_httpd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "srv_httpd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\..\obj\srv_httpd
INTDIR=.\..\..\..\obj\srv_httpd

ALL : "..\..\..\distrib\lib\srv_httpd.dll"


CLEAN :
	-@erase "$(INTDIR)\httpd_auth.obj"
	-@erase "$(INTDIR)\httpd_conf.obj"
	-@erase "$(INTDIR)\httpd_format.obj"
	-@erase "$(INTDIR)\httpd_html.obj"
	-@erase "$(INTDIR)\httpd_http.obj"
	-@erase "$(INTDIR)\httpd_io.obj"
	-@erase "$(INTDIR)\httpd_log.obj"
	-@erase "$(INTDIR)\httpd_main.obj"
	-@erase "$(INTDIR)\httpd_modctl.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\srv_httpd.exp"
	-@erase "..\..\..\distrib\lib\srv_httpd.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srv_httpd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\srv_httpd.pdb" /machine:I386 /out:"..\..\..\distrib\lib\srv_httpd.dll" /implib:"$(OUTDIR)\srv_httpd.lib" 
LINK32_OBJS= \
	"$(INTDIR)\httpd_auth.obj" \
	"$(INTDIR)\httpd_conf.obj" \
	"$(INTDIR)\httpd_format.obj" \
	"$(INTDIR)\httpd_html.obj" \
	"$(INTDIR)\httpd_http.obj" \
	"$(INTDIR)\httpd_io.obj" \
	"$(INTDIR)\httpd_log.obj" \
	"$(INTDIR)\httpd_main.obj" \
	"$(INTDIR)\httpd_modctl.obj"

"..\..\..\distrib\lib\srv_httpd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "srv_httpd - Win32 Release"
SOURCE=.\httpd_auth.c

"$(INTDIR)\httpd_auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_conf.c

"$(INTDIR)\httpd_conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_format.c

"$(INTDIR)\httpd_format.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_html.c

"$(INTDIR)\httpd_html.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_http.c

"$(INTDIR)\httpd_http.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_io.c

"$(INTDIR)\httpd_io.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_log.c

"$(INTDIR)\httpd_log.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_main.c

"$(INTDIR)\httpd_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\httpd_modctl.c

"$(INTDIR)\httpd_modctl.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

