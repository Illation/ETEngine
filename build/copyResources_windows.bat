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

IF "%COPY_DLLS%" == "true" (
	copy "%SOURCE_DIR%\..\dependancies\%PLATFORM%\sdl2\SDL2.dll" "%OUT_DIR%\SDL2.dll" /y /D
	copy "%SOURCE_DIR%\..\dependancies\%PLATFORM%\freeImage\FreeImage.dll" "%OUT_DIR%\FreeImage.dll" /y /D
	copy "%SOURCE_DIR%\..\dependancies\%PLATFORM%\openAL\OpenAL32.dll" "%OUT_DIR%\OpenAL32.dll" /y /D
	IF "%PLATFORM%" == "x32" (
		copy "%SOURCE_DIR%\..\dependancies\%PLATFORM%\assimp\Assimp32.dll" "%OUT_DIR%\Assimp32.dll" /y /D
	) ELSE (
		copy "%SOURCE_DIR%\..\dependancies\%PLATFORM%\assimp\Assimp64.dll" "%OUT_DIR%\Assimp64.dll" /y /D
	)
)

xcopy "%SOURCE_DIR%\Engine\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOURCE_DIR%\Engine\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d
xcopy "%SOURCE_DIR%\Demo\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOURCE_DIR%\Demo\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d
xcopy "%SOURCE_DIR%\Demo\Config" "%OUT_DIR%" /s/i/y/d

exit