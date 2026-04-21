#include <algorithm>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "book.hpp"
#include "date.hpp"
#include "library.hpp"
#include "loan.hpp"
#include "member.hpp"

// Library - Book Management
void Library::add_book(std::string title, std::string author,
                       Date date_published) {
    _books.push_back(Book{_next_book_id++, std::move(title), std::move(author),
                          std::move(date_published)});
}

std::expected<void, RemoveBookError> Library::remove_book(uint32_t book_id) {
    // Find book based on book_id
    auto book_it = std::find_if(
        _books.begin(), _books.end(),
        [book_id](const Book& book) { return book.id == book_id; });

    // Return an error if it doesn't exist
    if (book_it == _books.end()) {
        return std::unexpected(RemoveBookError::NotFound);
    }

    // Find book_id in Loans to check if it's currently loaned
    auto loan_it =
        std::find_if(_loans.begin(), _loans.end(), [book_id](const Loan& loan) {
            return loan.book_id == book_id && !loan.is_returned();
        });

    // Return an error if it's loaned
    if (loan_it != _loans.end()) {
        return std::unexpected(RemoveBookError::CurrentlyOnLoan);
    }

    // Safe to remove
    _books.erase(book_it);
    return {};
}

std::optional<Book> Library::find_book_by_id(uint32_t book_id) const {
    auto it = std::find_if(
        _books.begin(), _books.end(),
        [book_id](const Book& book) { return book.id == book_id; });

    if (it == _books.end()) {
        return std::nullopt;
    }

    return *it;
}

const std::vector<Book>& Library::list_all_books() const { return _books; }

// Library - Member Management
std::expected<void, AddMemberError> Library::add_member(std::string name) {
    // Find if a member with the same name exist
    // Prevent adding member with the same name
    auto it = std::find_if(
        _members.begin(), _members.end(),
        [&name](const Member& member) { return member.name == name; });

    // If found, return an error
    if (it != _members.end()) {
        return std::unexpected(AddMemberError::NameAlreadyExists);
    }

    // Safe to add
    _members.push_back(Member{_next_member_id++, std::move(name)});
    return {};
}

std::expected<void, RemoveMemberError> Library::remove_member(
    uint32_t member_id) {
    auto member_it = std::find_if(
        _members.begin(), _members.end(),
        [member_id](const Member& member) { return member.id == member_id; });

    if (member_it == _members.end()) {
        return std::unexpected(RemoveMemberError::NotFound);
    }

    auto loan_it = std::find_if(
        _loans.begin(), _loans.end(), [member_id](const Loan& loan) {
            return loan.member_id == member_id && !loan.is_returned();
        });

    // Prevent member from being remove while it has active loan
    if (loan_it != _loans.end()) {
        return std::unexpected(RemoveMemberError::HasActiveLoans);
    }

    // Safe to remove
    _members.erase(member_it);
    return {};
}

std::optional<Member> Library::find_member_by_id(uint32_t member_id) const {
    auto it = std::find_if(
        _members.begin(), _members.end(),
        [member_id](const Member& member) { return member.id == member_id; });

    if (it == _members.end()) {
        return std::nullopt;
    }

    return *it;
}

const std::vector<Member>& Library::list_all_members() const {
    return _members;
}

// Library - Loan Management
std::expected<Loan, BorrowError> Library::borrow_book(uint32_t member_id,
                                                      uint32_t book_id) {
    // Verify those member and book exist
    auto member_it = std::find_if(
        _members.begin(), _members.end(),
        [member_id](const Member& member) { return member.id == member_id; });

    if (member_it == _members.end()) {
        return std::unexpected(BorrowError::MemberNotFound);
    }

    auto book_it = std::find_if(
        _books.begin(), _books.end(),
        [book_id](const Book& book) { return book.id == book_id; });

    if (book_it == _books.end()) {
        return std::unexpected(BorrowError::BookNotFound);
    }

    // Now check if the book has already been loaned
    auto loan_it =
        std::find_if(_loans.begin(), _loans.end(), [book_id](const Loan& loan) {
            return loan.book_id == book_id && !loan.is_returned();
        });

    if (loan_it != _loans.end()) {
        return std::unexpected(BorrowError::AlreadyOnLoan);
    }

    // Safe to loan
    auto borrow_date = today();
    _loans.push_back(Loan{book_id, member_id, borrow_date,
                          add_days(borrow_date, _days_due), std::nullopt});

    return _loans.back();
}

std::expected<Loan, ReturnError> Library::return_book(uint32_t member_id,
                                                      uint32_t book_id) {
    // First check if the member and book exists
    auto member_it = std::find_if(
        _members.begin(), _members.end(),
        [member_id](const Member& member) { return member.id == member_id; });

    if (member_it == _members.end()) {
        return std::unexpected(ReturnError::MemberNotFound);
    }

    auto book_it = std::find_if(
        _books.begin(), _books.end(),
        [book_id](const Book& book) { return book.id == book_id; });

    if (book_it == _books.end()) {
        return std::unexpected(ReturnError::BookNotFound);
    }

    // Find the active loan for this book
    auto loan_it =
        std::find_if(_loans.begin(), _loans.end(), [book_id](const Loan& loan) {
            return loan.book_id == book_id && !loan.is_returned();
        });

    if (loan_it == _loans.end()) {
        return std::unexpected(ReturnError::NotOnLoan);
    }

    if (loan_it->member_id != member_id) {
        return std::unexpected(ReturnError::NotYourLoan);
    }

    // Since it's not yet returned, we return it
    loan_it->return_date = today();
    return *loan_it;
}

std::vector<Loan> Library::get_all_active_loans() const {
    std::vector<Loan> active_loans{};

    for (const Loan& loan : _loans) {
        if (!loan.is_returned()) {
            active_loans.push_back(loan);
        }
    }

    return active_loans;
}

std::vector<Loan> Library::get_all_overdue_loans() const {
    std::vector<Loan> overdue_loans{};

    Date now = today();
    for (const Loan& loan : _loans) {
        if (!loan.is_returned() && now > loan.due_date) {
            overdue_loans.push_back(loan);
        }
    }

    return overdue_loans;
}
