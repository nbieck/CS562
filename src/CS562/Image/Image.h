////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

namespace CS562
{
	// This contains and manages the concrete pixel data in CPU-side storage
	class Image
	{
	public:

		//in case i want to do something more fancy later for HDR images, etc.
		using PixelData = unsigned char;

		//construct an empty image
		Image();
		//construct an image with the given pixel data.
		Image(std::unique_ptr<PixelData> data, unsigned width, unsigned height, unsigned channels);
		//construct an empty image with the given dimensions
		Image(unsigned width, unsigned height, unsigned channels);

		//create a new image from the old one. Image data will be copied over
		Image(const Image& rhs);
		//move construct a new image, stealing the pixel data from the old one
		Image(Image&& rhs);

		Image& operator=(const Image& rhs);
		Image& operator=(Image&& rhs);

		//retrieves the pixel data to inspect/work on. DON'T DELETE THIS, THE IMAGE OWNS IT
		PixelData* GetData();
		const PixelData* GetData() const;

		unsigned GetWidth() const;
		unsigned GetHeight() const;
		unsigned GetNumChannels() const;

	private:

		std::unique_ptr<PixelData> pixels_;
		unsigned width_, height_, channels_;

	};
}
