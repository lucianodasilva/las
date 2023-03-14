#include "las/job.h"

namespace las {

	job::job () = default;

	job::job (job && other) noexcept {
		this->swap (other);
	}

	job::~job () {
		if (_thread.joinable ()) {
			stop ();
			_thread.join();
		}
	}

	job & job::operator = (job && other) noexcept {
		this->swap (other);
		return *this;
	}

	void job::stop () {
		_token.stop();
	}

	bool job::joinable () const noexcept {
		return _thread.joinable();
	}

	void job::join () {
		return _thread.join ();
	}

	void job::swap (job & other) {
		std::swap (_thread, other._thread);
		std::swap (_token, other._token);
	}

	thread_local job_token job::this_token {};

}