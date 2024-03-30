#include "las/test/random.hpp"

namespace las::test {

    uint_fast32_t uniform (uint_fast32_t dist) {
        thread_local uniform_generator unigen;
        return unigen(dist);
    }

}