#ifndef __IEBPR_SIGNAL_HPP__
#define __IEBPR_SIGNAL_HPP__

#include <csignal>

namespace iebpr
{
	template <int SIGNAL_TYPE>
	class SignalHandler
	{
	private:
		// if using multi-threading, these need to be volatile
		// which... so far will not be implemented
		volatile bool _in_use;
		static volatile std::sig_atomic_t _sig_set;
		struct sigaction _saved_sigint_act;

	public:
		SignalHandler(void) noexcept
			: _in_use(false){};

		// activation, replace signal handler with local signal handler, and
		// save original signal handler for later restore
		void activate(void)
		{
			if (_in_use)
				return;
			// save the old signal action
			sigaction(SIGNAL_TYPE, nullptr, &_saved_sigint_act);
			// replace with local action
			std::signal(SIGNAL_TYPE, _signal_handler);
			_sig_set = 0;
			_in_use = true;
			return;
		}

		bool sig_received(void) noexcept
		{
			return _sig_set;
		}

		// does the reverse to activate()
		void deactivate(void)
		{
			if (!_in_use)
				return;
			// restore original handler
			std::signal(SIGNAL_TYPE, _saved_sigint_act.sa_handler);
			_in_use = false;
			return;
		}

	private:
		static void _signal_handler(int sig) noexcept
		{
			_sig_set = 1;
			return;
		};
	};

	template <int SIGNAL_TYPE>
	volatile std::sig_atomic_t SignalHandler<SIGNAL_TYPE>::_sig_set = 0;

} // namespace iebpr

#endif