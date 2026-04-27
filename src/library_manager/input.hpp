#pragma once

// I need helper functions that can
// - read integer
// - read unsigned integer
// - read string (series of char maybe?)
//
// Let's make it pure functional like there are no mutations or side-effects

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include "date.hpp"

enum class IntError { EndOfFile, StreamFailure, InvalidInput };
std::expected<int, IntError> read_int(std::string_view prompt);

enum class UnsignedError {
    EndOfFile,
    StreamFailure,
    InvalidInput,
    IntegerOverflow
};
std::expected<uint32_t, UnsignedError> read_unsigned(std::string_view prompt);

enum class StringError {
    EndOfFile,
    StreamFailure,
    InvalidInput,
};
std::expected<std::string, StringError> read_string(std::string_view prompt);

enum class DateError {
    EndOfFile,
    StreamFailure,
    InvalidInput,
};

std::expected<Date, DateError> read_date(std::string_view prompt);

void wait_for_enter();
