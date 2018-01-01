SET SOLUTION_DIR=%1
SET OUT_DIR=%2

ECHO ON
ECHO Solution Directory: %SOLUTION_DIR% ; Output Directory: %OUT_DIR%

xcopy "%SOLUTION_DIR%..\dependancies\x32\sdl2\SDL2.dll" "%OUT_DIR%" /y/D/d
xcopy "%SOLUTION_DIR%..\dependancies\x32\freeImage\FreeImage.dll" "%OUT_DIR%" /y/D/d
xcopy "%SOLUTION_DIR%\Engine\StaticDependancies\Assimp32.dll" "%OUT_DIR%" /y/D/d

xcopy "%SOLUTION_DIR%\Engine\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOLUTION_DIR%\Engine\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d
xcopy "%SOLUTION_DIR%\Demo\Resources" "%OUT_DIR%\Resources" /s/i/y/d
xcopy "%SOLUTION_DIR%\Demo\Shaders" "%OUT_DIR%\Shaders" /s/i/y/d