///////////////////////////////////////////////////
//
// Author: Niklas Bieck
// Class: CS562
//
///////////////////////////////////////////////////

#pragma once

namespace CS562
{
	class ContextState
	{
	public:

		static void SetActiveTextureUnit(unsigned idx);
		static unsigned GetActiveTextureUnit();

	private:

		static unsigned active_tex_unit_;
	};
}
