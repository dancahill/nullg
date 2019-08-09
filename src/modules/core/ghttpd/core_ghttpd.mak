# Microsoft Developer Studio Generated NMAKE File, Based on core_ghttpd.dsp
!IF "$(CFG)" == ""
CFG=core_ghttpd - Win32 Release
!MESSAGE core_ghttpd - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "core_ghttpd - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core_ghttpd.mak" CFG="core_ghttpd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core_ghttpd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=..\..\..\..\obj\modules\core\ghttpd
INTDIR=..\..\..\..\obj\modules\core\ghttpd

ALL : "..\..\..\..\distrib\lib\core\ghttpd.dll"


CLEAN :
	-@erase "$(INTDIR)\ghttpd_auth.obj"
	-@erase "$(INTDIR)\ghttpd_conf.obj"
	-@erase "$(INTDIR)\ghttpd_format.obj"
	-@erase "$(INTDIR)\ghttpd_html.obj"
	-@erase "$(INTDIR)\ghttpd_http.obj"
	-@erase "$(INTDIR)\ghttpd_io.obj"
	-@erase "$(INTDIR)\ghttpd_lang.obj"
	-@erase "$(INTDIR)\ghttpd_log.obj"
	-@erase "$(INTDIR)\ghttpd_main.obj"
	-@erase "$(INTDIR)\ghttpd_modctl.obj"
	-@erase "$(INTDIR)\ghttpd_sql.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\core_ghttpd.exp"
	-@erase "..\..\..\..\distrib\lib\core\ghttpd.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\core_ghttpd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /dll /incremental:no /pdb:"$(OUTDIR)\core_ghttpd.pdb" /machine:I386 /out:"..\..\..\..\distrib\lib\core\ghttpd.dll" /implib:"$(OUTDIR)\core_ghttpd.lib" 
LINK32_OBJS= \
	"$(INTDIR)\ghttpd_auth.obj" \
	"$(INTDIR)\ghttpd_conf.obj" \
	"$(INTDIR)\ghttpd_format.obj" \
	"$(INTDIR)\ghttpd_html.obj" \
	"$(INTDIR)\ghttpd_http.obj" \
	"$(INTDIR)\ghttpd_io.obj" \
	"$(INTDIR)\ghttpd_lang.obj" \
	"$(INTDIR)\ghttpd_log.obj" \
	"$(INTDIR)\ghttpd_main.obj" \
	"$(INTDIR)\ghttpd_modctl.obj" \
	"$(INTDIR)\ghttpd_sql.obj"

"..\..\..\..\distrib\lib\core\ghttpd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "core_ghttpd - Win32 Release"
SOURCE=.\ghttpd_auth.c

"$(INTDIR)\ghttpd_auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_conf.c

"$(INTDIR)\ghttpd_conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_format.c

"$(INTDIR)\ghttpd_format.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_html.c

"$(INTDIR)\ghttpd_html.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_http.c

"$(INTDIR)\ghttpd_http.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_io.c

"$(INTDIR)\ghttpd_io.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_lang.c

"$(INTDIR)\ghttpd_lang.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_log.c

"$(INTDIR)\ghttpd_log.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_main.c

"$(INTDIR)\ghttpd_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_modctl.c

"$(INTDIR)\ghttpd_modctl.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ghttpd_sql.c

"$(INTDIR)\ghttpd_sql.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

