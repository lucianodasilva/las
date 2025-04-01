#include "report.hpp"

#include <iomanip>
#include "options.hpp"

namespace las::batch {

	namespace {
		std::string format_time (std::chrono::milliseconds const time) {
			std::stringstream stream;
			auto const hours = std::chrono::duration_cast < std::chrono::hours > (time).count ();
			auto const minutes = std::chrono::duration_cast < std::chrono::minutes > (time).count () % 60;
			auto const seconds = std::chrono::duration_cast < std::chrono::seconds > (time).count () % 60;
			auto const ms = std::chrono::duration_cast < std::chrono::milliseconds > (time).count () % 1000;

			stream
				<< std::setw (2) << std::setfill ('0') << hours << ":"
				<< std::setw (2) << std::setfill ('0') << minutes << ":"
				<< std::setw (2) << std::setfill ('0') << seconds << "."
				<< std::setw (3) << std::setfill ('0') << ms;

			return stream.str ();
		}
	}

	void print_report (std::ostream & stream, exec_report const & report, struct options const & opts) {
		stream << "Execution Report:" << '\n';
		auto const success_it =  report.exit_code_count.find(0);
		auto const success_count = (success_it != report.exit_code_count.end () ? success_it->second : 0);

		stream << "Success: " << success_count << '\n';
		stream << "Timed out: " << report.timed_out_count << '\n';
		stream << "Avg Iteration Time: " << report.it_runtime_accumulator.count() / opts.iterations << "ms" << '\n';
		stream << "Runtime: " << format_time (report.runtime) << '\n';

		if (success_count != opts.iterations && report.exit_code_count.size() > 1) {
			stream << "== Exit Code Counters == " << '\n';
			for (auto const & [EXIT_CODE, COUNT] : report.exit_code_count) {
				stream << "[" << EXIT_CODE << ": " << COUNT << "]" << '\n';
			}
		}
	}


}
