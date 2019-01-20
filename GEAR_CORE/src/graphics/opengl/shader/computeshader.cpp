#include "shader.h"
#include "computeshader.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

ComputeShader::ComputeShader(const char* computePath, bool useBinaries)
	:m_ComputePath(computePath) 
{
	bool isBinary = IsBinaryFile(computePath);
	std::string m_TempComputePath = AmmendFilePath(m_ComputePath, isBinary, useBinaries);
	m_ComputePath = m_TempComputePath.c_str();

	if (useBinaries && GLEW_ARB_gl_spirv && GLEW_ARB_spirv_extensions)
	{
		m_ComputeBin = FileUtils::read_binary(m_ComputePath);
		m_ComputeShaderID = CreateShader(m_ComputeBin);
	}
	else
	{
		m_ComputePath = computePath;
		m_ComputeCode = FileUtils::read_file(m_ComputePath);
		m_ComputeShaderID = CreateShader(m_ComputeCode);
	}
}

ComputeShader::~ComputeShader()
{
	glDeleteProgram(m_ComputeShaderID);
}

void ComputeShader::Enable() const
{
	glUseProgram(m_ComputeShaderID);
}

void ComputeShader::Disable() const
{
	glUseProgram(0);
}

unsigned int ComputeShader::CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Failed to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

unsigned int ComputeShader::CreateShader(const std::string& computeshader)
{
	unsigned int program = glCreateProgram();
	unsigned int cs = ComputeShader::CompileShader(GL_COMPUTE_SHADER, computeshader);

	glAttachShader(program, cs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(cs);

	return program;
}

//Binary Loader
bool ComputeShader::IsBinaryFile(const char* filePath)
{
	std::string file = filePath;
	if (file.find(".spv") != std::string::npos)
		return true;
	else
		return false;
}

std::string ComputeShader::AmmendFilePath(const char* filePath, bool fileIsBinary, bool shaderWantsBinary)
{
#ifdef GEAR_OPENGL
std::string folder("SPIR-V_GL");
#elif GEAR_VULKAN
std::string folder("SPIR-V_VK");
#endif
	if (fileIsBinary == shaderWantsBinary)
	{
		return filePath;
	}

	else if (!fileIsBinary && shaderWantsBinary)
	{
		std::string file = filePath;
		std::size_t pos = file.find("GLSL");
		if (pos != std::string::npos)
		{
			file.erase(pos, 4);
			file.insert(pos, folder);
			file.append(".spv");
			return file;
		}
		else
		{
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Can not ammend file path!" << std::endl;
			return"";
		}
	}

	else if (fileIsBinary && !shaderWantsBinary)
	{
		std::string file = filePath;
		std::size_t pos = file.find(folder);
		if (pos != std::string::npos)
		{
			file.erase(pos, folder.size());
			file.insert(pos, "GLSL");
			std::size_t pos2 = file.find(".spv");
			if (pos2 != std::string::npos)
			{
				file.erase(pos2, 4);
				return file;
			}
			else
			{
				std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Can not ammend file path!" << std::endl;
				return"";
			}
		}
		else
		{
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Can not ammend file path!" << std::endl;
			return"";
		}
	}
	else
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Can not ammend file path!" << std::endl;
		return"";
	}
}

unsigned int ComputeShader::CompileShader(unsigned int type, const std::vector<char>& source)
{
	unsigned int id = glCreateShader(type);
	glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, source.data(), source.size());
	glSpecializeShader(id, "main", 0, nullptr, nullptr);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Failed to compile " <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader from binary!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

unsigned int ComputeShader::CreateShader(const std::vector<char>& vertexshader)
{
	unsigned int program = glCreateProgram();
	unsigned int cs = ComputeShader::CompileShader(GL_COMPUTE_SHADER, vertexshader);

	glAttachShader(program, cs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(cs);

	return program;
}
