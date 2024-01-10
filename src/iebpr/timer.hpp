#ifndef __IEBPR_TIMER_HPP__
#define __IEBPR_TIMER_HPP__

#include <chrono>

namespace iebpr
{
	class Timer
	{
	private:
		using _clock = std::chrono::high_resolution_clock;
		using _time_t = std::chrono::time_point<_clock>;
		_time_t _start_time;
		_time_t _stop_time;

	public:
		explicit Timer() noexcept
			: _start_time(), _stop_time(){};
		inline void start(void) noexcept
		{
			_start_time = _clock::now();
			return;
		};
		inline void stop(void) noexcept
		{
			_stop_time = _clock::now();
			return;
		};
		inline std::chrono::milliseconds get_duration(void) const noexcept
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(_stop_time - _start_time);
		}
	};

} // namespace iebpr

#endif