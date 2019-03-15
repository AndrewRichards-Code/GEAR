#pragma once

#include "gear_common.h"

#include "GL/glew.h"
#include "utils/fileutils.h"


namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {

class ComputeShader
{
private:
	GLuint m_ComputeShaderID;
	const char* m_ComputePath;
	std::string m_ComputeCode;
	std::vector<char> m_ComputeBin;

public:
	ComputeShader(const char* computePath, bool useBinaries = false);
	~ComputeShader();

	void Enable() const;
	void Disable() const;

	inline GLuint GetShaderID() const { return m_ComputeShaderID; }

private:
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& computeshader);

	//Binary Loader
	bool IsBinaryFile(const char * input);
	std::string AmmendFilePath(const char* filePath, bool fileIsBinary, bool shaderWantsBinary);
	unsigned int CompileShader(unsigned int type, const std::vector<char>& source);
	unsigned int CreateShader(const std::vector<char>& computeshader);
};
}
}
}