SET SOLUTION_DIR=%1
SET OUT_DIR=%2
IF [%3] == [] (
	SET PLATFORM=x32
) ELSE (
	SET PLATFORM=%3
)

ECHO ON
ECHO Solution Directory: %SOLUTION_DIR% ; Output Directory: %OUT_DIR% ; Platform: %PLATFORM%

copy "%SOLUTION_DIR%\..\dependancies\%PLATFORM%\sdl2\SDL2.dll" "%OUT_DIR%\SDL2.dll" /y /D
copy "%SOLUTION_DIR%\..\dependancies\%PLATFORM%\freeImage\FreeImage.dll" "%OUT_DIR%\FreeImage.dll" /y /D
IF "%PLATFORM%" == "x32" (
	copy "%SOLUTION_DIR%\..\dependancies\%PLATFORM%\assimp\Assimp32.dll" "%OUT_DIR%\Assimp32.dll" /y /D
) ELSE (
	copy "%SOLUTION_DIR%\..\dependancies\%PLATFORM%\assimp\Assuno64.dll" "%OUT_DIR%\Assimp64.dll" /y /D
)

xcopy "%SOLUTION_DIR%\Engine\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOLUTION_DIR%\Engine\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d
xcopy "%SOLUTION_DIR%\Demo\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOLUTION_DIR%\Demo\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d