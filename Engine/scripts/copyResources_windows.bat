@ECHO OFF

SET SOURCE_DIR=%1
SET OUT_DIR=%2
IF [%3] == [] (
	SET PLATFORM=x32
) ELSE (
	SET PLATFORM=%3
)
IF [%4] == [] (
	SET COPY_DLLS=false
) ELSE (
	SET COPY_DLLS=%4
)

ECHO Source Directory: %SOURCE_DIR% ; Output Directory: %OUT_DIR% ; Platform: %PLATFORM% ; copy_dlls? %COPY_DLLS%

xcopy "%SOURCE_DIR%\Demo\Config" "%OUT_DIR%" /s/i/y/d

exit