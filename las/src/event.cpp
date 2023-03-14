#include "las/event.hpp"

namespace las {

    void event_guard::dismiss() {
        if (_observer) {
            _observer->dismiss();
            _observer.reset ();
        }
    }

    void event_guard::reset () {
        _observer.reset ();
    }

    event_guard::event_guard (event_guard && other) noexcept {
        this->swap(other);
    }

    event_guard & event_guard::operator = (event_guard && other) noexcept {
        this->swap(other);
        return *this;
    }

}