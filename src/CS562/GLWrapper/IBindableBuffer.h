////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_I_BINDABLE_BUFFER_H_
#define CS350_I_BINDABLE_BUFFER_H_

#include "Unbinder.h"

namespace CS562
{
	//will add more targets to this as nececssary
	namespace BufferTargets
	{
		enum BufferTargets
		{
			None		 = 0,
			Vertex		 = 0x8892,
			Index		 = 0x8893,
			DrawIndirect = 0x8F3F,
			ShaderStorage = 0x90D2,
		};
	}

	class IBindableBuffer
	{
	public:

		virtual Unbinder<IBindableBuffer> Bind(BufferTargets::BufferTargets) = 0;
		virtual Unbinder<IBindableBuffer> Bind(BufferTargets::BufferTargets target, unsigned index) = 0;
		virtual void Unbind() = 0;

		virtual unsigned GetGLObject() = 0;
	};
}

#endif
