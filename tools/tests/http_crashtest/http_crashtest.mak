# Microsoft Developer Studio Generated NMAKE File, Based on http_crashtest.dsp
!IF "$(CFG)" == ""
CFG=http_crashtest - Win32 Release
!MESSAGE No configuration specified. Defaulting to http_crashtest - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "http_crashtest - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "http_crashtest.mak" CFG="http_crashtest - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "http_crashtest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\http_crashtest.exe"


CLEAN :
	-@erase "$(INTDIR)\http_crashtest.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\http_crashtest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\http_crashtest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\http_crashtest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\http_crashtest.obj"

"$(OUTDIR)\http_crashtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("http_crashtest.dep")
!INCLUDE "http_crashtest.dep"
!ELSE 
!MESSAGE Warning: cannot find "http_crashtest.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "http_crashtest - Win32 Release"
SOURCE=.\http_crashtest.c

"$(INTDIR)\http_crashtest.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

