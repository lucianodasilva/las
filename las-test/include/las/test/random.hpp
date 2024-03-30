#pragma once
#ifndef LAS_RANDOM_HPP
#define LAS_RANDOM_HPP

#include <random>

namespace las::test {

    /// random number generator
    /// \note this is a simple random number generator that wraps std::mt19937 and provides a uniform distribution
    struct uniform_generator {
        uint_fast32_t operator() (uint_fast32_t const DIST) const {
            return _gen() % DIST;
        }
    private:
        mutable std::mt19937 _gen {std::random_device {} ()};
    };

    /// generate a random number using a uniform distribution within the range
    /// \param dist distribution
    /// \return a random number from zero to dist
    uint_fast32_t uniform(uint_fast32_t dist);

    /// generate a random number using a uniform distribution from low to high
    /// \param low lower bound for the uniform distribution
    /// \param high higher bound for the uniform distribution
    /// \return a random number from low to high
    inline uint_fast32_t uniform(uint_fast32_t low, uint_fast32_t high) {
        if (high < low) {
            std::swap (low, high);
        }

        return low + uniform (high - low);
    }

}

#endif //LAS_RANDOM_HPP
