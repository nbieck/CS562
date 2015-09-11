////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Unbinder.h"

namespace CS562
{
	class Framebuffer
	{
	public:

		Framebuffer();
		~Framebuffer();

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

		Unbinder<Framebuffer> Bind();
		void Unbind();



	private:
	};
}
