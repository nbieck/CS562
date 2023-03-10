////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "GBuffer.h"

namespace CS562
{
	GBuffer::GBuffer(unsigned width, unsigned height)
	{
		g_buff = std::make_shared<Framebuffer>();

		for (auto& tex : attachments)
		{
			tex = std::make_shared<Texture>();
		}

		{
			auto unbind = attachments[Buffers::LightAccumulation]->Bind(1);
			attachments[Buffers::LightAccumulation]->AllocateSpace(width, height, TextureFormatInternal::RGBA32F, 1);
		}
		{
			auto unbind = attachments[Buffers::Position]->Bind(1);
			attachments[Buffers::Position]->AllocateSpace(width, height, TextureFormatInternal::RGBA32F, 1);
		}
		{
			auto unbind = attachments[Buffers::Normal]->Bind(1);
			attachments[Buffers::Normal]->AllocateSpace(width, height, TextureFormatInternal::RGBA32F, 1);
		}
		{
			auto unbind = attachments[Buffers::Diffuse]->Bind(1);
			attachments[Buffers::Diffuse]->AllocateSpace(width, height, TextureFormatInternal::RGB16, 1);
		}
		{
			auto unbind = attachments[Buffers::Specular]->Bind(1);
			attachments[Buffers::Specular]->AllocateSpace(width, height, TextureFormatInternal::RGB16, 1);
		}
		{
			auto unbind = attachments[Buffers::Alpha]->Bind(1);
			attachments[Buffers::Alpha]->AllocateSpace(width, height, TextureFormatInternal::R32F, 1);
		}
		{
			auto unbind = attachments[Buffers::Depth]->Bind(1);
			attachments[Buffers::Depth]->AllocateSpace(width, height, TextureFormatInternal::DEPTH24, 1);
		}

		auto unbind = g_buff->Bind();
		g_buff->AttachTexture(Attachments::Color0, attachments[Buffers::LightAccumulation]);
		g_buff->AttachTexture(Attachments::Color1, attachments[Buffers::Position]);
		g_buff->AttachTexture(Attachments::Color2, attachments[Buffers::Normal]);
		g_buff->AttachTexture(Attachments::Color3, attachments[Buffers::Diffuse]);
		g_buff->AttachTexture(Attachments::Color4, attachments[Buffers::Specular]);
		g_buff->AttachTexture(Attachments::Color5, attachments[Buffers::Alpha]);
		g_buff->AttachTexture(Attachments::Depth, attachments[Buffers::Depth]);
	}

	void GBuffer::BindTextures(unsigned starting_idx, bool include_light)
	{
		unsigned start = 1;
		if (include_light)
			start = 0;

		for (unsigned i = start; i < Buffers::NumBuffs; ++i)
		{
			if (include_light)
				attachments[i]->Bind_NoUnbind(starting_idx + i);
			else
				attachments[i]->Bind_NoUnbind(starting_idx + i - 1);
		}
	}

	void GBuffer::UnbindTextures()
	{
		for (auto tex : attachments)
			tex->Unbind();
	}
}
