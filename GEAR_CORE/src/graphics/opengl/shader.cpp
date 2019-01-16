#include "shader.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

std::vector<std::unique_ptr<UniformBuffer>> Shader::m_UBO;
bool Shader::s_InitialiseUBO = false;

Shader::Shader(const char* vertexPath, const char* fragmentPath, bool useBinaries)
	:m_VertexPath(vertexPath), m_FragmentPath(fragmentPath)
{
	bool isBinaryVert = IsBinaryFile(vertexPath);
	bool isBinaryFrag = IsBinaryFile(fragmentPath);
	std::string m_TempVertexPath = AmmendFilePath(m_VertexPath, isBinaryVert, useBinaries);
	std::string m_TempFragmentPath = AmmendFilePath(m_FragmentPath, isBinaryFrag, useBinaries);
	m_VertexPath = m_TempVertexPath.c_str();
	m_FragmentPath = m_TempFragmentPath.c_str();

	if(useBinaries && GLEW_ARB_gl_spirv && GLEW_ARB_spirv_extensions)
	{
		m_VertexBin = FileUtils::read_binary(m_VertexPath);
		m_FragmentBin = FileUtils::read_binary(m_FragmentPath);
		m_ShaderID = CreateShader(m_VertexBin, m_FragmentBin);
	}
	else
	{
		m_VertexPath = vertexPath;
		m_FragmentPath = fragmentPath;
		m_VertexCode = FileUtils::read_file(m_VertexPath);
		m_FragmentCode = FileUtils::read_file(m_FragmentPath);
		m_ShaderID = CreateShader(m_VertexCode, m_FragmentCode);
	}
}
Shader::~Shader()
{
	glDeleteProgram(m_ShaderID);
}
void Shader::Enable() const
{
	glUseProgram(m_ShaderID);
}
void Shader::Disable() const
{
	glUseProgram(0);
}

void Shader::AddUBO(unsigned int size, unsigned int bindingIndex)
{
	m_UBO.emplace_back(std::make_unique<UniformBuffer>(size, bindingIndex));
}

void Shader::UpdateUBO(unsigned int bindingIndex, const float* data, unsigned int size, unsigned int offset)const
{
	unsigned int index = -1;
	for (unsigned int i = 0; i < m_UBO.size(); i++)
	{
		if (m_UBO[i]->GetBindingIndex() == bindingIndex)
		{
			index = i;
			break;
		}
	}
	if(index == -1)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::Shader: Failed to find UBO at Binding Index " << bindingIndex << "." << std::endl;
		return;
	}

	m_UBO[index]->SubDataBind(data, size, offset);
}

void Shader::SetLighting(int types)
{
	m_SetLightingUBO.m_Diffuse  = static_cast<float>((types & 0x0000000f) >> 0);
	m_SetLightingUBO.m_Specular = static_cast<float>((types & 0x000000f0) >> 4);
	m_SetLightingUBO.m_Ambient  = static_cast<float>((types & 0x00000f00) >> 8);
	m_SetLightingUBO.m_Emit     = static_cast<float>((types & 0x0000f000) >> 12);
		
	if (s_InitialiseUBO == false)
	{
		AddUBO(sizeof(SetLightingUBO), 3);
		s_InitialiseUBO = true;
	}
	UpdateUBO(3, &m_SetLightingUBO.m_Diffuse, sizeof(SetLightingUBO), 0);
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
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

unsigned int Shader::CreateShader(const std::string& vertexshader, const std::string& fragmentshader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = Shader::CompileShader(GL_VERTEX_SHADER, vertexshader);
	unsigned int fs = Shader::CompileShader(GL_FRAGMENT_SHADER, fragmentshader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

//Binary Loader

bool Shader::IsBinaryFile(const char* filePath)
{
	std::string file = filePath;
	if (file.find(".spv") != std::string::npos)
		return true;
	else
		return false;
}

#ifdef GEAR_OPENGL
		std::string folder("SPIR-V_GL");
#elif GEAR_VULKAN
		std::string folder("SPIR-V_VK");
#endif
std::string Shader::AmmendFilePath(const char* filePath, bool fileIsBinary, bool shaderWantsBinary)
{
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

unsigned int Shader::CompileShader(unsigned int type, const std::vector<char>& source)
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

unsigned int Shader::CreateShader(const std::vector<char>& vertexshader, const std::vector<char>& fragmentshader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = Shader::CompileShader(GL_VERTEX_SHADER, vertexshader);
	unsigned int fs = Shader::CompileShader(GL_FRAGMENT_SHADER, fragmentshader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glBindAttribLocation(program, GEAR_BUFFER_POSITIONS, "positions");
	glBindAttribLocation(program, GEAR_BUFFER_TEXTCOORDS, "textCoords");
	glBindAttribLocation(program, GEAR_BUFFER_TEXTIDS, "textIds");
	glBindAttribLocation(program, GEAR_BUFFER_NORMALS, "normals");
	glBindAttribLocation(program, GEAR_BUFFER_COLOUR, "colours");


	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}
