#pragma once
#ifndef REPORT_HPP
#define REPORT_HPP

#include <chrono>
#include <map>
#include <ostream>

namespace las::batch {

	struct exec_report {
		std::map < int, std::size_t >   exit_code_count;
		std::size_t                     timed_out_count = 0;
		std::chrono::milliseconds       it_runtime_accumulator {};
		std::chrono::milliseconds       runtime {};
	};

	void print_report (std::ostream & stream, exec_report const & report, struct options const & opts);
}

#endif //REPORT_HPP
