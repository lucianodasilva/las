#include "las/dispatcher.hpp"
#include "las/debug.hpp"
#include "las/locked_value.hpp"

namespace las {

    bool sync_dispatcher::is_done() const {
        auto locked_tasks = _queued_tasks.shared ();
        return locked_tasks.value ().empty();
    }

    void sync_dispatcher::dispatch() {
        auto locked_exec = _exec_tasks.unique(std::defer_lock);

        if (!locked_exec.lock ().try_lock()) {
            LAS_DEBUG_BREAK(); // Dispatch called as consequence of dispatched task unsupported. Call ignored!
            return;
        }

        {
            auto locked_queue = _queued_tasks.unique();
            std::swap (locked_exec.value (), locked_queue.value ());

            locked_queue.value ().resize (0);
        }

        for (auto & task : locked_exec.value ()) {
            task.invoke();
        }
    }

    void sync_dispatcher::enqueue_task(task_proxy &&proxy) {
        auto locked_tasks = _queued_tasks.unique ();
        locked_tasks.value ().emplace_back (std::forward < task_proxy > (proxy));
    }

    async_dispatcher::async_dispatcher(std::size_t thread_count) {
        for (std::size_t i = 0; i < thread_count; ++i) {
            _worker_threads.emplace_back([this] {
                for (;;) {
                    task_proxy task{};

                    {
                        auto locked_tasks = this->_tasks.unique ();

                        this->_exec_condition.wait(
                                locked_tasks.lock (),
                                [&] { return !locked_tasks.value().empty() || !this->_is_running; });

                        if (locked_tasks.value().empty() && !this->_is_running) {
                            return;
                        }

                        task = std::move(locked_tasks.value ().front());
                        locked_tasks.value ().pop();
                    }

                    task.invoke();
                }
            });
        }
    }

    async_dispatcher::~async_dispatcher() {
        join();
    }

    void async_dispatcher::enqueue_task(task_proxy &&proxy) {
        if (!this->_is_running) {
            LAS_DEBUG_BREAK(); // Enqueue tasks after shutdown not supported. Call ignored!
            return;
        }

        {
            auto locked_tasks = _tasks.unique ();
            locked_tasks.value ().push (std::forward < task_proxy >(proxy));
        }

        _exec_condition.notify_one();
    }

    void async_dispatcher::join() {
        _is_running = false;
        _exec_condition.notify_all();

        for (auto & worker: _worker_threads) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

}