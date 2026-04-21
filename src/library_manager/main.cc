#include <cstddef>
#include <cstdint>
#include <ctime>
#include <expected>
#include <iostream>
#include <limits>
#include <print>
#include <string>
#include <string_view>
#include <utility>

#include "date.hpp"
#include "library.hpp"
#include "loan.hpp"

// ── input helpers ────────────────────────────────────────────────────────────

enum class ReadError { StreamFailed, InvalidInput, OutOfRange };

std::expected<int, ReadError> read_int(std::string_view prompt) {
    int result = 0;

    std::print("{}", prompt);
    if (!(std::cin >> result)) {
        if (std::cin.eof() || std::cin.bad())
            return std::unexpected(ReadError::StreamFailed);

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return std::unexpected(ReadError::InvalidInput);
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return result;
}

std::expected<std::string, ReadError> read_line(std::string_view prompt) {
    std::string result;

    std::print("{}", prompt);
    if (!std::getline(std::cin, result))
        return std::unexpected(ReadError::StreamFailed);

    size_t begin = result.find_first_not_of(" \t");
    size_t end = result.find_last_not_of(" \t");

    if (begin == std::string::npos) {
        result.clear();
        return result;
    }

    result = result.substr(begin, end - begin + 1);
    return result;
}

std::expected<std::string, ReadError> read_string(std::string_view prompt) {
    while (true) {
        auto res = read_line(prompt);
        if (!res) return std::unexpected(res.error());
        if (!res->empty()) return res;
        std::println(std::cerr, "Input cannot be empty. Please try again.");
    }
}

std::expected<uint32_t, ReadError> read_uint32(std::string_view prompt) {
    auto res = read_int(prompt);
    if (!res) return std::unexpected(res.error());
    if (*res < 0) return std::unexpected(ReadError::OutOfRange);
    return static_cast<uint32_t>(*res);
}

// ── date helper ──────────────────────────────────────────────────────────────

enum class DateError { InvalidDate, StreamFailed };

std::expected<Date, DateError> read_date(std::string_view prompt) {
    std::println("{}", prompt);

    auto ask = [](std::string_view label) -> std::expected<int, DateError> {
        auto res = read_int(label);
        if (!res) return std::unexpected(DateError::StreamFailed);
        return *res;
    };

    auto y = ask("  Year : ");
    if (!y) return std::unexpected(y.error());
    auto m = ask("  Month: ");
    if (!m) return std::unexpected(m.error());
    auto d = ask("  Day  : ");
    if (!d) return std::unexpected(d.error());

    // Validation via mktime (fixes overflow)
    std::tm t{};
    t.tm_year = *y - 1900;
    t.tm_mon = *m - 1;
    t.tm_mday = *d;
    t.tm_isdst = -1;

    if (std::mktime(&t) == -1 || t.tm_year != *y - 1900 || t.tm_mon != *m - 1 ||
        t.tm_mday != *d) {
        return std::unexpected(DateError::InvalidDate);
    }
    return Date{*y, *m, *d};
}

// ── handlers ─────────────────────────────────────────────────────────────────

bool handle_add_member(Library& library) {
    auto name_res = read_string("Member Name: ");

    // If the stream failed (EOF/Bad), signal main to exit
    if (!name_res) return false;

    auto result = library.add_member(std::move(*name_res));
    if (!result) {
        switch (result.error()) {
            case AddMemberError::NameAlreadyExists:
                std::println(
                    std::cerr,
                    "Error: A member with the name '{}' already exists.",
                    *name_res);
                break;
        }
        return true;  // Keep running the app
    }

    std::println("Member added successfully.");
    return true;
}

bool handle_add_book(Library& library) {
    auto title = read_string("Title : ");
    if (!title) return false;

    auto author = read_string("Author: ");
    if (!author) return false;

    auto date = read_date("Publication date:");
    if (!date) {
        if (date.error() == DateError::StreamFailed) return false;
        std::println(std::cerr, "Invalid date — book not added.");
        return true;
    }

    library.add_book(std::move(*title), std::move(*author), *date);
    std::println("Book added successfully.");
    return true;
}

bool handle_list_books(Library& library) {
    const auto& books = library.list_all_books();
    if (books.empty()) {
        std::println("No books in the library.");
        return true;
    }

    std::println("\n{:<6} {:<30} {:<20} {}", "ID", "Title", "Author",
                 "Published");
    std::println("{:-<6} {:-<30} {:-<20} {:-<10}", "", "", "", "");
    for (const auto& b : books) {
        std::println(
            "{:<6} {:<30} {:<20} {:04}-{:02}-{:02}", b.id,
            b.title.length() > 29 ? b.title.substr(0, 28) + "~" : b.title,
            b.author.length() > 19 ? b.author.substr(0, 18) + "~" : b.author,
            b.date_published.year, b.date_published.month,
            b.date_published.day);
    }
    std::println();
    return true;
}

bool handle_list_members(Library& library) {
    const auto& members = library.list_all_members();
    if (members.empty()) {
        std::println("No members registered.");
        return true;
    }

    std::println("\n{:<6} {}", "ID", "Name");
    std::println("{:-<6} {:-<30}", "", "");
    for (const auto& m : members) {
        std::println("{:<6} {}", m.id, m.name);
    }
    std::println();
    return true;
}

bool handle_borrow_book(Library& library) {
    // Show context to the user
    handle_list_members(library);
    auto m_res = read_uint32("Member ID (0 to cancel): ");
    if (!m_res) return (m_res.error() != ReadError::StreamFailed);
    if (*m_res == 0) return true;

    handle_list_books(library);
    auto b_res = read_uint32("Book ID (0 to cancel)  : ");
    if (!b_res) return (b_res.error() != ReadError::StreamFailed);
    if (*b_res == 0) return true;

    // Attempt the domain logic
    auto result = library.borrow_book(*m_res, *b_res);

    if (!result) {
        switch (result.error()) {
            case BorrowError::MemberNotFound:
                std::println(std::cerr, "Member ID {} not found.", *m_res);
                break;
            case BorrowError::BookNotFound:
                std::println(std::cerr, "Book ID {} not found.", *b_res);
                break;
            case BorrowError::AlreadyOnLoan:
                std::println(std::cerr, "Book is already checked out.");
                break;
        }
        return true;
    }

    // Success
    const Loan& loan = *result;
    std::println("Success! Due date: {:04}-{:02}-{:02}", loan.due_date.year,
                 loan.due_date.month, loan.due_date.day);
    return true;
}

bool handle_return_book(Library& library) {
    auto m_res = read_uint32("Member ID: ");
    if (!m_res) return (m_res.error() != ReadError::StreamFailed);

    auto b_res = read_uint32("Book ID  : ");
    if (!b_res) return (b_res.error() != ReadError::StreamFailed);

    auto result = library.return_book(*m_res, *b_res);
    if (!result) {
        switch (result.error()) {
            case ReturnError::MemberNotFound:
                std::println(std::cerr, "Member not found.");
                break;
            case ReturnError::BookNotFound:
                std::println(std::cerr, "Book not found.");
                break;
            case ReturnError::NotOnLoan:
                std::println(std::cerr, "This book is not currently on loan.");
                break;
            case ReturnError::NotYourLoan:
                std::println(std::cerr,
                             "This book was borrowed by someone else.");
                break;
        }
        return true;
    }

    std::println("Book returned successfully.");
    return true;
}

bool handle_active_loans(Library& library) {
    const auto loans = library.get_all_active_loans();
    if (loans.empty()) {
        std::println("No active loans.");
        return true;
    }

    std::println("\n{:<8} {:<8} {:<12} {}", "Book ID", "Mbr ID", "Borrowed",
                 "Due");
    std::println("{:-<8} {:-<8} {:-<12} {:-<10}", "", "", "", "");
    for (const auto& l : loans) {
        std::println("{:<8} {:<8} {:04}-{:02}-{:02}  {:04}-{:02}-{:02}",
                     l.book_id, l.member_id, l.borrow_date.year,
                     l.borrow_date.month, l.borrow_date.day, l.due_date.year,
                     l.due_date.month, l.due_date.day);
    }
    std::println();
    return true;
}

bool handle_overdue_loans(Library& library) {
    const auto loans = library.get_all_overdue_loans();
    if (loans.empty()) {
        std::println("No overdue loans.");
        return true;
    }

    std::println("\n--- Overdue loans ---");
    for (const auto& l : loans) {
        std::println("Book {:>4} | Member {:>4} | Due: {:04}-{:02}-{:02}",
                     l.book_id, l.member_id, l.due_date.year, l.due_date.month,
                     l.due_date.day);
    }
    std::println();
    return true;
}

// ── menu ─────────────────────────────────────────────────────────────────────

void print_menu() {
    std::println("\n{:=^30}", "");
    std::println("{:^30}", "Library Management");
    std::println("{:=^30}", "");
    std::println("1. Add member");
    std::println("2. Add book");
    std::println("3. List members");
    std::println("4. List books");
    std::println("5. Borrow a book");
    std::println("6. Return a book");
    std::println("7. View active loans");
    std::println("8. View overdue loans");
    std::println("0. Exit");
}

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    Library library;

    while (true) {
        print_menu();

        auto choice_res = read_int("Choice: ");
        if (!choice_res) {
            if (choice_res.error() == ReadError::StreamFailed) break;
            std::println(std::cerr, "Invalid input. Please enter a number.");
            continue;
        }

        int choice = *choice_res;
        if (choice == 0) break;

        bool keep_running = true;
        switch (choice) {
            case 1:
                keep_running = handle_add_member(library);
                break;
            case 2:
                keep_running = handle_add_book(library);
                break;
            case 3:
                keep_running = handle_list_members(library);
                break;
            case 4:
                keep_running = handle_list_books(library);
                break;
            case 5:
                keep_running = handle_borrow_book(library);
                break;
            case 6:
                keep_running = handle_return_book(library);
                break;
            case 7:
                keep_running = handle_active_loans(library);
                break;
            case 8:
                keep_running = handle_overdue_loans(library);
                break;
            default:
                std::println(std::cerr, "Unknown option.");
                break;
        }

        if (!keep_running) break;
    }

    std::println("Goodbye!");
    return 0;
}
