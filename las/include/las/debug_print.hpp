#pragma once
#ifndef LAS_DEBUG_PRINT_HPP
#define LAS_DEBUG_PRINT_HPP

#ifdef NDEBUG

namespace las::debug {
	struct message {
		template < typename value_t >
		message & operator << (value_t const &) { return *this; }
	};
}

#define LAS_DEBUG_PRINT
#else

#include <iostream>
#include <sstream>
#include "las/details.hpp"

namespace las::debug {

	struct message : no_copy {

		template < class value_t >
		explicit message (value_t const & msg){
			_stream << msg;
		}

		message (message && other) noexcept :
			_stream (std::move (other._stream)),
			_active (other._active)
		{
			other.dismiss ();
		}

		template < class value_t >
		 message & operator << (const value_t & msg) {
			_stream << msg;
			return *this;
		}

		~message () {
			if (_active) {
				std::cerr << _stream.str () << '\n';
			}
		}

		void dismiss () { _active = false; }

	private:
		std::stringstream	_stream;
		bool				_active{true};
	};
}

#define LAS_DEBUG_PRINT las::debug::message ("[ " __FUNCTION__ " ] ")

#endif

#endif
