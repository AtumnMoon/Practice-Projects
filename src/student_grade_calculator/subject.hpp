#pragma once
#include <cstdint>
#include <string>
#include <utility>

struct Subject {
    std::string name;
    uint8_t units;
    uint8_t grade;

    Subject(std::string name, uint8_t grade, uint8_t units)
        : name(std::move(name)), grade(grade), units(units) {}
};
