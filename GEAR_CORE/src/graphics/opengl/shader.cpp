#include "shader.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

Shader::Shader(const char * vertexPath, const char * fragmentPath)
	:m_VertexPath(vertexPath), m_FragmentPath(fragmentPath)
{
	m_VertexCode = FileUtils::read_file(m_VertexPath);
	m_FragmentCode = FileUtils::read_file(m_FragmentPath);
	m_ShaderID = CreateShader(m_VertexCode, m_FragmentCode);
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

void Shader::SetLighting(int types) const
{
	int diffuse, specular, ambient, emit;
	diffuse = (types & 0x0000000f);
	specular = (types & 0x000000f0) >> 4;
	ambient = (types & 0x00000f00) >> 8;
	emit = (types & 0x0000f000) >> 12;
	int setLighting[] = { diffuse, specular, ambient, emit };

	Enable();
	SetUniformArray<int>("u_SetLighting", 1, 4, setLighting);
	Disable();
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
