///////////////////////////////////////////////////
//
// Author: Niklas Bieck
// Class: CS562
//
///////////////////////////////////////////////////

#include "Texture.h"

#include "../OpenGL/gl_core_4_4.hpp"

namespace CS562
{
	Texture::Texture()
	{
		gl::GenTextures(1, &gl_object_);
	}

	Texture::~Texture()
	{
		gl::DeleteTextures(1, &gl_object_);
	}

	void Texture::AllocateSpace(unsigned width, unsigned height, TextureFormatInternal format, unsigned mip_levels)
	{
		//we assume that the texture has been bound and is in fact what we will operate on
		gl::TexStorage2D(gl::TEXTURE_2D, mip_levels, static_cast<GLenum>(format), width, height);
	}

	void Texture::SetParameter(TextureParameter param, float value)
	{
		gl::TexParameterf(gl::TEXTURE_2D, static_cast<GLenum>(param), value);
	}

	void Texture::SetParameter(TextureParameter param, int value)
	{
		gl::TexParameterf(gl::TEXTURE_2D, static_cast<GLenum>(param), value);
	}

	Unbinder<Texture> Texture::Bind(unsigned bind_location)
	{
		gl::BindTexture(gl::TEXTURE_2D, gl_object_);

		return Unbinder<Texture>(*this);
	}

	void Texture::Unbind()
	{
		gl::BindTexture(gl::TEXTURE_2D, 0);
	}

	void Texture::TransferData(int x_offset, int y_offset, unsigned width, unsigned height, 
		TextureFormat format, TextureDataType type, void* data, int mip_level)
	{
		gl::TexSubImage2D(gl::TEXTURE_2D, mip_level, x_offset, y_offset, width, height,
			static_cast<GLenum>(format), static_cast<GLenum>(type), data);
	}

	void Texture::GenerateMipMaps()
	{
		gl::GenerateMipmap(gl::TEXTURE_2D);
	}
}
