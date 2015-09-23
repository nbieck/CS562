////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////


#include "../OpenGL/gl_core_4_4.hpp"
#include "Buffer.h"

namespace CS562
{
	template <typename Data>
	Buffer<Data>::Buffer()
		: last_bind_target_(BufferTargets::None), size_(0)
	{
		gl::GenBuffers(1, &gl_object_);
	}

	template <typename Data>
	Buffer<Data>::~Buffer()
	{
		gl::DeleteBuffers(1, &gl_object_);
	}

	template <typename Data>
	void Buffer<Data>::SetUpStorage(unsigned size, unsigned creation_flags, const Data* data)
	{
		gl::BufferStorage(last_bind_target_,	// the buffer we are working on, should of course be this object
						  size * sizeof(Data),	// This size of the buffer we are asking for
						  data,					// The actual data, if any
						  creation_flags);		// Specific flags for how we plan to use the storage

		size_ = size;
	}

	template <typename Data>
	void Buffer<Data>::ResizeableStorage(unsigned size, const Data* data)
	{
		gl::BufferData(last_bind_target_, // the buffer to work on
			size * sizeof(Data),		  //size of te storage requested in bytes
			data,						  //initial data
			gl::DYNAMIC_DRAW);		      // Usage mode
		//makes 2 assumptions:
		// - if we are using this one instead of the static ones, we want it to be dynamic
		// - We are using buffers only for drawing

		size_ = size;
	}

	template <typename Data>
	Unbinder<IBindableBuffer> Buffer<Data>::Bind(BufferTargets::BufferTargets target)
	{
		last_bind_target_ = target;
		gl::BindBuffer(target, gl_object_);
		indexed_binding_ = false;

		return Unbinder<IBindableBuffer>(*this);
	}

	template<typename Data>
	inline Unbinder<IBindableBuffer> Buffer<Data>::Bind(BufferTargets::BufferTargets target, unsigned index)
	{
		last_bind_target_ = target;
		last_bind_loc_ = index;
		indexed_binding_ = true;

		gl::BindBufferBase(target, index, gl_object_);

		return Unbinder<IBindableBuffer>(*this);
	}

	template <typename Data>	
	void Buffer<Data>::BindWithoutUnbind(BufferTargets::BufferTargets target)
	{
		last_bind_target_ = target;
		gl::BindBuffer(target, gl_object_);
		indexed_binding_ = false;
	}

	template <typename Data>
	void Buffer<Data>::Unbind()
	{
		if (!indexed_binding_)
			gl::BindBuffer(last_bind_target_, 0);
		else
			gl::BindBufferBase(last_bind_target_, last_bind_loc_, 0);
	}

	template <typename Data>
	void Buffer<Data>::SendData(unsigned offset, unsigned num_elements, const Data* data)
	{
		gl::BufferSubData(last_bind_target_,			// The buffer to update, this object
						  offset * sizeof(Data),		// Offset into the buffer (in bytes)
						  num_elements * sizeof(Data),	// size of region to replace (in bytes)
						  data);						// The new data
	}

	template <typename Data>
	Data *Buffer<Data>::Map(MapModes::MapModes mode)
	{
		return reinterpret_cast<Data *>(
			gl::MapBuffer(last_bind_target_, // buffer to map
						  mode));			 // which access to map for (our enum matches ogl)
	}

	template <typename Data>
	void Buffer<Data>::Unmap()
	{
		gl::UnmapBuffer(last_bind_target_);
	}

	template <typename Data>
	unsigned Buffer<Data>::GetGLObject()
	{
		return gl_object_;
	}

	template <typename Data>
	unsigned Buffer<Data>::GetSize() const
	{
		return size_;
	}
}
