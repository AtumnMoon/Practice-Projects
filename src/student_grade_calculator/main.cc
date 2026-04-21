#include <cstdint>
#include <iostream>
#include <print>
#include <string>
#include <utility>
#include <vector>

#include "errors.hpp"
#include "input.hpp"
#include "report.hpp"
#include "subject.hpp"

int main() {
    // Display the title
    std::println("╔{:═<30}╗", "");
    std::println("║{:^30}║", " Grade Report ");
    std::println("╚{:═<30}╝", "");

    // Ask for student name
    std::string student_name;
    while (student_name.empty()) {
        auto result = get_name("Student Name: ");
        if (result) {
            student_name = std::move(*result);
        } else {
            switch (result.error()) {
                case NameError::StreamFailure:
                    std::println(std::cerr, "Unexpected input error. Exiting.");
                    return 1;
                case NameError::NameTooLong:
                    std::println(std::cerr,
                                 "Name too long. Maximum 64 characters.");
                    break;
                case NameError::NameIsBlank:
                    std::println(std::cerr, "Name cannot be blank. Try again.");
                    break;
            }
        }
    }

    // Ask how many subjects
    int subject_count = 0;
    while (true) {
        auto result = get_int("How many subjects? ", 1, 20);
        if (result) {
            subject_count = *result;
            break;
        } else {
            switch (result.error()) {
                case IntError::StreamFailure:
                    std::println(std::cerr, "Unexpected input error. Exiting.");
                    return 1;
                case IntError::InvalidInput:
                    std::println(std::cerr,
                                 "Invalid input. Please enter a number.");
                    break;
                case IntError::OutOfRange:
                    std::println(std::cerr,
                                 "Please enter between 1 to 20 subjects.");
                    break;
            }
        }
    }

    // Collect input for each subject
    std::vector<Subject> subjects;
    subjects.reserve(subject_count);

    for (int i = 0; i < subject_count; ++i) {
        std::println("--- Subject {} ---", i + 1);

        // Get subject name
        std::string subject_name;
        while (subject_name.empty()) {
            auto result = get_name("Subject Name: ");
            if (result) {
                subject_name = std::move(*result);
            } else {
                switch (result.error()) {
                    case NameError::StreamFailure:
                        std::println(std::cerr,
                                     "Unexpected input error. Exiting.");
                        return 1;
                    case NameError::NameTooLong:
                        std::println(std::cerr,
                                     "Name too long. Maximum 64 characters.");
                        break;
                    case NameError::NameIsBlank:
                        std::println(std::cerr,
                                     "Name cannot be blank. Try again.");
                        break;
                }
            }
        }

        // Get subject grade (percentage 0-100)
        int grade = 0;
        while (true) {
            auto result = get_int("Grade (0-100): ", 0, 100);
            if (result) {
                grade = *result;
                break;
            } else {
                switch (result.error()) {
                    case IntError::StreamFailure:
                        std::println(std::cerr,
                                     "Unexpected input error. Exiting.");
                        return 1;
                    case IntError::InvalidInput:
                        std::println(std::cerr,
                                     "Invalid input. Please enter a number.");
                        break;
                    case IntError::OutOfRange:
                        std::println(std::cerr,
                                     "Please enter between 0 to 100.");
                        break;
                }
            }
        }

        // Get subject units (1-6 is realistic for university credits)
        int units = 0;
        while (true) {
            auto result = get_int("Units/Credits: ", 1, 6);
            if (result) {
                units = *result;
                break;
            } else {
                switch (result.error()) {
                    case IntError::StreamFailure:
                        std::println(std::cerr,
                                     "Unexpected input error. Exiting.");
                        return 1;
                    case IntError::InvalidInput:
                        std::println(std::cerr,
                                     "Invalid input. Please enter a number.");
                        break;
                    case IntError::OutOfRange:
                        std::println(std::cerr,
                                     "Please enter between 1 to 6 units.");
                        break;
                }
            }
        }

        std::println();
        subjects.push_back(Subject(std::move(subject_name),
                                   static_cast<uint8_t>(grade),
                                   static_cast<uint8_t>(units)));
    }

    // Print the grade report to console
    std::println();
    print_report(std::cout, student_name, subjects);

    // Ask to save report to file
    std::println();
    std::print("Save report to file? (y/n): ");
    std::string answer;
    if (std::getline(std::cin, answer) && !answer.empty() &&
        (answer[0] == 'y' || answer[0] == 'Y')) {
        save_report(student_name, subjects);
    }

    return 0;
}
