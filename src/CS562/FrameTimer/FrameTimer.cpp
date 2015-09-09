////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "FrameTimer.h"

#include <Windows.h>

#include <iostream>

namespace CS562
{

	FrameTimer::FrameTimer()
		: dt_(0.016), avg_fps_(60.0), last_check_t_(0)
	{
		LARGE_INTEGER i;
		if (!QueryPerformanceFrequency(&i))
		{
			std::cerr << "Could not query performance frequency." << std::endl;
			throw "Could not query performance frequency.";
		}

		timer_frequency_ = static_cast<double>(i.QuadPart);

		QueryPerformanceCounter(&i);
		last_check_t_ = i.QuadPart;
	}

	void FrameTimer::Update()
	{
		//to keep a kind of running average of frame rate
		const double alpha = 0.4;

		LARGE_INTEGER i;
		QueryPerformanceCounter(&i);

		__int64 now = i.QuadPart;

		dt_ = static_cast<double>(now - last_check_t_) / timer_frequency_;

		avg_fps_ = (1.0 / dt_) * alpha + avg_fps_ * (1 - alpha);

		last_check_t_ = now;
	}

	double FrameTimer::dt() const
	{
		return dt_;
	}

	double FrameTimer::fps() const
	{
		return avg_fps_;
	}
}
