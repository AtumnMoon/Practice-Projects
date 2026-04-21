#include <chrono>
#include <ctime>

#include "date.hpp"

Date today() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

#if defined(_WIN32)
    std::tm local_tm{};
    localtime_s(&local_tm, &t);  // Windows — arguments are reversed
    return Date{local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday};
#else
    std::tm local_tm{};
    localtime_r(&t, &local_tm);  // Linux/macOS
    return Date{local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday};
#endif
}

Date add_days(const Date& date, int days) {
    std::tm local_tm{};
    local_tm.tm_sec = 0;
    local_tm.tm_min = 0;
    local_tm.tm_hour = 0;
    local_tm.tm_mday = date.day + days;
    local_tm.tm_mon = date.month - 1;
    local_tm.tm_year = date.year - 1900;
    local_tm.tm_isdst = -1;

    std::mktime(&local_tm);

    return Date{local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday};
}
