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

    struct dispatcher : no_copy {
    public:

        virtual ~dispatcher () = default;

        struct task_proxy : no_copy {
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
                inline explicit task_handle (ft_t && origin) :
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

            inline void invoke () const {
                if (handle) {
                    handle->invoke();
                }
            }

            void swap (task_proxy & other) {
                std::swap (handle, other.handle);
            }

            std::unique_ptr < task_handle_base > handle;

            template < typename return_t >
            static inline task_proxy make (std::packaged_task < return_t () > && package) {
                return task_proxy {
                        std::make_unique < task_handle <
                                std::packaged_task < return_t () > > > (
                                std::move(package))};
            }
        };

        template < typename func_t, typename ... args_t >
        inline void enqueue (func_t && func, args_t && ... args)
        {
            auto package = std::packaged_task < std::result_of_t < func_t (args_t...) > () > (
                    std::bind (std::forward < func_t > (func), std::forward < args_t > (args)...)
            );

            enqueue_task(task_proxy::make (std::move(package)));
        }

    protected:
        virtual void enqueue_task (task_proxy && proxy) = 0;
    };

    struct sync_dispatcher : public dispatcher {
    public:

        [[nodiscard]]
        bool is_done () const;
        void dispatch ();

    protected:
        void enqueue_task (task_proxy && proxy) override;
    private:

        locked_value < std::vector < dispatcher::task_proxy > >
            _queued_tasks,
            _exec_tasks;
    };

    struct async_dispatcher : public dispatcher {
    public:

        explicit async_dispatcher (std::size_t thread_count);
        ~async_dispatcher() override;

        void join ();

    protected:
        void enqueue_task (task_proxy && proxy) override;
    private:

        std::vector < std::thread >                             _worker_threads;

        locked_value <std::queue < dispatcher::task_proxy >, std::mutex>
                                                                _tasks;

        std::condition_variable	                                _exec_condition;
        std::atomic_bool 			                            _is_running { true };

    };

}

#endif
