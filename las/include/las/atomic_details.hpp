#pragma once
#ifndef LAS_ATOMIC_DETAILS_HPP
#define LAS_ATOMIC_DETAILS_HPP

#include <atomic>
#include <immintrin.h>
#include <type_traits>

namespace las::atomic_details {

    template < typename, typename = void >
    struct is_stack_node : std::false_type {};

    template < typename type >
    struct is_stack_node < type, std::void_t <decltype (std::declval <type>().next) >> {
        static constexpr bool value = std::is_same_v <decltype(std::declval <type>().next), type * >;
    };

    /// checks if a type fulfills the stack node concept
    /// \tparam type the type to check
    /// \note a stack node must have a member with the signature <c>type * type::next</c>
    template < typename type >
    constexpr bool is_stack_node_v = is_stack_node < type >::value;

	/// compare and swap with acquire and relaxed memory order
	/// \tparam type the type of the atomic variable
	/// \param target the atomic variable to compare and swap
	/// \param expected the expected value
	/// \param desired the desired value
	/// \return true if the compare and swap was successful, false otherwise
	template < typename type >
	bool compare_and_swap (std::atomic < type > & target, type & expected, type desired) {
		return target.compare_exchange_weak (expected, desired, std::memory_order_acquire, std::memory_order_relaxed);
	}

	/// hooks a node to the head of a linked list
	/// \tparam node_type the type of the node
	/// \param next_field address offset for the next field
	/// \param head the head of the linked list
	/// \param node_ptr the node to hook
	template < typename node_type >
	void atomic_push (node_type * node_type::*next_field, std::atomic < node_type * > & head, node_type * node_ptr) {
		node_ptr->*next_field = head.load (std::memory_order_relaxed);

		// lets go for the optimistic approach
		if (compare_and_swap (head, node_ptr->*next_field, node_ptr)) {
			return;
		}

		// lets go for the pessimistic approach
		while (!compare_and_swap (head, node_ptr->*next_field, node_ptr)) {
			_mm_pause();
		}
	}

	/// hooks a node to the head of a linked list
	/// \tparam node_type the type of the node
	/// \param head the head of the linked list
	/// \param node_ptr the node to hook
	template < typename node_type >
	void atomic_push (std::atomic < node_type * > & head, node_type * node_ptr) {
		static_assert (atomic_details::is_stack_node_v < node_type >, "Node type does not match node requirements");
		atomic_push (&node_type::next, head, node_ptr);
	}

	/// hooks a sequence of nodes to the head of a linked list
	/// \tparam node_type the type of the node
	/// \param next_field address offset for the next field
	/// \param head the head of the linked list
	/// \param first the first node of the sequence
	/// \param last the last node of the sequence
	template < typename node_type >
	void atomic_insert_at_head (node_type * node_type::*next_field, std::atomic < node_type * > & head, node_type * first, node_type * last) {
		last->*next_field = head.load (std::memory_order_relaxed);

		// lets go for the optimistic approach
		if (compare_and_swap (head, last->*next_field, first)) {
			return;
		}

		// lets go for the pessimistic approach
		while (!compare_and_swap (head, last->*next_field, first)) {
			_mm_pause();
		}
	}

	/// hooks a sequence of nodes to the head of a linked list
	/// \tparam node_type the type of the node
	/// \param head the head of the linked list
	/// \param first the first node of the sequence
	/// \param last the last node of the sequence
	template < typename node_type >
	void atomic_insert_at_head (std::atomic < node_type * > & head, node_type * first, node_type * last) {
		static_assert (atomic_details::is_stack_node_v < node_type >, "Node type does not match node requirements");
		atomic_insert_at_head (&node_type::next, head, first, last);
	}

	/// unhooks the top node of a linked list
	/// \tparam node_type the type of the node
	/// \param next_field address offset for the next field
	/// \param head the head of the linked list
	/// \return the unhooked node
	template < typename node_type >
	node_type * atomic_pop (node_type * node_type::*next_field, std::atomic < node_type * > & head) {
		auto * old_head = head.load (std::memory_order_relaxed);

		if (old_head) {
			// lets go for the optimistic approach
			if (compare_and_swap (head, old_head, old_head->*next_field)) {
				return old_head;
			}

			// lets go for the pessimistic approach
			while(old_head && !compare_and_swap (head, old_head, old_head->*next_field)) {
				_mm_pause();
			}
		}

		return old_head;
	}

	/// unhooks the top node of a linked list
	/// \tparam node_type the type of the node
	/// \param head the head of the linked list
	/// \return the unhooked node
	template < typename node_type >
	node_type * atomic_pop (std::atomic < node_type * > & head) {
		static_assert (atomic_details::is_stack_node_v < node_type >, "Node type does not match node requirements");
		return atomic_pop (&node_type::next, head);
	}

	/// unhooks the full linked list from the head
	/// \tparam node_type the type of the node
	/// \param head the head of the linked list
	/// \return the full linked list now derreferenced from the head
	template < typename node_type >
	node_type * atomic_detach (std::atomic < node_type * > & head) {
		return head.exchange (nullptr, std::memory_order_relaxed);
	}

	/// find the tail of a chain
	/// \tparam node_type the type of the node
	/// \param next_field address offset for the next field
	/// \param head the head of the chain
	/// \return the tail of the chain
	/// \note this function is NOT thread safe
	template < typename node_type >
	node_type * find_tail (node_type * node_type::*next_field, node_type * head) {
		node_type * tail = nullptr;

		while (head) {
			tail = head;
			head = head->*next_field;
		}

		return tail;
	}

	/// find the tail of a chain
	/// \tparam node_type the type of the node
	/// \param head the head of the chain
	/// \return the tail of the chain
	/// \note this function is NOT thread safe
	template < typename node_type >
	node_type * find_tail (node_type * head) {
		static_assert (atomic_details::is_stack_node_v < node_type >, "Node type does not match node requirements");
		return find_tail (&node_type::next, head);
	}

}

#endif
