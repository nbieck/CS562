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

		R16 = 0x822A,
		RG16 = 0x822C,
		RGB16 = 0x8054,
		RGBA16 = 0x805B,

		R32F = 0x822E,
		RG32F = 0x8230,
		RGB32F = 0x8815,
		RGBA32F = 0x8814,

		DEPTH16 = 0x81A5,
		DEPTH24 = 0x81A6,
		DEPTH32 = 0x81A7,
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
		Float = 0x1406,
	};

	enum class TextureParameter
	{
		WrapS = 0x2802,
		WrapT = 0x2803,
		MagFilter = 0x2800,
		MinFilter = 0x2801
	};

	namespace TextureParamValue
	{
		const int FilterNearest = 0x2600;
		const int FilterLinear = 0x2601;
		const int FilterNearestMipmapNearest = 0x2700;
		const int FilterNearestMipmapLinear = 0x2702;
		const int FilterLinearMipmapNearest = 0x2701;
		const int FilterLinearMipmapLinear = 0x2703;
		const int ClampToEdge = 0x812F;
		const int ClampToBorder = 0x812D;
		const int MirroredRepeat = 0x8370;
		const int Repeat = 0x2901;
	}

	namespace ImageAccessMode
	{
		enum type
		{
			ReadOnly = 0x88B8,
			WriteOnly = 0x88B9,
			ReadWrite = 0x88BA
		};
	}

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
		Unbinder<Texture> BindImage(unsigned bind_location, ImageAccessMode::type access);
		void Bind_NoUnbind(unsigned bind_location);
		void Unbind();

		//Data transfer
		void TransferData(int x_offset, int y_offset, unsigned width, unsigned height, 
			TextureFormat format, TextureDataType type, void* data, int mip_level = 0);

		void GenerateMipMaps();

		//for internal use only
		unsigned GetGLObject();

	private:

		unsigned gl_object_;

		TextureFormatInternal format_;

		unsigned last_bind_location_;
		bool last_bound_as_tex_;

	};
}
