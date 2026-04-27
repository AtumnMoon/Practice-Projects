#pragma once

#include <compare>
#include <format>
#include <ostream>
#include <string>
#include <string_view>

struct Date {
    int year{};
    int month{};
    int day{};

    std::strong_ordering operator<=>(const Date&) const = default;
};

std::ostream& operator<<(std::ostream& os, const Date& d);

template <>
struct std::formatter<Date> : std::formatter<std::string_view> {
    // This function tells C++ how to format your Date struct
    auto format(const Date& d, std::format_context& ctx) const {
        // We format it into a temporary string
        // {:02} ensures 01 instead of 1 for months/days
        std::string s = std::format("{:02}-{:02}-{}", d.month, d.day, d.year);

        // Then we delegate the actual writing to the base string_view formatter
        return std::formatter<std::string_view>::format(s, ctx);
    }
};

Date today();
Date add_days(const Date& date, int days);
