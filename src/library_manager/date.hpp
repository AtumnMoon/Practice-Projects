#pragma once

#include <compare>

struct Date {
    int year{};
    int month{};
    int day{};

    std::strong_ordering operator<=>(const Date&) const = default;
};

Date today();
Date add_days(const Date& date, int days);
