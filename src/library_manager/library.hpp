#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include "book.hpp"
#include "date.hpp"
#include "loan.hpp"
#include "member.hpp"

enum class RemoveBookError { NotFound, CurrentlyOnLoan };
enum class AddMemberError { NameAlreadyExists };
enum class RemoveMemberError { NotFound, HasActiveLoans };
enum class BorrowError { BookNotFound, MemberNotFound, AlreadyOnLoan };
enum class ReturnError { BookNotFound, MemberNotFound, NotOnLoan, NotYourLoan };

class Library {
   private:
    std::vector<Book> _books{};
    std::vector<Member> _members{};
    std::vector<Loan> _loans{};

    uint32_t _next_member_id = 1;
    uint32_t _next_book_id = 1;

    static constexpr int DEFAULT_DAYS_DUE = 14;
    int _days_due{DEFAULT_DAYS_DUE};

   public:
    // Constructor
    explicit Library(int days_due = DEFAULT_DAYS_DUE) : _days_due{days_due} {}

    // Book Management
    void add_book(std::string title, std::string author, Date date_published);
    [[nodiscard]] std::expected<void, RemoveBookError> remove_book(
        uint32_t book_id);
    [[nodiscard]] std::optional<Book> find_book_by_id(uint32_t book_id) const;
    const std::vector<Book>& list_all_books() const;

    // Member Management
    [[nodiscard]] std::expected<void, AddMemberError> add_member(
        std::string name);
    [[nodiscard]] std::expected<void, RemoveMemberError> remove_member(
        uint32_t member_id);
    [[nodiscard]] std::optional<Member> find_member_by_id(
        uint32_t member_id) const;
    const std::vector<Member>& list_all_members() const;

    // Loan Management
    [[nodiscard]] std::expected<Loan, BorrowError> borrow_book(
        uint32_t member_id, uint32_t book_id);
    [[nodiscard]] std::expected<Loan, ReturnError> return_book(
        uint32_t member_id, uint32_t book_id);
    std::vector<Loan> get_all_active_loans() const;
    std::vector<Loan> get_all_overdue_loans() const;
};
