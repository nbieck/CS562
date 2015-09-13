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
		R8 = 0x8229,
		RG8 = 0x822B,
		RGB8 = 0x8051,
		RGBA8 = 0x8058,
	};

	enum class TextureFormat
	{
		R = 0x1903,
		RG = 0x8227,
		RGB = 0x1907,
		RGBA = 0x1908,
		Depth = 0x1902,
	};

	enum class TextureDataType
	{
		UnsignedByte = 0x1401,
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

		Texture(const Texture& rhs) = delete;
		Texture& operator=(const Texture& rhs) = delete;

		void AllocateSpace(unsigned width, unsigned height, TextureFormatInternal format, unsigned mip_levels);

		//parameters
		void SetParameter(TextureParameter param, float value);
		void SetParameter(TextureParameter param, int value);

		//Binding - Unbinding
		Unbinder<Texture> Bind(unsigned bind_location);
		void Bind_NoUnbind(unsigned bind_location);
		void Unbind();

		//Data transfer
		void TransferData(int x_offset, int y_offset, unsigned width, unsigned height, 
			TextureFormat format, TextureDataType type, void* data, int mip_level = 0);

		void GenerateMipMaps();

	private:

		unsigned gl_object_;

		unsigned last_bind_location_;

	};
}
