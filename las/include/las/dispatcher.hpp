#pragma once
#ifndef LAS_DISPATCHER_HPP
#define LAS_DISPATCHER_HPP

#include "las/details.hpp"
#include "las/locked_value.hpp"

#include <functional>
#include <future>
#include <memory>
#include <queue>

namespace las {

    /// task dispatcher abstract class
    class dispatcher : public no_copy {
    public:
        virtual ~dispatcher () = default;

        /// Enqueue a task to be executed
        /// \tparam func_t callable type
        /// \tparam args_t arguments type vector
        /// \param func callable object
        /// \param args arguments
        template < typename func_t, typename ... args_t >
        void enqueue (func_t && func, args_t && ... args)
        {
            auto package = std::packaged_task < std::result_of_t < func_t (args_t...) > () > (
                    std::bind (std::forward < func_t > (func), std::forward < args_t > (args)...)
            );

            enqueue_task(task_proxy::make (std::move(package)));
        }

    protected:

        class task_proxy : no_copy {
        public:
            virtual ~task_proxy() = default;

            class task_handle_base {
            public:
                virtual ~task_handle_base () = default;
                virtual void invoke () = 0;
            };

            template < class ft_t >
            class task_handle : public ft_t, public task_handle_base {
            public:
                explicit task_handle (ft_t && origin) :
                        ft_t (std::forward < ft_t > (origin))
                {}

                void invoke () override {
                    std::invoke(*this);
                }
            };

            task_proxy() = default;
            explicit task_proxy(std::unique_ptr < task_handle_base > && handle) :
                    handle { std::move (handle) }
            {}

            task_proxy(task_proxy && other) noexcept { this->swap (other); }

            task_proxy & operator = (task_proxy && other) noexcept {
                this->swap (other);
                return *this;
            }

            void invoke () const {
                if (handle) {
                    handle->invoke();
                }
            }

            void swap (task_proxy & other) noexcept {
                std::swap (handle, other.handle);
            }

            std::unique_ptr < task_handle_base > handle;

            template < typename return_t >
            static task_proxy make (std::packaged_task < return_t () > && package) {
                return task_proxy {
                    std::make_unique < task_handle <
                            std::packaged_task < return_t () > > > (
                            std::move(package))};
            }
        };

        /// Enqueue a task to be executed
        /// \param proxy task to be executed
        virtual void enqueue_task (task_proxy && proxy) = 0;
    };

    /// synchronous task dispatcher
    class sync_dispatcher : public dispatcher {
    public:
        /// check if the dispatcher is done
        /// \return true if the dispatcher has no more tasks to execute
        [[nodiscard]] bool is_done () const;

        /// dispatch all enqueued tasks
        void dispatch ();
    protected:
        void enqueue_task (task_proxy && proxy) override;
    private:
        locked_value < std::vector < dispatcher::task_proxy > >
            _queued_tasks,
            _exec_tasks;
    };

    /// asynchronous task dispatcher
    class async_dispatcher : public dispatcher {
    public:
        /// dispatcher constructor
        /// \param thread_count number of threads to use
        explicit async_dispatcher (std::size_t thread_count);
        ~async_dispatcher() override;

        /// stop execution and join all threads
        /// \note automatically called by the destructor
        void join ();
    protected:
        void enqueue_task (task_proxy && proxy) override;
    private:

        std::vector < std::thread >     _worker_threads;

        locked_value <std::queue < dispatcher::task_proxy >, std::mutex>
                                        _tasks;
        std::condition_variable         _exec_condition;
        std::atomic_bool 		        _is_running { true };

    };

}

#endif
