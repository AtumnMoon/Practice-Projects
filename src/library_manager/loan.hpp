#pragma once

#include <cstdint>
#include <optional>

#include "date.hpp"

struct Loan {
    uint32_t book_id{};
    uint32_t member_id{};
    Date borrow_date{};
    Date due_date{};
    std::optional<Date> return_date;

    bool is_returned() const { return return_date.has_value(); }
};
