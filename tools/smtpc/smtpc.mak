# Microsoft Developer Studio Generated NMAKE File, Based on smtpc.dsp
!IF "$(CFG)" == ""
CFG=smtpc - Win32 Release
!MESSAGE No configuration specified. Defaulting to smtpc - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "smtpc - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "smtpc.mak" CFG="smtpc - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "smtpc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\distrib\bin
INTDIR=.\..\..\obj\smtpc
# Begin Custom Macros
OutDir=.\..\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\nullgw-smtpc.exe"


CLEAN :
	-@erase "$(INTDIR)\dns.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\smtpc.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\nullgw-smtpc.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\smtpc.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\nullgw-smtpc.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dns.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\smtpc.obj"

"$(OUTDIR)\nullgw-smtpc.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("smtpc.dep")
!INCLUDE "smtpc.dep"
!ELSE 
!MESSAGE Warning: cannot find "smtpc.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "smtpc - Win32 Release"
SOURCE=.\dns.c

"$(INTDIR)\dns.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpc.c

"$(INTDIR)\smtpc.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

