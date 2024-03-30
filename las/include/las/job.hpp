#ifndef LAS_JOB_H
#define LAS_JOB_H

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <type_traits>

#include "details.hpp"
#include "traits.hpp"

namespace las {

	template < typename func_t >
	struct signature;

	template < typename return_t, typename ... args_t >
	struct signature < return_t (args_t...) > {
	public:
		using argument_types = std::tuple < args_t... >;
		using return_type = return_t;
	};

	struct job_token {
	public:

		struct context_t {
		public:

			std::atomic_bool 			stop_request {};
			std::chrono::milliseconds 	interval {};

			std::chrono::high_resolution_clock::time_point
				time_point {};
		};

		inline void interval_reset (std::chrono::milliseconds interval) {
			context->time_point = std::chrono::high_resolution_clock::now();
			context->interval = interval;
		}

		inline void interval_reset () {
			interval_reset (context->interval);
		}

		inline void interval_wait () {
			using namespace std::chrono;

			if (context->interval.count () == 0) {
				std::this_thread::yield();
				return;
			}

			auto exec_span = high_resolution_clock::now () - context->time_point;
			auto rem_time = context->interval - exec_span;

			if (rem_time.count () > 0) {
                std::this_thread::sleep_for(rem_time);
            } else {
                std::this_thread::yield();
            }

			context->time_point = high_resolution_clock::now();
		}

		[[nodiscard]]
        inline bool stop_requested () const {
			return context->stop_request.load ();
		}

		inline void stop () {
			context->stop_request.store (true);
		}

		std::shared_ptr < context_t >
			context {std::make_shared < context_t > () };
	};

	class job : no_copy {
	public:

		job ();

		template < typename call_t, typename ... args_t >
		inline explicit job (
                call_t && call, args_t && ... args_v
		) :
			_token {},
			_thread {
				[](auto call, auto token, auto ... args_v) {
					this_token = token;

					if constexpr (std::is_invocable_v<std::decay_t<decltype(call)>, decltype (token), decltype(args_v)...>) {
						std::invoke(
							std::forward<decltype(call)>(call),
							token,
							std::forward<decltype(args_v)>(args_v)...);
					} else {
                        std::invoke(
                                std::forward<decltype(call)>(call),
                                std::forward<decltype(args_v)>(args_v)...);
					}
				},
				std::forward<call_t>(call),
				_token,
				std::forward<args_t>(args_v)...
			}
		{}

		job (job && other) noexcept;

		~job ();

		job & operator=(job const &) = delete;
		job & operator = (job && other) noexcept;

		void stop ();

		[[nodiscard]]
		bool joinable () const noexcept;

		void join ();

		void swap (job & other);

		thread_local static job_token this_token;

	private:
		// the order of these fields is important, keep it as
		// thread depends on _token being instanced
		job_token 	_token;
		std::thread _thread;
	};
}


#endif //DRIVERS_JOB_H
