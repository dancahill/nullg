@ECHO OFF
fd ..\distrib\*.* /D /T:01:02:05 /S
fd output\*.* /D /T:01:02:05 /S
pkzip -!rpaex output\nullgw.zip output\setup.exe
move output\nullgw.zip output\nullgroupware-1.2.5-x86-win32.zip
