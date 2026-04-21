#include <cstddef>
#include <expected>
#include <iostream>
#include <limits>
#include <print>
#include <string>
#include <string_view>
#include "errors.hpp"

#include "input.hpp"

void trim(std::string& str) {
    constexpr std::string_view delimiter = " \t";
    size_t start = str.find_first_not_of(delimiter);
    size_t end = str.find_last_not_of(delimiter);
    if (start != std::string::npos) {
        str = str.substr(start, end - start + 1);
    } else {
        str.clear();
    }
}

std::expected<std::string, NameError> get_name(std::string_view prompt) {
    std::string name;
    std::print("{}", prompt);

    // Bail out on EOF or stream failure
    if (!std::getline(std::cin, name))
        return std::unexpected(NameError::StreamFailure);

    // Reject names over 64 characters
    if (name.length() > 64) return std::unexpected(NameError::NameTooLong);

    // Treat blank or whitespace-only as empty
    if (name.find_first_not_of(" \t") == std::string::npos)
        return std::unexpected(NameError::NameIsBlank);

    // Strip leading and trailing whitespace
    trim(name);
    return name;
}

std::expected<int, IntError> get_int(std::string_view prompt, int min,
                                     int max) {
    std::print("{}", prompt);
    int value;

    if (!(std::cin >> value)) {
        // Bail out on EOF or fatal stream failure
        if (std::cin.eof() || std::cin.bad())
            return std::unexpected(IntError::StreamFailure);

        // Handle failbit — recoverable bad input like "abc"
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return std::unexpected(IntError::InvalidInput);
    }

    // Flush the leftover newline before any subsequent getline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (value < min || value > max)
        return std::unexpected(IntError::OutOfRange);

    return value;
}
