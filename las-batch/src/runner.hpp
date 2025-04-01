#pragma once
#ifndef RUNNER_HPP
#define RUNNER_HPP

#include <chrono>
#include <string>

namespace las::batch {

	using process_handle_t = std::intptr_t;

	/// Monitor a process for a given timeout
	/// \param[in] handle the process handle
	/// \param[in] timeout the timeout
	/// \param[out] report the report to update
	void monitor (process_handle_t handle, std::chrono::milliseconds timeout, /* OUT */ struct exec_report & report);

	/// Evaluate if a command name is valid
	/// \param[in] command the command to evaluate
	/// \return true if the command is valid, false otherwise
	bool is_command_valid (std::string const & command);

	/// Start a process
	/// \param[in] opts the options to use
	process_handle_t start (struct options const & opts);

}

#endif //RUNNER_HPP
