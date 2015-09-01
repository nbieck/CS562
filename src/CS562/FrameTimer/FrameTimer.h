////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_FRAME_TIMER_H_
#define CS350_FRAME_TIMER_H_

namespace CS350
{
	class FrameTimer
	{
	public:

		FrameTimer();

		void Update();

		double dt() const;
		double fps() const;

	private:

		double dt_;

		double avg_fps_;

		double timer_frequency_;

		__int64 last_check_t_;
	};
}

#endif
