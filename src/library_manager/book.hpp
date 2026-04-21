#pragma once

#include <cstdint>
#include <string>

#include "date.hpp"

struct Book {
    uint32_t id{};
    std::string title;
    std::string author;
    Date date_published{};
};
