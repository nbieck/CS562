///////////////////////////////////////////////////
//
// Author: Niklas Bieck
// Class: CS562
//
///////////////////////////////////////////////////

#pragma once

#include "Unbinder.h"

namespace CS562
{
	enum class TextureFormatInternal
	{
		//add formats relevant to us here
	};

	enum class TextureFormat
	{

	};

	enum class TextureDataType
	{

	};

	enum class TextureParameter
	{

	};

	// This manages the GPU-side texture resource.
	class Texture
	{
	public:

		Texture();
		~Texture();

		void AllocateSpace(unsigned width, unsigned height, TextureFormatInternal format, unsigned mip_levels);

		//parameters
		void SetParameter(TextureParameter param, float value);
		void SetParameter(TextureParameter param, int value);

		//Binding - Unbinding
		Unbinder<Texture> Bind(unsigned bind_location);
		void Unbind();

		//Data transfer
		void TransferData(int x_offset, int y_offset, unsigned width, unsigned height, 
			TextureFormat format, TextureDataType type, int mip_level = 0);

		void GenerateMipMaps();

	private:

		unsigned gl_object_;

	};
}
