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
    auto book_it = std::find_if(
        _books.begin(), _books.end(),
        [book_id](const Book& book) { return book.id == book_id; });

    if (book_it == _books.end()) {
        return std::unexpected(RemoveBookError::NotFound);
    }

    auto loan_it = std::find_if(
        _loans.begin(), _loans.end(),
        [book_id](const Loan& loan) { return loan.book_id == book_id; });

    if (loan_it != _loans.end()) {
        return std::unexpected(RemoveBookError::CurrentlyOnLoan);
    }

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
    auto it = std::find_if(
        _members.begin(), _members.end(),
        [&name](const Member& member) { return member.name == name; });

    if (it != _members.end()) {
        return std::unexpected(AddMemberError::NameAlreadyExists);
    }

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
        _loans.begin(), _loans.end(),
        [member_id](const Loan& loan) { return loan.member_id == member_id; });

    if (loan_it != _loans.end()) {
        return std::unexpected(RemoveMemberError::HasActiveLoans);
    }

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

    auto loan_it = std::find_if(
        _loans.begin(), _loans.end(),
        [book_id](const Loan& loan) { return loan.book_id == book_id; });

    if (loan_it != _loans.end()) {
        return std::unexpected(BorrowError::AlreadyOnLoan);
    }

    auto borrow_date = today();
    _loans.push_back(Loan{book_id, member_id, borrow_date,
                          add_days(borrow_date, _days_due), std::nullopt});

    return _loans.back();
}

std::expected<Loan, ReturnError> Library::return_book(uint32_t member_id,
                                                      uint32_t book_id) {
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

    auto loan_it = std::find_if(
        _loans.begin(), _loans.end(),
        [book_id](const Loan& loan) { return loan.book_id == book_id; });

    if (loan_it == _loans.end()) {
        return std::unexpected(ReturnError::NotOnLoan);
    }

    if (loan_it->member_id != member_id) {
        return std::unexpected(ReturnError::NotYourLoan);
    }

    loan_it->return_date = today();
    Loan returned = *loan_it;
    _loans.erase(loan_it);
    return returned;
}

std::vector<Loan> Library::get_all_active_loans() const { return _loans; }

std::vector<Loan> Library::get_all_overdue_loans() const {
    std::vector<Loan> overdue{};

    Date now = today();
    for (const Loan& loan : _loans) {
        if (now > loan.due_date) {
            overdue.push_back(loan);
        }
    }

    return overdue;
}
