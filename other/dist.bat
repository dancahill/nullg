@ECHO OFF
fd ..\distrib\*.* /D /T:01:02:02 /S
fd output\*.* /D /T:01:02:02 /S
pkzip -!rpaex output\nullgw.zip output\setup.exe
move output\nullgw.zip output\nullgroupware-1.2.2-x86-win32.zip
