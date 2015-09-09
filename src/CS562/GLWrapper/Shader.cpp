////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Shader.h"

#include "../OpenGL/gl_core_4_4.hpp"

namespace CS562
{
	Shader::Shader(ShaderType::Type type)
		: is_usable_(false), type_(type)
	{
		unsigned gl_type;
		switch (type_)
		{
		case ShaderType::Vertex:
			gl_type = gl::VERTEX_SHADER;
			break;
		case ShaderType::Fragment:
			gl_type = gl::FRAGMENT_SHADER;
			break;
		case ShaderType::Geometry:
			gl_type = gl::GEOMETRY_SHADER;
			break;
		case ShaderType::TessControl:
			gl_type = gl::TESS_CONTROL_SHADER;
			break;
		case ShaderType::TessEvaluation:
			gl_type = gl::TESS_EVALUATION_SHADER;
			break;
		case ShaderType::Compute:
			gl_type = gl::COMPUTE_SHADER;
			break;
		}

		gl_object_ = gl::CreateShader(gl_type);
	}

	Shader::~Shader()
	{
		gl::DeleteShader(gl_object_);
	}

	void Shader::AppendSourceCode(const char* source)
	{
		source_.append(source);
	}

	bool Shader::Compile()
	{
		if (source_.empty())
		{
			error_ = "No source was provided.";
			return false;
		}

		const char *source_str = source_.c_str();
		gl::ShaderSource(gl_object_,	// The shader to add the source to
						 1,				// the number of source strings
						 &source_str,	// the source string
						 nullptr);		// the length of the source strings, if they are non-NUL terminated

		gl::CompileShader(gl_object_);

		//verify that we had no problems
		GLint success;
		gl::GetShaderiv(gl_object_, gl::COMPILE_STATUS, &success);

		if (success != gl::TRUE_)
		{
			GLint max_length = 0;
			gl::GetShaderiv(gl_object_, gl::INFO_LOG_LENGTH, &max_length);

			error_.resize(max_length);

			gl::GetShaderInfoLog(gl_object_, max_length, &max_length, &error_[0]);

			return false;
		}

		is_usable_ = true;
		return true;
	}

	std::string Shader::GetErrorString() const
	{
		return error_;
	}

	bool Shader::isUsable()
	{
		return is_usable_;
	}

	ShaderType::Type Shader::GetType() const
	{
		return type_;
	}
}
