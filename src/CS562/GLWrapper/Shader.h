////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_SHADER_H_
#define CS350_SHADER_H_

#include <string>

namespace CS562
{
	namespace ShaderType
	{
		enum Type
		{
			Vertex = 0,
			Fragment,
			Geometry,
			TessControl,
			TessEvaluation,
			Compute
		};
	}

	class Shader
	{
	public:

		Shader(ShaderType::Type type);
		~Shader();
		
		Shader(const Shader& rhs) = delete;
		Shader& operator=(const Shader& rhs) = delete;

		void AppendSourceCode(const char* source);
		bool Compile();
		std::string GetErrorString() const;
		ShaderType::Type GetType() const;

		bool isUsable();

	private:

		unsigned gl_object_;

		std::string source_;
		std::string error_;

		bool is_usable_;
		ShaderType::Type type_;

		friend class ShaderProgram;
	};
}

#endif
