#pragma once
namespace gear
{
namespace mipmap
{
	const char* help_doucumentation = 
R"(GEAR_MIPMAP: Help Documentation:
The GEAR_MIPMAP takes image files and builds mipmapped versions.

-h, -H, -help, -HELP                  : For this help documentation. Optional.
-pause -PAUSE                         : Pauses the program at the end of shader compilation, sets the -h flag. Optional.
-nologo, -NOLOGO                      : Disables copyright message. Optional.
-nooutput, -NOOUTPUT                  : Disables output messages. Optional.
-f:, -F:[filepath]                    : Filepath to a image file to be generated. This argument must be set.
-o:, -O:[directory]                   : Directory for the output image files. Default is the filepath directory.
-levels: -LEVELS:[unsigned int]       : The number of levels to generate. Optional.
-vk, -VK -vulkan -VULKAN              : Use Vulkan for mipmap generation.
-dx12, -DX12, -d3d12 -D3D12           : Use Direct3D 12 for mipmap generation.
)";
}
}