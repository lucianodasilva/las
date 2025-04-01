#include <iostream>

#include "options.hpp"
#include "report.hpp"
#include "runner.hpp"
#include "las/system.hpp"

int main (int arg_c, char ** arg_v) {
	using namespace las::batch;

	options opts;

	try {
		opts = options::parse (arg_c, arg_v);
	} catch (std::exception & ex) {
		std::cerr << "Invalid command line arguments: " << ex.what ()
			<< std::endl
			<< std::endl;

		options::print_help (std::cout);
		return 0;
	}

	if (opts.show_help) {
		options::print_help (std::cout);
		return 0;
	}

	if (!is_command_valid(opts.command)) {
		std::cerr << "Command \"" << opts.command << "\" is not valid or not executable" << std::endl;
	}

	auto const start_time = std::chrono::system_clock::now();

	// create child process
	exec_report report;
	auto it_cursor = opts.iterations;

	while (it_cursor > 0) {
		auto const START_IT_TIME = std::chrono::system_clock::now();

		try {
			// parent process
			auto pid = start(opts);

			monitor(pid, opts.timeout, report);

			report.it_runtime_accumulator += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - START_IT_TIME);
			--it_cursor;
		} catch (std::exception & ex) {
			std::cerr << ex.what () << '\n';
			std::cerr << "Failed to start process" << '\n';
			return 1;
		}
	}

	report.runtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time);
	print_report (std::cout, report, opts);

	return 0;

}
