#pragma once
#ifndef CONCURRENT_STRESS_TESTER_HPP
#define CONCURRENT_STRESS_TESTER_HPP

#include <cinttypes>
#include <functional>
#include <thread>
#include <vector>

#include <las/barrier.hpp>
#include <las/test/random.hpp>

namespace las::test {

    /// Stresser threading policy.
    enum struct stresser_threading_policy : uint8_t {
        no_affinity,                    ///< no affinity set, create threads as set by THREAD_COUNT.
        affinity,                       ///< create threads as set by THREAD_COUNT with affinity set per thread to a virtual core.
        affinity_physical_cores,        ///< create threads as set by THREAD_COUNT with affinity set per thread to a physical core to a maximum of available physical cores.
        affinity_all_physical_cores,    ///< create one thread per available physical core and set affinity to that core. Ignores THREAD_COUNT.
    };

    using stress_task_callback_t = std::function < void (void) >;

    /// A task to be executed in a random_iterative_task.
    struct stress_task {
        stress_task_callback_t  callback;       ///< The task to be executed.
        int                     probability;    ///< The weighted probability of this task being executed.
    };

    /// A task to be executed by a concurrent_stress_tester.
    struct random_iterative_task {
        random_iterative_task (std::vector < stress_task > tasks, std::size_t ITERATIONS);
        void operator ()() const;
    private:
        std::vector < stress_task > const   TASKS;
        uniform_generator const             GEN {};
        uint_fast32_t const                 DISTRIBUTION;
        std::size_t const                   ITERATIONS;
    };

    /// iterative random task executor
    struct concurrent_stress_tester final {

        /// Construct a concurrent_stress_tester.
        /// \param POLICY The thread construction policy to use.
        /// \param THREAD_COUNT The number of threads to create.
        explicit concurrent_stress_tester (stresser_threading_policy POLICY = stresser_threading_policy::no_affinity, std::size_t THREAD_COUNT = std::thread::hardware_concurrency());

        ~concurrent_stress_tester();

        /// Dispatch tasks to the threads.
        /// \param tasks The tasks to dispatch.
        /// \param ITERATIONS The number of iterations to run the tasks.
        /// \param custom_main_thread_task A task to run on the main thread while the random iterations are executing.
        void dispatch(std::vector < stress_task > const & tasks, std::size_t ITERATIONS = 1000000, std::function < void () > const & custom_main_thread_task = nullptr);

        /// Get the number of threads created.
        /// \return The number of threads created.
        [[nodiscard]] std::size_t thread_count () const { return THREAD_COUNT; }

        /// Get the number of cores to be created for a given configuration and their affinity.
        [[nodiscard]] static std::vector < core_id_t > core_affinity (stresser_threading_policy POLICY, std::size_t THREAD_COUNT);
    private:

        static void lane_thread (
            concurrent_stress_tester const &        this_,
            barrier *                               start_sync,
            barrier *                               end_sync,
            std::atomic_bool const *                RUN_TOKEN,
            core_id_t                               CPU_ID);

        static void lane_phase (concurrent_stress_tester const & this_);

        std::vector < core_id_t > const             AFFINITY_LIST;
        stresser_threading_policy const             POLICY;
        std::size_t const                           THREAD_COUNT;

        std::unique_ptr < barrier >                 _start_sync{ std::make_unique < barrier >(static_cast < int32_t >(THREAD_COUNT + 1)) };
        std::unique_ptr < barrier >                 _end_sync{ std::make_unique < barrier >(static_cast < int32_t > (THREAD_COUNT + 1)) };
        std::unique_ptr < std::atomic_bool >        _run_token{ std::make_unique < std::atomic_bool >(true) };
        std::unique_ptr < random_iterative_task >   _iterative_task;
        std::vector < std::thread >                 _lanes;
    };

}

#endif //CONCURRENT_STRESS_TESTER_HPP
