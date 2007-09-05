@ECHO OFF
fd ..\distrib\*.* /D /T:01:03:00 /S
"C:\Program Files\Inno Setup 5\Compil32.exe" /cc distrib.iss
fd output\*.* /D /T:01:03:00 /S
pkzip -!rpaex output\nullgs.zip output\setup.exe
move output\nullgs.zip output\nullgs-1.3.30-x86-win32.zip
fd output\*.* /D /T:01:03:00 /S
