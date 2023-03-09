#pragma once
#ifndef LAS_STRING_HPP
#define LAS_STRING_HPP

#include <algorithm>
#include <charconv>
#include <cstring>
#include <locale>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace las::string {

    inline void
    split_into(std::string_view view, std::string_view delimiter, std::vector<std::string_view> &out_vector) {
        std::size_t offset = 0;

        while (offset < view.size()) {
            auto pos = view.find_first_of(delimiter, offset);

            if (pos == std::string_view::npos) {
                out_vector.emplace_back(view.substr(offset, view.size()));
                break;
            }

            if (pos != offset) {
                out_vector.emplace_back(view.substr(offset, pos - offset));
            }

            offset = pos + delimiter.size();
        }
    }

    inline std::vector<std::string_view> split(std::string_view view, std::string_view delimiter) {
        std::vector<std::string_view> result;
        split_into(view, delimiter, result);

        return result;
    }

    template<typename predicate_t>
    inline std::string_view trim_left(std::string_view view, predicate_t predicate) {

        auto char_it = std::find_if_not(
                view.begin(), view.end(),
                predicate);

        if (char_it == view.end() || *char_it == 0) {
            return {};
        }

        view.remove_prefix(char_it - view.begin());

        return view;
    }

    template<typename predicate_t>
    inline std::string_view trim_right(std::string_view view, predicate_t predicate) {
        std::locale const LOC{};

        auto char_it = std::find_if_not(
                view.rbegin(), view.rend(),
                predicate);

        if (char_it == view.rend() || *char_it == 0) {
            return {};
        }

        view.remove_suffix(char_it - view.rbegin());

        return view;
    }

    template<typename predicate_t>
    inline std::string_view trim(std::string_view view, predicate_t predicate) {
        return trim_left(trim_right(view, predicate), predicate);
    }

    inline std::string_view trim_left(std::string_view view, char token) {
        return trim_left(view, [token](char char_v) { return char_v == token; });
    }

    inline std::string_view trim_right(std::string_view view, char token) {
        return trim_right(view, [token](char char_v) { return char_v == token; });
    }

    inline std::string_view trim(std::string_view view, char token) {
        return trim_left(trim_right(view, token), token);
    }

    inline std::string_view trim_left(std::string_view view) {
        std::locale loc{};
        return trim_left(view, [&loc](char char_v) { return !std::isgraph(char_v, loc); });
    }

    inline std::string_view trim_right(std::string_view view) {
        std::locale loc{};
        return trim_right(view, [&loc](char char_v) { return !std::isgraph(char_v, loc); });
    }

    inline std::string_view trim(std::string_view view) {
        return trim_left(trim_right(view));
    }

    template<typename num_t>
    inline std::optional<num_t> as_number(std::string_view value) {
        num_t num{};

        if (auto [p, e] = std::from_chars(value.begin(), value.end(), num);
                e == std::errc()) {
            return num;
        }

        return std::nullopt;
    }

    template<>
    inline std::optional<double> as_number(std::string_view value) {

        try {
            return std::stod(std::string(value));
        } catch (...) {}

        return std::nullopt;
    }

    template<>
    inline std::optional<float> as_number(std::string_view value) {

        try {
            return std::stof(std::string(value));
        } catch (...) {}

        return std::nullopt;
    }

    inline std::size_t length(std::string const &str) {
        return str.length();
    }

    inline std::size_t length(char const *ptr) {
        return std::strlen(ptr);
    }

    template<std::size_t LEN>
    inline std::size_t length(char const (&)[LEN]) { // NOLINT: signature, not definition
        return LEN;
    }

    inline std::size_t length(std::string_view view) {
        return view.size();
    }

    template<typename ... str_t>
    inline std::string cat(str_t &&... values) {
        auto len = (string::length(values) + ...);

        std::string result;
        result.reserve(len);

        (result.append(values), ...);

        return result;
    }

    inline bool begins_with(std::string_view str, std::string_view token) {
        if (str.size() < token.size()) {
            return false;
        }

        return std::equal(token.begin(), token.end(), str.begin());
    }

    inline bool ends_with(std::string_view str, std::string_view token) {
        if (str.size() < token.size()) {
            return false;
        }

        return std::equal(token.rbegin(), token.rend(), str.rbegin());
    }

    inline bool contains(std::string_view str, std::string_view sub_str) {
        if (sub_str.empty() || sub_str.size() > str.size()) {
            return false;
        }

        return str.find(sub_str) != std::string::npos;
    }

    inline bool next_line(std::string_view &view, std::string_view &line, std::string_view delimiter) {
        auto index = view.find_first_of(delimiter);

        if (index != std::string_view::npos && index > 0) {
            line = view.substr(0, index);
            view.remove_prefix(index + delimiter.size());
            return true;
        }

        if (index == 0) {
            line = view.substr(0, 0);
            view.remove_prefix(delimiter.size());
            return true;
        }

        line = view;
        return false;

    }

    inline std::string_view parse_key_value_to(
            std::string_view view,
            std::unordered_map<std::string_view, std::string_view> &dest,
            std::string_view line_delimiter,
            std::string_view value_delimiter
    ) {
        std::string_view cursor = view;
        std::string_view line;

        while (next_line(cursor, line, line_delimiter)) {
            if (line.empty()) {
                break;
}

            std::string_view key;
            std::string_view value;

            if (next_line(line, key, value_delimiter)) {
                value = line;
                dest[trim(key)] = trim(value);
            }
        }

        return cursor;
    }

}

#endif
