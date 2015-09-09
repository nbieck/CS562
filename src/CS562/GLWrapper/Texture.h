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
	enum class ImageFormat
	{
		//add formats relevant to us here
	};

	// This manages the GPU-side texture resource.
	class Texture
	{
	public:

		Texture();
		~Texture();

		void AllocateSpace(unsigned width, unsigned height, ImageFormat format, unsigned mip_levels);

		//parameters

		//Binding - Unbinding
		Unbinder<Texture> Bind(unsigned bind_location);
		void Unbind();

		//Data transfer

	private:

		unsigned gl_object_;

	};
}
