#include <catch2/catch_all.hpp>
#include <las/dispatcher.hpp>

namespace las::test {

    TEST_CASE ("Sync Dispatcher", "[dispatcher]") {

        sync_dispatcher dispatcher;

        SECTION ("Dispatch no return value no params") {
            bool task_called{false};

            dispatcher.enqueue([&task_called]() {
                task_called = true;
            });

            // task call should be deferred until dispatch is called
            REQUIRE_FALSE (task_called);

            dispatcher.dispatch();

            REQUIRE (task_called);
            // no more tasks should be available to dispatch
            REQUIRE (dispatcher.is_done());
        }

        SECTION ("Dispatch multiple tasks") {
            bool task_1_called{false};
            bool task_2_called{false};

            dispatcher.enqueue([&task_1_called]() {
                task_1_called = true;
            });

            dispatcher.enqueue([&task_2_called]() {
                task_2_called = true;
            });

            // task calls should be deferred until dispatch is called
            REQUIRE_FALSE (task_1_called);
            REQUIRE_FALSE (task_2_called);

            dispatcher.dispatch();

            REQUIRE (task_1_called);
            REQUIRE (task_2_called);

            // no more tasks should be available to dispatch
            REQUIRE (dispatcher.is_done());
        }

        SECTION ("Enqueue with params") {
            int const EXPECTED_VALUE = 123;
            int task_value{};

            dispatcher.enqueue(
                    [&task_value](int value) {
                        task_value = value;
                    },
                    EXPECTED_VALUE);

            dispatcher.dispatch();

            REQUIRE (task_value == EXPECTED_VALUE);
        }

    }

    TEST_CASE ("Async Dispatcher", "[dispatcher]") {
        using namespace std::chrono_literals;

        int const TEST_THREAD_COUNT{2};
        async_dispatcher dispatcher{TEST_THREAD_COUNT};

        SECTION ("Dispatch multiple tasks") {
            std::promise<bool>
                    task_1_promise,
                    task_2_promise;

            auto task_1_future = task_1_promise.get_future();
            auto task_2_future = task_2_promise.get_future();

            dispatcher.enqueue([&task_1_promise]() {
                task_1_promise.set_value(true);
            });

            dispatcher.enqueue([&task_2_promise]() {
                task_2_promise.set_value(true);
            });

            if (task_1_future.wait_for(1s) == std::future_status::timeout) {
                FAIL("Timeout");
            }

            if (task_2_future.wait_for(1s) == std::future_status::timeout) {
                FAIL("Timeout");
            }

            REQUIRE (task_1_future.get());
            REQUIRE (task_2_future.get());
        }

        SECTION ("Enqueue with params") {
            int const EXPECTED_VALUE = 123;

            std::promise<int> task_promise_value;
            auto task_future_value = task_promise_value.get_future();

            dispatcher.enqueue(
                    [&task_promise_value](int value) {
                        task_promise_value.set_value(value);
                    },
                    EXPECTED_VALUE);

            if (task_future_value.wait_for(1s) == std::future_status::timeout) {
                FAIL("Future timedout");
            }

            REQUIRE (task_future_value.get() == EXPECTED_VALUE);
        }

    }

}