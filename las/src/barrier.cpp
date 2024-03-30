#include <las/barrier.hpp>

namespace las {

    barrier::barrier (int32_t const COUNT) :
            COUNTER_RESET_VAL (COUNT),
            _counter{COUNT}
    {}

}