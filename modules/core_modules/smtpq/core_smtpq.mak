# Microsoft Developer Studio Generated NMAKE File, Based on core_smtpq.dsp
!IF "$(CFG)" == ""
CFG=core_smtpq - Win32 Release
!MESSAGE core_smtpq - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "core_smtpq - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core_smtpq.mak" CFG="core_smtpq - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core_smtpq - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\..\..\..\obj\core_modules\core_smtpq
INTDIR=.\..\..\..\obj\core_modules\core_smtpq

ALL : "..\..\..\distrib\lib\core\smtpq.dll"


CLEAN :
	-@erase "$(INTDIR)\smtpq_bounce.obj"
	-@erase "$(INTDIR)\smtpq_client.obj"
	-@erase "$(INTDIR)\smtpq_conf.obj"
	-@erase "$(INTDIR)\smtpq_main.obj"
	-@erase "$(INTDIR)\smtpq_queue.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\core_smtpq.exp"
	-@erase "..\..\..\distrib\lib\core\smtpq.dll"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\core_smtpq.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\core_smtpq.pdb" /machine:I386 /out:"..\..\..\distrib\lib\core\smtpq.dll" /implib:"$(OUTDIR)\core_smtpq.lib" 
LINK32_OBJS= \
	"$(INTDIR)\smtpq_bounce.obj" \
	"$(INTDIR)\smtpq_client.obj" \
	"$(INTDIR)\smtpq_conf.obj" \
	"$(INTDIR)\smtpq_main.obj" \
	"$(INTDIR)\smtpq_queue.obj"

"..\..\..\distrib\lib\core\smtpq.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(CFG)" == "core_smtpq - Win32 Release"
SOURCE=.\smtpq_bounce.c

"$(INTDIR)\smtpq_bounce.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpq_client.c

"$(INTDIR)\smtpq_client.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpq_conf.c

"$(INTDIR)\smtpq_conf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpq_main.c

"$(INTDIR)\smtpq_main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\smtpq_queue.c

"$(INTDIR)\smtpq_queue.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

