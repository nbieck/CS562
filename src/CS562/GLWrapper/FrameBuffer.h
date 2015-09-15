////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Unbinder.h"
#include "Texture.h"

#include <memory>
#include <vector>

namespace CS562
{
	enum class Attachments
	{
		Color0 = 0x8CE0,
		Color1,
		Color2,
		Color3,
		Color4,
		Color5,
		Color6,
		Color7,
		Color8,
		Color9,
		Depth = 0x8D00,
		Stencil = 0x8D20,
		DepthStencil = 0x821A,
	};

	class Framebuffer
	{
	public:

		Framebuffer();
		~Framebuffer();

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

		Unbinder<Framebuffer> Bind();
		void Unbind();

		//returns an ID that can be used to enable and disable the chosen attachment
		std::size_t AttachTexture(Attachments location, std::shared_ptr<Texture> tex);

		//enables the given attachment
		void EnableAttachment(std::size_t attachment_number);
		//disables the given attachment
		void DisableAttachment(std::size_t attchment_number);

		//enables the given attachments, disables the rest
		void EnableAttachments(std::vector<std::size_t> attachments);

	private:

		unsigned gl_object_;

		std::vector<Attachments> attachments_;
		std::vector<unsigned> gl_attachment_buff_;
	};
}
