@ECHO OFF

SET PATH_TO_CLAMAV="c:\cygwin\bin\clamscan.exe"
SET PATH_TO_MKS="c:\mks\mksscan.exe"

REM ClamAV
if exist "%PATH_TO_CLAMAV%" (
	"%PATH_TO_CLAMAV%" %1 %2 %3 %4 %5
	if ERRORLEVEL 1 (
		exit 2
	)
)

REM MKS Antivirus
if exist "%PATH_TO_MKS%" (
	"%PATH_TO_MKS%" %1 %2 %3 %4 %5
	if ERRORLEVEL 1 (
		exit 2
	)
)

exit 0
