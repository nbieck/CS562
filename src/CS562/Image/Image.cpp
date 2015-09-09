////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Image.h"

namespace CS562
{
	Image::Image()
		: width_(0), height_(0), channels_(0)
	{}

	Image::Image(std::unique_ptr<PixelData[]> data, unsigned width, unsigned height, unsigned channels)
		: pixels_(std::move(data)), width_(width), height_(height), channels_(channels)
	{}

	Image::Image(unsigned width, unsigned height, unsigned channels)
		: width_(width), height_(height), channels_(channels)
	{
		pixels_ = std::make_unique<PixelData[]>(width_ * height_ * channels_);
	}

	Image::Image(const Image & rhs)
		: width_(rhs.width_), height_(rhs.height_), channels_(rhs.channels_)
	{
		pixels_ = std::make_unique<PixelData[]>(width_ * height_ * channels_);
		std::memcpy(pixels_.get(), rhs.pixels_.get(), width_ * height_ * channels_ * sizeof(PixelData));
	}

	Image::Image(Image && rhs)
		: pixels_(std::move(rhs.pixels_)), width_(rhs.width_), height_(rhs.height_), channels_(rhs.channels_)
	{
		rhs.width_ = rhs.height_ = rhs.channels_ = 0;
	}

	Image & Image::operator=(const Image & rhs)
	{
		if (this != &rhs)
		{
			if (width_ * height_ * channels_ != rhs.width_ * rhs.height_ * rhs.channels_)
				pixels_ = std::make_unique<PixelData[]>(rhs.width_ * rhs.height_ * rhs.channels_);
			std::memcpy(pixels_.get(), rhs.pixels_.get(), rhs.width_ * rhs.height_ * rhs.channels_ * sizeof(PixelData));

			width_ = rhs.width_;
			height_ = rhs.height_;
			channels_ = rhs.channels_;
		}

		return *this;
	}

	Image & Image::operator=(Image && rhs)
	{
		pixels_ = std::move(rhs.pixels_);
		width_ = rhs.width_;
		height_ = rhs.height_;
		channels_ = rhs.channels_;

		rhs.width_ = rhs.height_ = rhs.channels_ = 0;

		return *this;
	}

	Image::PixelData * Image::GetData()
	{
		return pixels_.get();
	}

	const Image::PixelData * Image::GetData() const
	{
		return pixels_.get();
	}

	unsigned Image::GetWidth() const
	{
		return width_;
	}

	unsigned Image::GetHeight() const
	{
		return height_;
	}

	unsigned Image::GetNumChannels() const
	{
		return channels_;
	}
}
