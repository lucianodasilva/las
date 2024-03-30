#include <algorithm>
#include <las/test/concurrent_stress_tester.hpp>

namespace las::test {

    random_iterative_task::random_iterative_task (std::vector < stress_task > tasks, std::size_t const ITERATIONS) :
            TASKS (std::move(tasks)),
            DISTRIBUTION { std::accumulate (TASKS.begin(), TASKS.end(), uint_fast32_t {}, [](auto const & acc, auto const & task) {
                return acc + task.probability;
            })},
            ITERATIONS(ITERATIONS)
    {}

    void random_iterative_task::operator ()() const {
        for (std::size_t i = 0; i < ITERATIONS; ++i) {
            auto const IDX = GEN(DISTRIBUTION);
            auto dist_sum = 0;

            for (auto const & [callback, probability] : TASKS) {
                dist_sum += probability;

                if (IDX < dist_sum) {
                    callback();
                    break;
                }
            }
        }
    }

    concurrent_stress_tester::concurrent_stress_tester (stresser_threading_policy const POLICY, std::size_t const THREAD_COUNT) :
        AFFINITY_LIST(core_affinity (POLICY, THREAD_COUNT)),
        POLICY (POLICY),
        THREAD_COUNT (AFFINITY_LIST.size ())
    {
        // initialize thread lanes
        for (auto const & CORE_AFFINITY : AFFINITY_LIST) {
            _lanes.emplace_back (lane_thread, std::reference_wrapper (*this), _start_sync.get(), _end_sync.get(), _run_token.get(), CORE_AFFINITY);
        }
    }

    concurrent_stress_tester::~concurrent_stress_tester () {
        // end execution
        this->_run_token->store (false);
        this->_start_sync->arrive_and_wait ();

        // join threads
        for (auto & lane : _lanes) {
            if (lane.joinable ()) {
                lane.join ();
            }
        }
    }

    void concurrent_stress_tester::dispatch (std::vector < stress_task > const & tasks, std::size_t const ITERATIONS, std::function < void () > const & custom_main_thread_task) {
        this->_iterative_task = std::make_unique < random_iterative_task > (tasks, ITERATIONS / THREAD_COUNT);

        // notify lanes and wait for them to sync
        _start_sync->arrive_and_wait ();

        // if we have something to run on main thread, do it
        if (custom_main_thread_task) {
            custom_main_thread_task ();
        }

        // notify lanes to prepare for next task execution
        _end_sync->arrive_and_wait ();
    }


    std::vector < core_id_t > concurrent_stress_tester::core_affinity(stresser_threading_policy const POLICY, std::size_t const THREAD_COUNT) {
        auto vcore_count = std::thread::hardware_concurrency ();

        switch (POLICY) {
        case stresser_threading_policy::no_affinity: {
            auto affinity_vector = std::vector ( THREAD_COUNT, UNDEFINED_CORE_ID );
            return affinity_vector;
        }
        case stresser_threading_policy::affinity: {
            auto affinity_vector = std::vector ( THREAD_COUNT, UNDEFINED_CORE_ID );

            std::generate (affinity_vector.begin(), affinity_vector.end(), [vcore_count, n=0]() mutable {
                if (n == vcore_count) { n = 0; }
                return n++;
            });

            return affinity_vector;
        }
        case stresser_threading_policy::affinity_physical_cores: {
            auto const PCORE_VECTOR = physical_cores ();
            auto const CORE_COUNT = std::min(THREAD_COUNT, PCORE_VECTOR.size ());

            auto affinity_vector = std::vector (CORE_COUNT, UNDEFINED_CORE_ID );

            std::generate (affinity_vector.begin(), affinity_vector.end(), [&, n = 0]() mutable {
                if (n == PCORE_VECTOR.size ()) { n = 0; }
                return PCORE_VECTOR [n++];
            });

            return affinity_vector;
        }
        case stresser_threading_policy::affinity_all_physical_cores: {
            return physical_cores ();
        }
        default:
            throw std::runtime_error ("Unknown execution policy");
        }
    }

    void concurrent_stress_tester::lane_thread (
        concurrent_stress_tester const & this_,
        barrier * start_sync,
        barrier * end_sync,
        std::atomic_bool const * RUN_TOKEN,
        core_id_t const CPU_ID
    ){
        if (CPU_ID != UNDEFINED_CORE_ID) {
            this_thread_affinity_set (CPU_ID);
        }

        while (true) {
            start_sync->arrive_and_wait ();

            if (!RUN_TOKEN->load ()) {
                break;
            }

            lane_phase (this_);

            end_sync->arrive_and_wait ();
        }
    }

    void concurrent_stress_tester::lane_phase (concurrent_stress_tester const & this_) {
        // if no task is available, return
        if (!this_._iterative_task) {
            return;
        }

        // force the copying of the task to the local stack
        auto local_task = *this_._iterative_task;

        // call tasks
        local_task();
    }

}