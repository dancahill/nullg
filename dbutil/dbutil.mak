# Microsoft Developer Studio Generated NMAKE File, Based on dbutil.dsp
!IF "$(CFG)" == ""
CFG=dbutil - Win32 Release
!MESSAGE No configuration specified. Defaulting to dbutil - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "dbutil - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbutil.mak" CFG="dbutil - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbutil - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe
OUTDIR=.\..\distrib\bin
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\dbutil.exe"


CLEAN :
	-@erase "$(INTDIR)\dbio.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mdb.obj"
	-@erase "$(INTDIR)\sql.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dbutil.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbutil.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"$(OUTDIR)\dbutil.exe" /libpath:"../win/lib" 
LINK32_OBJS= \
	"$(INTDIR)\dbio.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\mdb.obj" \
	"$(INTDIR)\sql.obj"

"$(OUTDIR)\dbutil.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../groupware/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dbutil.dep")
!INCLUDE "dbutil.dep"
!ELSE 
!MESSAGE Warning: cannot find "dbutil.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dbutil - Win32 Release"
SOURCE=.\dbio.c

"$(INTDIR)\dbio.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mdb.c

"$(INTDIR)\mdb.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sql.c

"$(INTDIR)\sql.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

