////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../GLWrapper/FrameBuffer.h"
#include "../GLWrapper/Texture.h"

#include <memory>
#include <array>

namespace CS562
{
	namespace Buffers
	{
		enum Buffers
		{
			LightAccumulation = 0,
			Position,
			Normal,
			Diffuse,
			Specular,
			Alpha,
			Depth,

			NumBuffs
		};
	}

	class GBuffer
	{
	public:

		GBuffer(unsigned width, unsigned height);

		void BindTextures(unsigned starting_idx, bool include_light = true);
		void UnbindTextures();

		std::shared_ptr<Framebuffer> g_buff;

		std::array<std::shared_ptr<Texture>, Buffers::NumBuffs> attachments;
	};
}
