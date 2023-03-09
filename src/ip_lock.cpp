#include "las/ip_lock.hpp"
#include "las/system.hpp"

#ifdef LAS_OS_GNU_LINUX

#include <filesystem>
#include <sys/file.h>
#include <unistd.h>

namespace las {

	struct ip_lock::internal {
	public:

		inline explicit internal (std::string_view lock_name) :
			file_desc {([lock_name]() -> int {
				namespace fs = std::filesystem;

				// TODO: this can fail to properly lock if tmp folder "per user" is enabled
				auto lock_file =
					fs::path ("/tmp") /
					fs::path (lock_name);

				int t_desc =
					::open(lock_file.c_str(), O_CREAT, 0600);

				if (t_desc != -1) {
					if (::flock(t_desc, LOCK_EX | LOCK_NB)) { // NOLINT
						// file lock failed
						::close (t_desc);
						t_desc = -1;
					}
				}

				return t_desc;

			}())},
			locked { file_desc != -1 }
		{}

		~internal () {
			if (locked) {
                ::close(file_desc);
            }
		}

		int file_desc;
		bool locked;
	};

	ip_lock::ip_lock (std::string_view lock_name) :
		_internal { std::make_unique < ip_lock::internal > (lock_name) }
	{}

	ip_lock::~ip_lock () = default;

	ip_lock::operator bool () const {
		return _internal->locked;
	}

}

#endif