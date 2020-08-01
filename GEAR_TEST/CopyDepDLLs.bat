@echo off
set /A debug = %1

if %debug% == 1 (
	copy ".\..\GEAR_CORE\dep\ASSIMP\lib\Debug\assimp-vc142-mtd.dll" ".\exe\x64\Debug\assimp-vc142-mtd.dll"
	copy ".\..\GEAR_CORE\dep\FREETYPE\win64\freetype.dll"			 ".\exe\x64\Debug\freetype.dll"
	copy ".\..\GEAR_CORE\dep\OPENAL\libs\Win64\OpenAL32.dll"		 ".\exe\x64\Debug\OpenAL32.dll"
) else (
	copy ".\..\GEAR_CORE\dep\ASSIMP\lib\Release\assimp-vc142-mt.dll" ".\exe\x64\Release\assimp-vc142-mt.dll"
	copy ".\..\GEAR_CORE\dep\FREETYPE\win64\freetype.dll"			  ".\exe\x64\Release\freetype.dll"
	copy ".\..\GEAR_CORE\dep\OPENAL\libs\Win64\OpenAL32.dll"		  ".\exe\x64\Release\OpenAL32.dll"
)
pause