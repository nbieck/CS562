///////////////////////////////////////////////////
//
// Author: Niklas Bieck
// Class: CS562
//
///////////////////////////////////////////////////

#include "Texture.h"

#include "ContextState.h"
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
		format_ = format;
	}

	void Texture::SetParameter(TextureParameter param, float value)
	{
		gl::TexParameterf(gl::TEXTURE_2D, static_cast<GLenum>(param), value);
	}

	void Texture::SetParameter(TextureParameter param, int value)
	{
		gl::TexParameteri(gl::TEXTURE_2D, static_cast<GLenum>(param), value);
	}

	Unbinder<Texture> Texture::Bind(unsigned bind_location)
	{
		if (ContextState::GetActiveTextureUnit() != bind_location)
			ContextState::SetActiveTextureUnit(bind_location);

		gl::BindTexture(gl::TEXTURE_2D, gl_object_);

		last_bind_location_ = bind_location;
		last_bound_as_tex_ = true;

		return Unbinder<Texture>(*this);
	}

	Unbinder<Texture> Texture::BindImage(unsigned bind_location, ImageAccessMode::type access)
	{
		gl::BindImageTexture(bind_location, gl_object_, 0, gl::FALSE_, 0, access, static_cast<unsigned>(format_));

		last_bind_location_ = bind_location;
		last_bound_as_tex_ = false;

		return Unbinder<Texture>(*this);
	}

	void Texture::Bind_NoUnbind(unsigned bind_location)
	{
		if (ContextState::GetActiveTextureUnit() != bind_location)
			ContextState::SetActiveTextureUnit(bind_location);

		gl::BindTexture(gl::TEXTURE_2D, gl_object_);

		last_bind_location_ = bind_location;
		last_bound_as_tex_ = true;
	}

	void Texture::Unbind()
	{
		if (last_bound_as_tex_)
		{
			if (ContextState::GetActiveTextureUnit() != last_bind_location_)
				ContextState::SetActiveTextureUnit(last_bind_location_);

			gl::BindTexture(gl::TEXTURE_2D, 0);
		}
		else
		{
			//gl::BindImageTexture(last_bind_location_, 0, 0, gl::FALSE_, 0, gl::READ_ONLY, gl::RGB8);
		}
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

	unsigned Texture::GetGLObject()
	{
		return gl_object_;
	}
}
