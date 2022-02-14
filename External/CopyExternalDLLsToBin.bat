set platform=%1
set config=%2

echo Current Directory: %CD%
echo Platform         : %platform%
echo Configuration    : %config%

echo assimp-vc142-mt.dll
copy ".\..\External\ASSIMP\lib\Release\assimp-vc142-mt.dll"	".\..\bin\%platform%\%config%\assimp-vc142-mt.dll"
echo freetype.dll
copy ".\..\External\FREETYPE\win64\freetype.dll"			".\..\bin\%platform%\%config%\freetype.dll"
echo OpenAL32.dll
copy ".\..\External\OPENAL\libs\Win64\OpenAL32.dll"			".\..\bin\%platform%\%config%\OpenAL32.dll"