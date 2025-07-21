#include <las/system.hpp>

#include <las/string.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>

namespace las {

#if defined (LAS_OS_GNU_LINUX)
    std::vector < core_id_t > physical_cores () {
        using namespace std::filesystem;
        auto const hc_val = std::thread::hardware_concurrency();

        std::vector < core_id_t > cores;

        for (std::size_t i = 0; i < hc_val; ++i) {
            // build sys details core path
            auto const cpu_path = path (
                string::cat (
                    "/sys/devices/system/cpu/cpu",
                    std::to_string (i),
                    "/topology/thread_siblings"));

            // read and parse thread sibling information
            auto const opt_content = file_content (cpu_path);

            if (!opt_content) {
                throw std::runtime_error ("Failed to read cpu information");
            }

            auto const opt_siblings = string::as_number < uint32_t > (
                *opt_content,
                string::number_format::hex);

            if (!opt_siblings) {
                throw std::runtime_error ("Failed to read cpu information");
            }

            auto core_id = ctz (*opt_siblings);

            // if distinct push
            if (std::find (cores.begin(), cores.end(), core_id) == cores.end()) {
                cores.push_back (core_id);
            }
        }

        std::sort (cores.begin (), cores.end());

        return cores;
    }

#endif

#if defined (LAS_OS_WINDOWS)
    std::vector < core_id_t > physical_cores () {
        std::vector < core_id_t > cores;

        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr;
        DWORD length_in_bytes = 0;

        // get size of buffer;
        if (FAILED(GetLogicalProcessorInformation(
            nullptr,
            &length_in_bytes)))
        {
            std::cerr << "get cpu physical cores failed" << std::endl;
            return {};
        }

        buffer = reinterpret_cast <PSYSTEM_LOGICAL_PROCESSOR_INFORMATION> (malloc(length_in_bytes));

        if (!buffer) {
            std::cerr << "get cpu physical cores allocation failed" << std::endl;
            return {};
        }

        if (FAILED(GetLogicalProcessorInformation(
            buffer,
            &length_in_bytes)))
        {
            std::cerr << "get cpu physical cores failed" << std::endl;
            return {};
        }

        auto* it = buffer;
        auto* end = it + length_in_bytes / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        while (it < end) {
            if (it->Relationship == RelationProcessorCore) {
                unsigned long id;

                if (BitScanForward64(&id, it->ProcessorMask)) {
                    cores.push_back(id);
                }
            }

            ++it;
        }

        free(buffer);

        return cores;
    }
#endif

    std::optional < std::string > file_content (std::filesystem::path const & file) {
        using namespace std::filesystem;

        // there is no file here
        if (!exists (file)) { return std::nullopt; }

        // try to open the file
        std::ifstream filestream (file, std::ios::binary);

        // TODO: perhaps we should throw an exception here
        // file stream failed to open
        if (!filestream) {
            return std::nullopt;
        }

        // read file content into a string
        return std::string (std::istreambuf_iterator {filestream}, {});
    }

}