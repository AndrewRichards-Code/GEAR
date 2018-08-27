#pragma once

#include <vector>

#include "GL/glew.h"
#include "../utils/fileutils.h"

#define GEAR_CALC_LIGHT_DIFFUSE  0x00000001
#define GEAR_CALC_LIGHT_SPECULAR 0x00000010
#define GEAR_CALC_LIGHT_AMIBIENT 0x00000100
#define GEAR_CALC_LIGHT_EMIT     0x00001000

namespace GEAR {
namespace GRAPHICS {

class Shader
{
private:
	GLuint m_ShaderID;
	const char* m_VertexPath;
	const char* m_FragmentPath;
	std::string m_VertexCode;
	std::string m_FragmentCode;

public:
	Shader(const char* vertexPath, const char* fragmentPath);
	~Shader();

	void Enable() const;
	void Disable() const;

	void SetLighting(int types) const;

	template<typename T>
	inline void SetUniform(const char* locationInput, const std::vector<T>& values) const
	{
		GLint location = glGetUniformLocation(m_ShaderID, locationInput);
		if (typeid(T) == typeid(float))
		{
				 if (values.size() == 1) glUniform1f(location, (float)values[0]);
			else if (values.size() == 2) glUniform2f(location, (float)values[0], (float)values[1]);
			else if (values.size() == 3) glUniform3f(location, (float)values[0], (float)values[1], (float)values[2]);
			else if (values.size() == 4) glUniform4f(location, (float)values[0], (float)values[1], (float)values[2], (float)values[3]);
			else throw;
		}
		else if (typeid(T) == typeid(int)) 
		{
				 if (values.size() == 1) glUniform1i(location, (int)values[0]);
			else if (values.size() == 2) glUniform2i(location, (int)values[0], (int)values[1]);
			else if (values.size() == 3) glUniform3i(location, (int)values[0], (int)values[1], (int)values[2]);
			else if (values.size() == 4) glUniform4i(location, (int)values[0], (int)values[1], (int)values[2], (int)values[3]);
			else throw;
		}
		else if (typeid(T) == typeid(unsigned int)) 
		{
				 if (values.size() == 1) glUniform1ui(location, (unsigned int)values[0]);
			else if (values.size() == 2) glUniform2ui(location, (unsigned int)values[0], (unsigned int)values[1]);
			else if (values.size() == 3) glUniform3ui(location, (unsigned int)values[0], (unsigned int)values[1], (unsigned int)values[2]);
			else if (values.size() == 4) glUniform4ui(location, (unsigned int)values[0], (unsigned int)values[1], (unsigned int)values[2], (unsigned int)values[3]);
			else throw;
		}
		else throw;
	}

	template<typename T>
	inline void SetUniformArray(const char* locationInput, const int size, const GLsizei& count, const T& values) const
	{
		GLint location = glGetUniformLocation(m_ShaderID, locationInput);
		if (typeid(T) == typeid(float))
		{
				 if (size == 1) glUniform1fv(location, count, &values);
			else if (size == 2) glUniform2fv(location, count, &values);
			else if (size == 3) glUniform3fv(location, count, &values);
			else if (size == 4) glUniform4fv(location, count, &values);
			else throw;
		}
		else if (typeid(T) == typeid(int))
		{
				 if (size == 1) glUniform1iv(location, count, &values);
			else if (size == 2) glUniform2iv(location, count, &values);
			else if (size == 3) glUniform3iv(location, count, &values);
			else if (size == 4) glUniform4iv(location, count, &values);
			else throw;
		}
		else if (typeid(T) == typeid(unsigned int))
		{
				 if (size == 1) glUniform1uiv(location, count, &values);
			else if (size == 2) glUniform2uiv(location, count, &values);
			else if (size == 3) glUniform3uiv(location, count, &values);
			else if (size == 4) glUniform4uiv(location, count, &values);
			else throw;
		}
		else throw;
	}

	template<int N>
	inline void SetUniformMatrix(const char* locationInput, const GLsizei& count, GLboolean tranpose, const float& values) const
	{
		GLint location = glGetUniformLocation(m_ShaderID, locationInput);
		     if (N == 2) glUniformMatrix2fv(location, count, tranpose, &values);
		else if (N == 3) glUniformMatrix3fv(location, count, tranpose, &values);
		else if (N == 4) glUniformMatrix4fv(location, count, tranpose, &values);
		else throw;
	}

private:
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexshader, const std::string& fragmentshader);
};
}
}