# Microsoft Developer Studio Generated NMAKE File, Based on srv_smtpd.dsp
!IF "$(CFG)" == ""
CFG=srv_smtpd - Win32 Release
!MESSAGE srv_smtpd - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "srv_smtpd - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "srv_smtpd.mak" CFG="srv_smtpd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "srv_smtpd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\obj\srv_smtpd
INTDIR=.\..\..\obj\srv_smtpd

ALL : "..\..\distrib\lib\srv_smtpd.dll"


CLEAN :
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\conf.obj"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\smtpc.obj"
	-@erase "$(INTDIR)\smtpd.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\srv_smtpd.exp"
	-@erase "..\..\distrib\lib\srv_smtpd.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srv_smtpd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\srv_smtpd.pdb" /machine:I386 /out:"..\..\distrib\lib\srv_smtpd.dll" /implib:"$(OUTDIR)\srv_smtpd.lib" 
LINK32_OBJS= \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\conf.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\smtpc.obj" \
	"$(INTDIR)\smtpd.obj"

"..\..\distrib\lib\srv_smtpd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "srv_smtpd - Win32 Release"
SOURCE=.\auth.c

"$(INTDIR)\auth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\conf.c

"$(INTDIR)\conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filter.c

"$(INTDIR)\filter.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpc.c

"$(INTDIR)\smtpc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpd.c

"$(INTDIR)\smtpd.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

