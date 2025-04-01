#pragma once
#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <chrono>
#include <string>
#include <vector>

namespace las::batch {

    struct options {

        static void print_help (std::ostream & stream);

        static options parse (int arg_c, char ** arg_v);

        std::vector < std::string > args;
        std::string                 command;
        std::chrono::milliseconds   timeout;
        uint32_t                    iterations;
        bool                        show_help;
        bool                        verbose;
    };

}

#endif //OPTIONS_HPP