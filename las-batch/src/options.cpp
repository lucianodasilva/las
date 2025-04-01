#include "options.hpp"

#include <cxxopts.hpp>

namespace las::batch {

    namespace {
        cxxopts::Options make_parser () {
            auto parser = cxxopts::Options("Batch Tester", "Batch runs a command tracking failure/success rates");

            parser.positional_help ("command [args]...");
            parser.show_positional_help ();

            parser.add_options()
                ("i,iterations", "Number of times to execute the targeted command", cxxopts::value <uint32_t>()->default_value("1000"))
                ("t,timeout", "Maximum time an interation is allowed to run, in milliseconds", cxxopts::value<uint32_t>()->default_value("10"))
                ("v,verbose", "Disable redirection of standard output and standard error")
                ("command", "Targeted command", cxxopts::value < std::string > ())
                ("args", "Targeted command arguments", cxxopts::value < std::vector < std::string > > ());

            parser.add_options()
                ("h,help", "Print Help");

            parser.parse_positional ({"command", "args"});

            return parser;
        }
    }

    void options::print_help (std::ostream & stream) {
        stream << make_parser().help () << "\n";
    }

    options options::parse (int arg_c, char ** arg_v) {
        auto const opts = make_parser ().parse (arg_c, arg_v);

        if (opts.count ("command") == 0) {
            throw std::invalid_argument ("Target command missing");
        }

        return {
            /* ARGS       */ (opts.count ("args") != 0 ? opts ["args"].as<std::vector <std::string>>() : std::vector <std::string> ()),
            /* COMMAND    */ opts ["command"].as<std::string>(),
            /* TIMEOUT    */ std::chrono::milliseconds(opts ["timeout"].as < uint32_t >()),
            /* ITERATIONS */ opts ["iterations"].as < uint32_t > (),
            /* SHOW HELP  */ opts ["help"].as<bool>(),
            /* VERBOSE    */ opts ["verbose"].as<bool>(),
        };
    }

}