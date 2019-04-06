#pragma once

#include "gear_common.h"

#include "graphics/opengl/buffer/vertexarray.h"
#include "graphics/opengl/buffer/buffermanager.h"
#include "utils/fileutils.h"


namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {

class Shader
{
private:
	GLuint m_ShaderID;
	std::vector<unsigned int>m_ShaderModuleIDs;
	
	std::string m_VertexPath;
	std::string m_FragmentPath;
	std::string m_VertexCode;
	std::string m_FragmentCode;
	std::vector<char> m_VertexBin;
	std::vector<char> m_FragmentBin;

	static bool s_InitialiseUBO;
	struct SetLightingUBO
	{
		float m_Diffuse;
		float m_Specular;
		float m_Ambient;
		float m_Emit;
	}m_SetLightingUBO;

public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath, bool useBinaries = false);
	~Shader();

	void Enable() const;
	void Disable() const;

	void AddAdditionalShaderModule(unsigned int type, const std::string& shaderPath, bool useBinaries = false);
	void FinalisePipline();

	enum LightCalculation : int
	{
		GEAR_CALC_LIGHT_DIFFUSE   = 1,
		GEAR_CALC_LIGHT_SPECULAR  = 2,
		GEAR_CALC_LIGHT_AMBIENT   = 4,
		GEAR_CALC_LIGHT_EMIT      = 8
	};
	void SetLighting(int types);

	template<typename T> //Consider using switch over if, else if and else statements
	inline void SetUniform(std::string locationInput, const std::vector<T>& values) const
	{
		GLint location = glGetUniformLocation(m_ShaderID, locationInput.c_str());
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
	inline void SetUniformArray(std::string locationInput, int size, GLsizei count, T* values) const
	{
		GLint location = glGetUniformLocation(m_ShaderID, locationInput.c_str());
		if (typeid(T) == typeid(float))
		{
				 if (size == 1) glUniform1fv(location, count, (float*)values);
			else if (size == 2) glUniform2fv(location, count, (float*)values);
			else if (size == 3) glUniform3fv(location, count, (float*)values);
			else if (size == 4) glUniform4fv(location, count, (float*)values);
			else throw;
		}
		else if (typeid(T) == typeid(int))
		{
				 if (size == 1) glUniform1iv(location, count, (int*)values);
			else if (size == 2) glUniform2iv(location, count, (int*)values);
			else if (size == 3) glUniform3iv(location, count, (int*)values);
			else if (size == 4) glUniform4iv(location, count, (int*)values);
			else throw;
		}
		else if (typeid(T) == typeid(unsigned int))
		{
				 if (size == 1) glUniform1uiv(location, count, (unsigned int*)values);
			else if (size == 2) glUniform2uiv(location, count, (unsigned int*)values);
			else if (size == 3) glUniform3uiv(location, count, (unsigned int*)values);
			else if (size == 4) glUniform4uiv(location, count, (unsigned int*)values);
			else throw;
		}
		else throw;
	}

	template<int N>
	inline void SetUniformMatrix(std::string locationInput, GLsizei count, GLboolean tranpose, const float& values) const
	{
		GLint location = glGetUniformLocation(m_ShaderID, locationInput.c_str());
			 if (N == 2) glUniformMatrix2fv(location, count, tranpose, &values);
		else if (N == 3) glUniformMatrix3fv(location, count, tranpose, &values);
		else if (N == 4) glUniformMatrix4fv(location, count, tranpose, &values);
		else throw;
	}

	inline GLuint GetShaderID() const { return m_ShaderID; }
	
private:
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexshader, const std::string& fragmentshader);
	
	//Binary Loader
	bool IsBinaryFile(const std::string& input);
	std::string AmmendFilePath(const std::string& filePath, bool fileIsBinary, bool shaderWantsBinary);
	unsigned int CompileShader(unsigned int type, const std::vector<char>& source);
	unsigned int CreateShader(const std::vector<char>& vertexshader, const std::vector<char>& fragmentshader);
};
}
}
}