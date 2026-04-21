#pragma once
#include <expected>
#include <string>
#include <string_view>

#include "errors.hpp"

void trim(std::string& str);

std::expected<std::string, NameError> get_name(std::string_view prompt);
std::expected<int, IntError> get_int(std::string_view prompt, int min, int max);
