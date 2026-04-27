#include <cstdint>
#include <cstdio>
#include <expected>
#include <ios>
#include <iostream>
#include <limits>
#include <print>
#include <string>
#include <string_view>

#include "date.hpp"
#include "input.hpp"

std::expected<int, IntError> read_int(std::string_view prompt) {
    std::print("{}", prompt);

    int value{};

    if (!(std::cin >> value)) {
        // Check for EOF
        if (std::cin.eof()) return std::unexpected(IntError::EndOfFile);

        // Check for stream failure
        if (std::cin.bad()) return std::unexpected(IntError::StreamFailure);

        // Maybe recoverable
        // First clear the state
        std::cin.clear();

        // Now clear the buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Then return the error
        return std::unexpected(IntError::InvalidInput);
    }
    // Now clear the buffer to ensure nothing remains
    // This is mainly for mixing this with 'read_string'
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Safe to return the value
    return value;
}

std::expected<uint32_t, UnsignedError> read_unsigned(std::string_view prompt) {
    std::print("{}", prompt);

    int value{};

    if (!(std::cin >> value)) {
        // Check for EOF
        if (std::cin.eof()) return std::unexpected(UnsignedError::EndOfFile);

        // Check for stream failure
        if (std::cin.bad())
            return std::unexpected(UnsignedError::StreamFailure);

        // Maybe recoverable
        // First clear the state
        std::cin.clear();

        // Now clear the buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Then return the error
        return std::unexpected(UnsignedError::InvalidInput);
    }
    // Now clear the buffer to ensure nothing remains
    // This is mainly for mixing this with 'read_string'
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Now that temp has a value, we make
    // sure that it's a proper unsigned int
    if (value < 0) return std::unexpected(UnsignedError::IntegerOverflow);

    return static_cast<uint32_t>(value);
}

std::expected<std::string, StringError> read_string(std::string_view prompt) {
    std::print("{}", prompt);

    std::string result;

    // Get the whole line including whitespaces
    if (std::getline(std::cin, result)) {
        return result;
    }

    // Check for EOF
    if (std::cin.eof()) return std::unexpected(StringError::EndOfFile);

    // Check for stream failure
    if (std::cin.bad()) return std::unexpected(StringError::StreamFailure);

    // Cleanup the buffer
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return std::unexpected(StringError::InvalidInput);
}

std::expected<Date, DateError> read_date(std::string_view prompt) {
    auto raw_input = read_string(prompt);

    // Handle underlying string errors
    if (!raw_input) {
        switch (raw_input.error()) {
            case StringError::EndOfFile:
                return std::unexpected(DateError::EndOfFile);
            case StringError::StreamFailure:
                return std::unexpected(DateError::StreamFailure);
            case StringError::InvalidInput:
                return std::unexpected(DateError::InvalidInput);
        }
    }

    // Try to parse "MM-DD-YYYY"
    int m, d, y;
    // sscanf returns the number of successfully filled variables
    if (std::sscanf(raw_input->c_str(), "%d-%d-%d", &m, &d, &y) != 3) {
        return std::unexpected(DateError::InvalidInput);
    }

    // Basic Validation
    if (m < 1 || m > 12 || d < 1 || d > 31 || y < 1900 || y > 2100) {
        return std::unexpected(DateError::InvalidInput);
    }

    return Date{y, m, d};
}

void wait_for_enter() {
    std::cin.clear();
    std::cin.sync();
    std::println("Press 'Enter' to continue...");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}
