#pragma once
#ifndef LAS_VIEW_HPP
#define LAS_VIEW_HPP

#include <vector>
#include <stdexcept>

namespace las {

    namespace details {

        template < typename value_t >
        struct basic_view_traits {
        public:
            using value_type        = std::decay_t < value_t >;

            using pointer           = value_type*;
            using const_pointer     = value_type const*;

            using reference         = value_type&;
            using const_reference   = value_type const&;

            using iterator          = pointer;
            using const_iterator    = const_pointer;

            using size_type         = std::size_t;
        };

        template < typename value_t >
        struct basic_view_traits < value_t const > {
        public:
            using value_type        = std::decay_t < value_t > const;

            using pointer           = value_type*;
            using const_pointer     = pointer;

            using reference         = value_type&;
            using const_reference   = reference;

            using iterator          = const_pointer;
            using const_iterator    = const_pointer;

            using size_type         = std::size_t;
        };

        template < typename value_t >
        struct basic_view_storage {
        public:

            using traits            = basic_view_traits < value_t >;
            using pointer           = typename traits::pointer;
            using size_type         = typename traits::size_type;

            inline constexpr basic_view_storage() = default;
            inline constexpr basic_view_storage(pointer data, size_type size) :
                _data(data),
                _size(size)
            {}

            inline bool empty() const noexcept { return _size == 0 || _data == nullptr; }
            inline pointer data() const noexcept { return _data; }
            inline size_type size() const noexcept { return _size; }

        private:
            pointer     _data = nullptr;
            size_type   _size = 0;
        };

        template < typename value_t >
        struct basic_const_view : public basic_view_storage < value_t > {
        private:
            using base_class = basic_view_storage < value_t >;
        public:

            using base_class::base_class;
            using traits            = basic_view_traits < value_t >;

            using value_type        = typename traits::value_type;
            using pointer           = typename traits::pointer;
            using const_pointer     = typename traits::const_pointer;
            using const_iterator    = typename traits::const_iterator;
            using const_reference   = typename traits::const_reference;
            using size_type         = typename traits::size_type;

            inline constexpr basic_const_view() = default;

            inline constexpr basic_const_view(pointer begin, pointer end) :
                base_class(begin, end - begin)
            {}

            template < typename ... others_t >
            inline constexpr explicit basic_const_view(std::vector < value_type, others_t... > & vect) :
                base_class(vect.data(), vect.size())
            {}

            template < typename ... others_t >
            inline constexpr explicit basic_const_view(std::vector < std::decay_t < value_type >, others_t... > const & vect) :
                base_class(vect.data(), vect.size())
            {}

            template < size_type ARRAY_SIZE >
            inline constexpr explicit basic_const_view(value_type(&array)[ARRAY_SIZE]) :
                base_class(array, ARRAY_SIZE)
            {}

            inline constexpr const_iterator begin() const noexcept {
                return this->data();
            }

            inline constexpr const_iterator end() const noexcept {
                return this->data() + this->size();
            }

            inline constexpr const_iterator cbegin() const noexcept {
                return this->begin();
            }

            inline constexpr const_iterator cend() const noexcept {
                return this->end();
            }

            inline constexpr const_reference operator [](size_type const INDEX) const noexcept {
                return *(this->data() + INDEX);
            }

            inline constexpr const_reference at(size_type const INDEX) const {
                if (INDEX >= this->size()) {
                    throw std::out_of_range("view::at: pos value out of range");
                }

                return *(this->data() + INDEX);
            }

        };
    }

    template < typename value_t >
    struct view : public details::basic_const_view < value_t > {
    private:
        using base_class = details::basic_const_view < value_t >;
    public:
        using base_class::base_class;
        using traits        = typename base_class::traits;

        using iterator      = typename traits::iterator;
        using reference     = typename traits::reference;
        using size_type     = typename traits::size_type;

        using base_class::begin;
        using base_class::end;
        using base_class::operator[];
        using base_class::at;

        inline constexpr iterator begin() noexcept {
            return this->data();
        }

        inline constexpr iterator end() noexcept {
            return this->data() + this->size();
        }

        inline constexpr reference operator [](size_type const INDEX) noexcept {
            return *(this->data() + INDEX);
        }

        inline constexpr reference at(size_type const INDEX) {
            if (INDEX >= this->size()) {
                throw std::out_of_range("view::at: pos value out of range");
            }

            return *(this->data() + INDEX);
        }

    };

    template < typename value_t >
    struct view < value_t const > : public details::basic_const_view < value_t const > {
    private:
        using base_class = details::basic_const_view < value_t const >;
    public:
        using base_class::base_class;
    };


    /** deduction guides **/
    template < typename value_t >
    view(value_t *, std::size_t) -> view < value_t >;

    template < typename value_t >
    view(value_t *, value_t *) -> view < value_t >;

    template < typename value_t, std::size_t ARRAY_SIZE >
    view(value_t(&)[ARRAY_SIZE]) -> view < value_t >;
    
    template < typename value_t, typename ... others_t >
    view(std::vector < value_t, others_t... >&) -> view < value_t >;

    template < typename value_t, typename ... others_t >
    view(std::vector < value_t, others_t... > const &) -> view < value_t const >;

}

#endif
