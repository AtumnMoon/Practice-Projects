#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "grade.hpp"
#include "report.hpp"
#include "subject.hpp"

void print_report(std::ostream& out, const std::string& student_name,
                  const std::vector<Subject>& subjects) {
    // Header box must fit "Grade Report: <name>" with 4 chars padding
    const size_t title_len =
        std::string_view("Grade Report: ").size() + student_name.size();
    const size_t width = std::max(title_len + 4, size_t(TABLE_WIDTH));

    // Header
    std::println(out, "╔{:═<{}}╗", "", width);
    std::println(out, "║{:^{}}║", "Grade Report: " + student_name, width);
    std::println(out, "╚{:═<{}}╝", "", width);
    std::println(out);

    // Column headers
    std::println(out, "{:<{}} {:<{}} {:<{}} {:<{}}", "Subject", COL_SUBJECT,
                 "Grade", COL_GRADE, "Units", COL_UNITS, "Remark", COL_REMARK);
    std::println(out, "{:-<{}} {:-<{}} {:-<{}} {:-<{}}", "", COL_SUBJECT, "",
                 COL_GRADE, "", COL_UNITS, "", COL_REMARK);

    // Subject rows
    for (const auto& s : subjects) {
        float grade = calculate_grade(s.grade);
        std::string remark = is_passing(grade) ? "Passed" : "Failed";

        // Truncate subject name if it overflows the column
        std::string display = s.name.length() > size_t(COL_SUBJECT)
                                  ? s.name.substr(0, COL_SUBJECT - 1) + "~"
                                  : s.name;

        std::println(out, "{:<{}} {:<{}.2f} {:<{}} {:<{}}", display,
                     COL_SUBJECT, grade, COL_GRADE, s.units, COL_UNITS, remark,
                     COL_REMARK);
    }

    std::println(out, "{:-<{}}", "", width);

    float gwa = calculate_gwa(subjects);

    // Best = lowest PUP grade number (1.0), worst = highest (5.0)
    const Subject* best = &subjects[0];
    const Subject* worst = &subjects[0];
    for (const auto& s : subjects) {
        if (calculate_grade(s.grade) < calculate_grade(best->grade)) best = &s;
        if (calculate_grade(s.grade) > calculate_grade(worst->grade))
            worst = &s;
    }

    std::println(out, "GWA            : {:.2f}", gwa);
    std::println(out, "Highest Grade  : {} ({:.2f})", best->name,
                 calculate_grade(best->grade));
    std::println(out, "Lowest Grade   : {} ({:.2f})", worst->name,
                 calculate_grade(worst->grade));
    std::println(out, "{:-<{}}", "", width);

    if (is_overall_passing(subjects)) {
        std::println(out, "Overall Status : PASSED");
    } else {
        int failed_count = 0;
        for (const auto& s : subjects)
            if (!is_passing(calculate_grade(s.grade))) ++failed_count;

        if (failed_count > 0) {
            std::println(out,
                         "Overall Status : FAILED — {} subject(s) not passed",
                         failed_count);
        } else {
            // All subjects passed individually but GWA missed the threshold
            std::println(
                out,
                "Overall Status : FAILED — GWA did not meet passing threshold");
        }
    }
}

void save_report(const std::string& student_name,
                 const std::vector<Subject>& subjects) {
    // Build safe filename: lowercase, spaces to underscores, strip
    // non-alphanumeric
    std::string filename = student_name;
    std::transform(
        filename.begin(), filename.end(), filename.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::replace(filename.begin(), filename.end(), ' ', '_');
    filename.erase(std::remove_if(filename.begin(), filename.end(),
                                  [](unsigned char c) {
                                      return !std::isalnum(c) && c != '_';
                                  }),
                   filename.end());
    filename += ".txt";

    std::ofstream file(filename);
    if (!file) {
        std::println(std::cerr, "Failed to create file: {}", filename);
        return;
    }

    print_report(file, student_name, subjects);
    std::println("Report saved to {}", filename);
}
