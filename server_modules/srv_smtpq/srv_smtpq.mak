# Microsoft Developer Studio Generated NMAKE File, Based on srv_smtpq.dsp
!IF "$(CFG)" == ""
CFG=srv_smtpq - Win32 Release
!MESSAGE srv_smtpq - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "srv_smtpq - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "srv_smtpq.mak" CFG="srv_smtpq - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "srv_smtpq - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\obj\srv_smtpq
INTDIR=.\..\..\obj\srv_smtpq

ALL : "..\..\distrib\lib\srv_smtpq.dll"


CLEAN :
	-@erase "$(INTDIR)\bounce.obj"
	-@erase "$(INTDIR)\conf.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\smtpc.obj"
	-@erase "$(INTDIR)\smtpq.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\srv_smtpq.exp"
	-@erase "..\..\distrib\lib\srv_smtpq.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srv_smtpq.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\srv_smtpq.pdb" /machine:I386 /out:"..\..\distrib\lib\srv_smtpq.dll" /implib:"$(OUTDIR)\srv_smtpq.lib" 
LINK32_OBJS= \
	"$(INTDIR)\bounce.obj" \
	"$(INTDIR)\conf.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\smtpc.obj" \
	"$(INTDIR)\smtpq.obj"

"..\..\distrib\lib\srv_smtpq.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "srv_smtpq - Win32 Release"
SOURCE=.\bounce.c

"$(INTDIR)\bounce.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\conf.c

"$(INTDIR)\conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpc.c

"$(INTDIR)\smtpc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpq.c

"$(INTDIR)\smtpq.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

