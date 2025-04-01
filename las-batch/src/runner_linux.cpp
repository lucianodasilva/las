#include <las/las.h>

#if defined (LAS_OS_GNU_LINUX)

#include "options.hpp"
#include "report.hpp"
#include "runner.hpp"

#include <cstring>
#include <thread>

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace las::batch {

	void monitor (process_handle_t const handle, std::chrono::milliseconds const timeout, /* OUT */ struct exec_report & report) {
		auto status = 0;
		auto const start_time = std::chrono::system_clock::now();

		do {
			if (auto const wait_result = ::waitpid(static_cast < pid_t > (handle), &status, WNOHANG | WUNTRACED); wait_result == 0) {
				// child is still running
				std::this_thread::yield();
			} else if (wait_result > 0) {

				// child process has exited
				++report.exit_code_count[status];
				return;
			} else {
				throw std::runtime_error(
					"Failed to wait for child process with error: (" +
					std::to_string(wait_result) +
					")" +
					std::string(::strerror(errno)));
			}

		} while (std::chrono::system_clock::now() - start_time < timeout);

		// timeout
		::kill(static_cast < pid_t > (handle), SIGKILL);
		++report.timed_out_count;
	}

	bool is_command_valid (std::string const & command) {
		return ::access (command.c_str(), X_OK) == 0;
	}

	process_handle_t start (options const & opts) {
		// create native args
		auto native_args = std::make_unique < char * [] > (opts.args.size() + 1);

		for (auto i = 0; i < opts.args.size(); ++i) {
			native_args [i] = const_cast < char * > (opts.args[i].c_str());
		}

		native_args[opts.args.size()]         = nullptr;
		constexpr char * const no_env[1] = {nullptr};

		// -- vfork -- avoid stack changes after this point -----------------------------------------
		auto const pid = ::vfork();

		if (pid < 0) {
			throw std::runtime_error ("Failed to fork process with error:" + std::to_string(pid));
		}

		if (pid > 0) {
			// parent process
			return pid;
		}

		if (!opts.verbose) {
			// child process
			// redirect process output to /dev/null
			auto const dev_null = ::open("/dev/null", O_WRONLY);

			if (dev_null == -1) {
				throw std::runtime_error ("Failed to open /dev/null");
			}

			// redirect standard output and standard error to /dev/null
			if (dup2(dev_null, STDOUT_FILENO) == -1 || dup2(dev_null, STDERR_FILENO) == -1) {
				throw std::runtime_error ("Failed to redirect output");
			}
		}

		// replace child process with new executable
		execve (opts.command.c_str (), native_args.get(), no_env);

		// failure to replace process with executable
		// exit without calling destructors and others to avoid stack corruption on the parent process
		_exit (1);
	}

}

#endif