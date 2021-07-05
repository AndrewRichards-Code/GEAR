set platform=%1
set config=%2

echo Platform      : %platform%
echo Configuration : %config%

copy ".\..\GEAR_CORE\dep\ASSIMP\lib\"%config%"\assimp-vc142-mtd.dll"  ".\exe\"%platform%"\"%config%"\assimp-vc142-mtd.dll"
copy ".\..\GEAR_CORE\dep\FREETYPE\win64\freetype.dll"			      ".\exe\"%platform%"\"%config%"\freetype.dll"
copy ".\..\GEAR_CORE\dep\OPENAL\libs\Win64\OpenAL32.dll"		      ".\exe\"%platform%"\"%config%"\OpenAL32.dll"

pause