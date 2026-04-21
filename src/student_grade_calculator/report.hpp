#pragma once
#include <ostream>
#include <string>
#include <vector>

#include "subject.hpp"

// Column widths — 3 space separators between 4 columns are included in
// TABLE_WIDTH
constexpr int COL_SUBJECT = 22;
constexpr int COL_GRADE = 8;
constexpr int COL_UNITS = 7;
constexpr int COL_REMARK = 8;
constexpr int TABLE_WIDTH =
    COL_SUBJECT + COL_GRADE + COL_UNITS + COL_REMARK + 3;

void print_report(std::ostream& out, const std::string& student_name,
                  const std::vector<Subject>& subjects);

void save_report(const std::string& student_name,
                 const std::vector<Subject>& subjects);
