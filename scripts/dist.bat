@ECHO OFF
fd ..\distrib\*.* /D /T:01:03:00 /S
fd output\*.* /D /T:01:03:00 /S
pkzip -!rpaex output\nullgw.zip output\setup.exe
move output\nullgw.zip output\nullgroupware-1.3.7-x86-win32.zip
