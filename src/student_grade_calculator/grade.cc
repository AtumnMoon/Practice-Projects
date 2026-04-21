#include <vector>

#include "grade.hpp"
#include "subject.hpp"

float calculate_grade(int percentage) {
    if (percentage >= 97) return 1.0f;
    if (percentage >= 93) return 1.25f;
    if (percentage >= 89) return 1.5f;
    if (percentage >= 85) return 1.75f;
    if (percentage >= 81) return 2.0f;
    if (percentage >= 77) return 2.25f;
    if (percentage >= 73) return 2.5f;
    if (percentage >= 69) return 2.75f;
    if (percentage >= 65) return 3.0f;
    return 5.0f;
}

float calculate_gwa(const std::vector<Subject>& subjects) {
    float total_weighted = 0.0f;
    int total_units = 0;
    for (const auto& s : subjects) {
        total_weighted += calculate_grade(s.grade) * s.units;
        total_units += s.units;
    }
    return total_weighted / static_cast<float>(total_units);
}

bool is_passing(float grade) { return grade <= 3.0f; }

bool is_overall_passing(const std::vector<Subject>& subjects) {
    // Fails if any individual subject is 5.0, or if GWA exceeds 3.0
    for (const auto& s : subjects)
        if (!is_passing(calculate_grade(s.grade))) return false;
    return is_passing(calculate_gwa(subjects));
}
