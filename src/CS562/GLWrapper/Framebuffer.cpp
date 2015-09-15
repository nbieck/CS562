////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameBuffer.h"

#include "../OpenGL/gl_core_4_4.hpp"

namespace CS562
{
	Framebuffer::Framebuffer()
	{
		gl::GenFramebuffers(1, &gl_object_);
	}

	Framebuffer::~Framebuffer()
	{
		gl::DeleteFramebuffers(1, &gl_object_);
	}

	Unbinder<Framebuffer> Framebuffer::Bind()
	{
		gl::BindFramebuffer(gl::FRAMEBUFFER, gl_object_);

		return Unbinder<Framebuffer>(*this);
	}

	void Framebuffer::Unbind()
	{
		gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
	}

	std::size_t Framebuffer::AttachTexture(Attachments location, std::shared_ptr<Texture> tex)
	{
		attachments_.push_back(location);
		gl_attachment_buff_.push_back(static_cast<unsigned>(location));

		gl::FramebufferTexture(gl::FRAMEBUFFER, static_cast<unsigned>(location), tex->GetGLObject(), 0);

		return attachments_.size() - 1;
	}

	void Framebuffer::EnableAttachment(std::size_t attachment_number)
	{
		gl_attachment_buff_[attachment_number] = static_cast<unsigned>(attachments_[attachment_number]);

		gl::DrawBuffers(gl_attachment_buff_.size(), gl_attachment_buff_.data());
	}

	void Framebuffer::DisableAttachment(std::size_t attachment_number)
	{
		gl_attachment_buff_[attachment_number] = gl::NONE;

		gl::DrawBuffers(gl_attachment_buff_.size(), gl_attachment_buff_.data());
	}

	void Framebuffer::EnableAttachments(std::vector<std::size_t> attachments)
	{
		std::fill(gl_attachment_buff_.begin(), gl_attachment_buff_.end(), gl::NONE);

		for (auto att : attachments)
		{
			gl_attachment_buff_[att] = static_cast<unsigned>(attachments_[att]);
		}

		gl::DrawBuffers(gl_attachment_buff_.size(), gl_attachment_buff_.data());
	}
}
