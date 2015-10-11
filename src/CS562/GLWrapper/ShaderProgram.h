////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_SHADER_PROGRAM_H_
#define CS350_SHADER_PROGRAM_H_

#include "Shader.h"
#include "Unbinder.h"

#include <glm\glm.hpp>

#include <memory>
#include <vector>
#include <string>

namespace CS562
{
	class ShaderProgram
	{
	public:

		ShaderProgram();
		~ShaderProgram();
		
		ShaderProgram(const ShaderProgram& rhs) = delete;
		ShaderProgram& operator=(const ShaderProgram& rhs) = delete;

		void AddShader(const std::shared_ptr<Shader>& shader);

		bool Link();
		bool isUsable() const;
		std::string GetErrorString() const;

		Unbinder<ShaderProgram> Bind();
		void Unbind();

		void RunCompute(unsigned width = 1, unsigned height = 1, unsigned depth = 1);

		//SetUniform
		void SetUniform(const char* name, float v0);
		void SetUniform(const char* name, float v0, float v1);
		void SetUniform(const char* name, float v0, float v1, float v2);
		void SetUniform(const char* name, float v0, float v1, float v2, float v3);
		void SetUniform(const char* name, int v0);
		void SetUniform(const char* name, int v0, int v1);
		void SetUniform(const char* name, int v0, int v1, int v2);
		void SetUniform(const char* name, int v0, int v1, int v2, int v3);
		void SetUniform(const char* name, const glm::vec2& val);
		void SetUniform(const char* name, const glm::vec3& val);
		void SetUniform(const char* name, const glm::vec4& val);
		void SetUniform(const char* name, const glm::ivec2& val);
		void SetUniform(const char* name, const glm::ivec3& val);
		void SetUniform(const char* name, const glm::ivec4& val);
		void SetUniform(const char* name, const glm::mat2& val);
		void SetUniform(const char* name, const glm::mat3& val);
		void SetUniform(const char* name, const glm::mat4& val);

		template <typename... Ts>
		void SetUniform(const std::string& name, Ts... ts)
		{
			SetUniform(name.c_str, ts...);
		}

	private:

		unsigned gl_object_;

		std::vector<std::shared_ptr<Shader>> shaders_;

		std::string error_;

		bool is_usable_;

		int GetUniformLocation(const char* name);
	};
}

#endif
