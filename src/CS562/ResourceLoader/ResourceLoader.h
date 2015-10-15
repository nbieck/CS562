////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_RESOURCE_LOADER_H_
#define CS350_RESOURCE_LOADER_H_

#include "../GLWrapper/Shader.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../CompoundObjects/Geometry.h"
#include "../CompoundObjects/Material.h"
#include "../GLWrapper/Texture.h"

#include <memory>
#include <map>

namespace CS562
{
	class ResourceLoader
	{
	public:

		static std::shared_ptr<Shader> LoadShaderFromFile(const char* filename, ShaderType::Type type);

		static std::shared_ptr<ShaderProgram> LoadShaderProgramFromFile(const char* filename);

		static std::string LoadObjFile(std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>>& geom, std::vector<std::shared_ptr<Material>>& mats, std::string filename);

		static std::shared_ptr<Texture> LoadTextureFromFile(const std::string& filename);

		static std::shared_ptr<Texture> LoadHDRTexFromFile(const std::string& filename);

	private:

		static std::map<std::string, std::weak_ptr<Texture>> loaded_textures_;

		static int ComputeMipLevels(int width, int height);

		template <typename T>
		static void InvertImageVertically(int width, int height, int channels, T * const image)
		{
			for (int j = 0; j * 2 < height; ++j)
			{
				int index1 = j * width * channels;
				int index2 = (height - 1 - j) * width * channels;
				for (int i = width * channels; i > 0; --i)
				{
					T temp = image[index1];
					image[index1] = image[index2];
					image[index2] = temp;
					++index1;
					++index2;
				}
			}
		}

	};
}

#endif
