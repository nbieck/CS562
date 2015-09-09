////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "ShaderProgram.h"

#include "../OpenGL/gl_core_4_4.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace CS562
{
	ShaderProgram::ShaderProgram()
		: is_usable_(false)
	{
		gl_object_ = gl::CreateProgram();
	}

	ShaderProgram::~ShaderProgram()
	{
		gl::DeleteProgram(gl_object_);
	}

	void ShaderProgram::AddShader(const std::shared_ptr<Shader>& shader)
	{
		shaders_.push_back(shader);
	}

	bool ShaderProgram::Link()
	{
		for (auto shader : shaders_)
		{
			gl::AttachShader(gl_object_, shader->gl_object_);
		}

		gl::LinkProgram(gl_object_);

		GLint link_success;
		gl::GetProgramiv(gl_object_, gl::LINK_STATUS, &link_success);

		if (link_success != gl::TRUE_)
		{
			GLint max_length = 0;
			gl::GetProgramiv(gl_object_, gl::INFO_LOG_LENGTH, &max_length);

			error_.resize(max_length);

			gl::GetProgramInfoLog(gl_object_, max_length, &max_length, &error_[0]);

			return false;
		}
		
		for (auto shader : shaders_)
		{
			gl::DetachShader(gl_object_, shader->gl_object_);
		}

		is_usable_ = true;
		return true;
	}

	bool ShaderProgram::isUsable() const
	{
		return is_usable_;
	}

	std::string ShaderProgram::GetErrorString() const
	{
		return error_;
	}

	Unbinder<ShaderProgram> ShaderProgram::Bind()
	{
		gl::UseProgram(gl_object_);

		return Unbinder<ShaderProgram>(*this);
	}

	void ShaderProgram::Unbind()
	{
		gl::UseProgram(0);
	}

	void ShaderProgram::SetUniform(const char* name, float v0)
	{
		gl::ProgramUniform1f(gl_object_, GetUniformLocation(name), v0);
	}

	void ShaderProgram::SetUniform(const char* name, float v0, float v1)
	{
		gl::ProgramUniform2f(gl_object_, GetUniformLocation(name), v0, v1);
	}

	void ShaderProgram::SetUniform(const char* name, float v0, float v1, float v2)
	{
		gl::ProgramUniform3f(gl_object_, GetUniformLocation(name), v0, v1, v2);
	}

	void ShaderProgram::SetUniform(const char* name, float v0, float v1, float v2, float v3)
	{
		gl::ProgramUniform4f(gl_object_, GetUniformLocation(name), v0, v1, v2, v3);
	}

	void ShaderProgram::SetUniform(const char* name, int v0)
	{
		gl::ProgramUniform1i(gl_object_, GetUniformLocation(name), v0);
	}

	void ShaderProgram::SetUniform(const char* name, int v0, int v1)
	{
		gl::ProgramUniform2i(gl_object_, GetUniformLocation(name), v0, v1);
	}

	void ShaderProgram::SetUniform(const char* name, int v0, int v1, int v2)
	{
		gl::ProgramUniform3i(gl_object_, GetUniformLocation(name), v0, v1, v2);
	}

	void ShaderProgram::SetUniform(const char* name, int v0, int v1, int v2, int v3)
	{
		gl::ProgramUniform4i(gl_object_, GetUniformLocation(name), v0, v1, v2, v3);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::vec2& val)
	{
		SetUniform(name, val.x, val.y);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::vec3& val)
	{
		SetUniform(name, val.x, val.y, val.z);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::vec4& val)
	{
		SetUniform(name, val.x, val.y, val.z, val.w);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::ivec2& val)
	{
		SetUniform(name, val.x, val.y);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::ivec3& val)
	{
		SetUniform(name, val.x, val.y, val.z);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::ivec4& val)
	{
		SetUniform(name, val.x, val.y, val.z, val.w);
	}

	void ShaderProgram::SetUniform(const char* name, const glm::mat2& val)
	{
		gl::ProgramUniformMatrix2fv(gl_object_, GetUniformLocation(name), 1, gl::FALSE_, glm::value_ptr(val));
	}

	void ShaderProgram::SetUniform(const char* name, const glm::mat3& val)
	{
		gl::ProgramUniformMatrix3fv(gl_object_, GetUniformLocation(name), 1, gl::FALSE_, glm::value_ptr(val));
	}

	void ShaderProgram::SetUniform(const char* name, const glm::mat4& val)
	{
		gl::ProgramUniformMatrix4fv(gl_object_, GetUniformLocation(name), 1, gl::FALSE_, glm::value_ptr(val));
	}

	int ShaderProgram::GetUniformLocation(const char* name)
	{
		return gl::GetUniformLocation(gl_object_, name);
	}
}
