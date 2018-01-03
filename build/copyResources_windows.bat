SET SOLUTION_DIR=%1
SET OUT_DIR=%2
IF [%3] == [] SET PLATFORM="x32"
ELSE SET PLATFORM=%3

ECHO ON
ECHO Solution Directory: %SOLUTION_DIR% ; Output Directory: %OUT_DIR% ; Platform: %PLATFORM%

xcopy "%SOLUTION_DIR%..\dependancies\%PLATFORM%\sdl2\SDL2.dll" "%OUT_DIR%" /y/D/d
xcopy "%SOLUTION_DIR%..\dependancies\%PLATFORM%\freeImage\FreeImage.dll" "%OUT_DIR%" /y/D/d
xcopy "%SOLUTION_DIR%..\dependancies\%PLATFORM%\assimp\assimp.dll" "%OUT_DIR%" /y/D/d

xcopy "%SOLUTION_DIR%\Engine\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOLUTION_DIR%\Engine\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d
xcopy "%SOLUTION_DIR%\Demo\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOLUTION_DIR%\Demo\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d