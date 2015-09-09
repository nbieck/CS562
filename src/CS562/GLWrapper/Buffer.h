////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_BUFFER_H_
#define CS350_BUFFER_H_

#include "IBindableBuffer.h"

namespace CS562
{
	namespace BufferCreateFlags
	{
		enum BufferCreateFlags
		{
			None			= 0,
			DynamicStorage	= 0x0100,
			MapRead			= 0x0001,
			MapWrite		= 0x0002,
			MapPersistent	= 0x0040,
			MapCoherent		= 0x0080,
			ClientStorage	= 0x0200,
		};
	}

	namespace MapModes
	{
		enum MapModes
		{
			Read		= 0x88B8,
			Write		= 0x88B9,
			ReadWrite	= 0x88BA,
		};
	}

	/*!
		\note
			All operations on the buffer require the buffer to be bound first
	*/
	template <typename Data>
	class Buffer : public IBindableBuffer
	{
	public:
		typedef Data data_type;
		
		Buffer();
		~Buffer();

		Buffer(const Buffer& rhs) = delete;
		Buffer& operator=(const Buffer& rhs) = delete;
		
		/*!
			\param size
				The number of elements of type Data that will be contained in the buffer

			\param creation_flags
				A combination of the BufferCreateFlags

			\param data
				The initial data to load into the buffer, should be provided when DynamicStorage is not set as a flag
		*/
		void SetUpStorage(unsigned size, unsigned creation_flags, const Data* data = nullptr);

		/*!
			\brief
				Allocates storage which can be later resized (also used for the actual resizing)

			\warning
				Every time this is called, your previous data will be wiped

			\param size
				The size of storage to allocate

			\param data
				optional data to initialize the storage with
		*/
		void ResizeableStorage(unsigned size, const Data* data = nullptr);

		Unbinder<IBindableBuffer> Bind(BufferTargets::BufferTargets target) override;
		// this will bind the buffer without creating an unbinder object
		//intended for use with index buffers, that never get unbound
		void BindWithoutUnbind(BufferTargets::BufferTargets target);
		void Unbind() override;

		void SendData(unsigned offset, unsigned num_elements, const Data* data);

		Data* Map(MapModes::MapModes mode);
		void Unmap();

		unsigned GetGLObject() override;

		unsigned GetSize() const;

	private:

		unsigned gl_object_;

		BufferTargets::BufferTargets last_bind_target_;
		unsigned size_;
	};
}

#include "Buffer.inl"

#endif
