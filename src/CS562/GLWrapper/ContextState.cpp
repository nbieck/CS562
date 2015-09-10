///////////////////////////////////////////////////
//
// Author: Niklas Bieck
// Class: CS562
//
///////////////////////////////////////////////////

#include "ContextState.h"

#include "../OpenGL/gl_core_4_4.hpp"

namespace CS562
{
	unsigned ContextState::active_tex_unit_ = 0;

	void ContextState::SetActiveTextureUnit(unsigned idx)
	{
		gl::ActiveTexture(gl::TEXTURE0 + idx);
		active_tex_unit_ = idx;
	}

	unsigned ContextState::GetActiveTextureUnit()
	{
		return active_tex_unit_;
	}
}
