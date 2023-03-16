#pragma once
#ifndef LAS_VIEW_HPP
#define LAS_VIEW_HPP

#include <vector>
#include <stdexcept>

namespace las {

    template < typename type_t >
    using remove_all_modifiers_t = std::remove_cv_t < std::remove_reference_t < type_t > >;

    template < typename value_t >
    struct basic_const_view {
    public:

        using value_type =          remove_all_modifiers_t<value_t>;
        using const_reference =     value_type const &;
        using pointer =             value_type *;
        using const_pointer =       value_type const *;
        using size_type =           std::size_t;
        using const_iterator =      const_pointer;

        constexpr basic_const_view () noexcept = default;
        constexpr basic_const_view (basic_const_view const & other) noexcept = default;
        constexpr basic_const_view & operator = (basic_const_view const &) = default;

        inline constexpr basic_const_view (const_pointer ptr, size_type size) noexcept :
                ADDRESS (ptr),
                SIZE (size)
        {}

        /// Construct view from vector
        template < typename ... others_t >
        inline constexpr explicit basic_const_view (std::vector < value_type, others_t... > const & vec) :
                ADDRESS (vec.data ()),
                SIZE (vec.size())
        {}

        template < size_type SIZE >
        inline constexpr explicit basic_const_view (value_type const (&array)[SIZE]) :
                ADDRESS (array),
                SIZE (SIZE)
        {}

        inline constexpr const_iterator begin () const noexcept {
            return this->ADDRESS;
        }

        inline constexpr const_iterator end () const noexcept {
            return this->ADDRESS + SIZE;
        }

        inline constexpr const_iterator cbegin () const noexcept {
            return this->ADDRESS;
        }

        inline constexpr const_iterator cend () const noexcept {
            return this->ADDRESS + SIZE;
        }

        inline constexpr const_reference operator [](std::size_t const POS) const noexcept {
            return *(this->ADDRESS + POS);
        }

        inline constexpr const_reference at (std::size_t const POS) const {
            if (POS >= this->SIZE) {
                throw std::out_of_range("view::at: pos value out of range");
            }

            return *(this->ADDRESS + POS);
        }

        inline constexpr const_reference front () const noexcept {
            return *(this->ADDRESS);
        }

        inline constexpr const_reference back () const noexcept {
            return *(this->ADDRESS + (this->SIZE - 1));
        }

        inline constexpr const_pointer data () const noexcept {
            return ADDRESS;
        }

        [[nodiscard]]
        inline constexpr size_type size () const noexcept {
            return SIZE;
        }

        [[nodiscard]]
        inline constexpr bool empty () const noexcept {
            return this->SIZE == 0;
        }

    protected:
        pointer const       ADDRESS = nullptr;
        size_type const     SIZE = 0;
    };


    /// Simple non owning data view
    template < typename value_t >
    struct view : public basic_const_view < value_t > {
    public:

        using value_type =          remove_all_modifiers_t<value_t>;
        using reference =           value_type &;
        using const_reference =     value_type const &;
        using pointer =             value_type *;
        using const_pointer =       value_type const *;
        using size_type =           std::size_t;
        using iterator =            pointer;
        using const_iterator =      const_pointer;

        using basic_const_view < value_t >::basic_const_view;

        inline constexpr view (pointer ptr, size_type size) noexcept :
                basic_const_view < value_t >::ADDRESS (ptr),
                basic_const_view < value_t >::SIZE (size)
        {}

        /// Construct view from vector
        template < typename ... others_t >
        inline constexpr explicit view (std::vector < value_type, others_t... > & vec) :
                basic_const_view < value_t >::ADDRESS (vec.data ()),
                basic_const_view < value_t >::SIZE (vec.size())
        {}

        template < size_type ARRAY_SIZE >
        inline constexpr explicit view (value_type (&array)[ARRAY_SIZE]) :
                basic_const_view < value_t >::ADDRESS (array),
                basic_const_view < value_t >::SIZE (ARRAY_SIZE)
        {}

        inline iterator begin () noexcept {
            return this->ADDRESS;
        }


        inline iterator end () noexcept {
            return this->ADDRESS + this->SIZE;
        }

        inline reference operator [](std::size_t const POS) noexcept {
            return *(this->ADDRESS + POS);
        }

        inline reference at (std::size_t const POS) {
            if (POS >= this->SIZE) {
                throw std::out_of_range("view::at: pos value out of range");
            }

            return *(this->ADDRESS + POS);
        }

        inline reference front () noexcept {
            return *(this->ADDRESS);
        }

        inline reference back () noexcept {
            return *(this->ADDRESS + (this->SIZE - 1));
        }

        inline pointer data () noexcept {
            return this->ADDRESS;
        }
    };

    template < typename value_t >
    struct view < const value_t > : public basic_const_view < value_t > {
    public:
        using basic_const_view < value_t >::basic_const_view;
    };

}

#endif //LAS_VIEW_HPP
