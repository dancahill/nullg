# Microsoft Developer Studio Generated NMAKE File, Based on sentinel.dsp
!IF "$(CFG)" == ""
CFG=Sentinel - Win32 Release
!MESSAGE No configuration specified. Defaulting to Sentinel - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "Sentinel - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sentinel.mak" CFG="Sentinel - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sentinel - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
OUTDIR=.\..\..\distrib\bin
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\..\..\distrib\bin
# End Custom Macros

ALL : "$(OUTDIR)\sentinel.exe"


CLEAN :
	-@erase "$(INTDIR)\admin.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\companies.obj"
	-@erase "$(INTDIR)\contacts.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\forums.obj"
	-@erase "$(INTDIR)\html.obj"
	-@erase "$(INTDIR)\http.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\messages.obj"
	-@erase "$(INTDIR)\orders.obj"
	-@erase "$(INTDIR)\products.obj"
	-@erase "$(INTDIR)\queries.obj"
	-@erase "$(INTDIR)\security.obj"
	-@erase "$(INTDIR)\Sentinel.res"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\sql.obj"
	-@erase "$(INTDIR)\tasks.obj"
	-@erase "$(INTDIR)\users.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\webmail.obj"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(OUTDIR)\sentinel.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sentinel.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"libc" /out:"$(OUTDIR)\sentinel.exe" /libpath:"../win/lib" 
LINK32_OBJS= \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\html.obj" \
	"$(INTDIR)\http.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\security.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\sql.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\admin.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\companies.obj" \
	"$(INTDIR)\contacts.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\forums.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\messages.obj" \
	"$(INTDIR)\orders.obj" \
	"$(INTDIR)\products.obj" \
	"$(INTDIR)\queries.obj" \
	"$(INTDIR)\tasks.obj" \
	"$(INTDIR)\users.obj" \
	"$(INTDIR)\webmail.obj" \
	"$(INTDIR)\Sentinel.res"

"$(OUTDIR)\sentinel.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../win/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Sentinel.res" /d "NDEBUG" 

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("sentinel.dep")
!INCLUDE "sentinel.dep"
!ELSE 
!MESSAGE Warning: cannot find "sentinel.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Sentinel - Win32 Release"
SOURCE=.\common.c

"$(INTDIR)\common.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\html.c

"$(INTDIR)\html.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\http.c

"$(INTDIR)\http.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\md5.c

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\security.c

"$(INTDIR)\security.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\server.c

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sql.c

"$(INTDIR)\sql.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\win32.c

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\admin.c

"$(INTDIR)\admin.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\calendar.c

"$(INTDIR)\calendar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\companies.c

"$(INTDIR)\companies.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\contacts.c

"$(INTDIR)\contacts.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\files.c

"$(INTDIR)\files.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\forums.c

"$(INTDIR)\forums.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\links.c

"$(INTDIR)\links.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\messages.c

"$(INTDIR)\messages.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\orders.c

"$(INTDIR)\orders.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\products.c

"$(INTDIR)\products.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\queries.c

"$(INTDIR)\queries.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tasks.c

"$(INTDIR)\tasks.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\users.c

"$(INTDIR)\users.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\webmail.c

"$(INTDIR)\webmail.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\win\Sentinel.rc

"$(INTDIR)\Sentinel.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\Sentinel.res" /i "\NullLogic\nullgroupware\archives\releases2\nullgroupware-1.0.0-src\nullgroupware-1.0.0\source\win" /d "NDEBUG" $(SOURCE)



!ENDIF 

