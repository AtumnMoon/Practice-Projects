#pragma once
#include <vector>

#include "subject.hpp"

float calculate_grade(int percentage);
float calculate_gwa(const std::vector<Subject>& subjects);
bool is_passing(float grade);
bool is_overall_passing(const std::vector<Subject>& subjects);
