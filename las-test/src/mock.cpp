#include "las/test/mock.h"

namespace las::test {

    void basic_mock::reset () {
        _call_count = 0;
        _failed_call_count = 0;
    }

    bool basic_mock::called_once() const {
        return _call_count.load() == 1;
    }

    bool basic_mock::not_called() const {
        return _call_count.load() == 0;
    }

    bool basic_mock::has_failed() const {
        return _failed_call_count.load() != 0;
    }

    void basic_mock::inc_call() const {
        ++_call_count;
    }

    void basic_mock::inc_failed_call() const {
        ++_failed_call_count;
    }

    void mock_container::attach(basic_mock &call_log_ref) {
        _call_logs.push_back(&call_log_ref);
    }

    void mock_container::reset() {
        for (auto *item: _call_logs) {
            item->reset();
        }
    }

}