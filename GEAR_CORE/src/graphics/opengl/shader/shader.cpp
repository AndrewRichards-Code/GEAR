#include "shader.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

bool Shader::s_InitialiseUBO = false;

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, bool useBinaries)
	:m_VertexPath(vertexPath), m_FragmentPath(fragmentPath)
{
	bool isBinaryVert = IsBinaryFile(vertexPath);
	bool isBinaryFrag = IsBinaryFile(fragmentPath);
	std::string m_TempVertexPath = AmmendFilePath(m_VertexPath, isBinaryVert, useBinaries);
	std::string m_TempFragmentPath = AmmendFilePath(m_FragmentPath, isBinaryFrag, useBinaries);
	m_VertexPath = m_TempVertexPath;
	m_FragmentPath = m_TempFragmentPath;

	/*int NumberOfExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
	std::vector<std::string> ext(NumberOfExtensions);
	for (int i = 0; i < NumberOfExtensions; i++) 
		ext[i] = (const char*)glGetStringi(GL_EXTENSIONS, i);*/

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
	for (auto& id : m_ShaderModuleIDs)
	{
		glDeleteShader(id);
	}
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

void Shader::AddAdditionalShaderModule(unsigned int type, const std::string& shaderPath, bool useBinaries)
{
	bool isBinaryVert = IsBinaryFile(shaderPath);
	std::string m_ShaderPath = AmmendFilePath(shaderPath, isBinaryVert, useBinaries);

	unsigned int m_ShaderModuleID;
	if (useBinaries && GLEW_ARB_gl_spirv && GLEW_ARB_spirv_extensions)
	{
		std::vector<char>m_ShaderBin = FileUtils::read_binary(m_ShaderPath.c_str());
		m_ShaderModuleID = CompileShader(type, m_ShaderBin);
	}
	else
	{
		std::string m_ShaderCode = FileUtils::read_file(shaderPath);
		m_ShaderModuleID = CompileShader(type, m_ShaderCode);
	}
	m_ShaderModuleIDs.push_back(m_ShaderModuleID);
}

void Shader::FinalisePipline()
{
	for (auto& id : m_ShaderModuleIDs)
	{
		glAttachShader(m_ShaderID, id);
	}
	glLinkProgram(m_ShaderID);
	glValidateProgram(m_ShaderID);
}

void Shader::SetLighting(int types)
{
	m_SetLightingUBO.m_Diffuse  = static_cast<float>((types & GEAR_CALC_LIGHT_DIFFUSE) != 0);
	m_SetLightingUBO.m_Specular = static_cast<float>((types & GEAR_CALC_LIGHT_SPECULAR) != 0);
	m_SetLightingUBO.m_Ambient  = static_cast<float>((types & GEAR_CALC_LIGHT_AMBIENT) != 0);
	m_SetLightingUBO.m_Emit     = static_cast<float>((types & GEAR_CALC_LIGHT_EMIT) != 0);
		
	if (s_InitialiseUBO == false)
	{
		BufferManager::AddUBO(sizeof(SetLightingUBO), 3);
		s_InitialiseUBO = true;
	}
	BufferManager::UpdateUBO(3, &m_SetLightingUBO.m_Diffuse, sizeof(SetLightingUBO), 0);
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
			(type == GL_VERTEX_SHADER ? "vertex" : GL_FRAGMENT_SHADER ? "fragment" : GL_GEOMETRY_SHADER ? "geometry" : GL_TESS_CONTROL_SHADER ? "Tessellation Control" : GL_TESS_EVALUATION_SHADER ? "Tessellation Evaluation" : "UNKNOWN" ) << " shader!" << std::endl;
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
	m_ShaderModuleIDs.push_back(vs);
	m_ShaderModuleIDs.push_back(fs);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	return program;
}

//Binary Loader

bool Shader::IsBinaryFile(const std::string& filePath)
{
	std::string file = filePath;
	if (file.find(".spv") != std::string::npos)
		return true;
	else
		return false;
}

std::string Shader::AmmendFilePath(const std::string& filePath, bool fileIsBinary, bool shaderWantsBinary)
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

unsigned int Shader::CompileShader(unsigned int type, const std::vector<char>& source)
{
	unsigned int id = glCreateShader(type);
	glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, source.data(), source.size());
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
			(type == GL_VERTEX_SHADER ? "vertex" : GL_FRAGMENT_SHADER ? "fragment" : GL_GEOMETRY_SHADER ? "geometry" : GL_TESS_CONTROL_SHADER ? "Tessellation Control" : GL_TESS_EVALUATION_SHADER ? "Tessellation Evaluation" : "UNKNOWN") << " shader from binary!" << std::endl;
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
	m_ShaderModuleIDs.push_back(vs);
	m_ShaderModuleIDs.push_back(fs);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glBindAttribLocation(program, (GLuint)VertexArray::BufferType::GEAR_BUFFER_POSITIONS, "positions");
	glBindAttribLocation(program, (GLuint)VertexArray::BufferType::GEAR_BUFFER_TEXTCOORDS, "textCoords");
	glBindAttribLocation(program, (GLuint)VertexArray::BufferType::GEAR_BUFFER_TEXTIDS, "textIds");
	glBindAttribLocation(program, (GLuint)VertexArray::BufferType::GEAR_BUFFER_NORMALS, "normals");
	glBindAttribLocation(program, (GLuint)VertexArray::BufferType::GEAR_BUFFER_COLOUR, "colours");

	return program;
}
