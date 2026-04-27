#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "book.hpp"
#include "input.hpp"
#include "library.hpp"
#include "loan.hpp"
#include "member.hpp"

constexpr static int TITLE_PADDING = 40;

constexpr std::string_view TREE_BRANCH = "\u251C\u2500";
constexpr std::string_view TREE_CORNER = "\u2514\u2500";
constexpr std::string_view EM_DASH = "\u2014";
constexpr std::string_view FROWN = "\u2639";

struct MenuItem {
    std::string label;
    std::function<void()> action;
};

enum class AppState { MainMenu, BookMenu, MemberMenu, LoanMenu, Exit };

void menu_builder(std::string_view title, const std::vector<MenuItem>& menu,
                  AppState& current_state) {
    std::println("\n{:=^{}}", std::format(" {} ", title), TITLE_PADDING);

    for (size_t i = 0; i < menu.size(); ++i) {
        std::println("{}. {}", i + 1, menu[i].label);
    }
    std::println("{:=<{}}", "", TITLE_PADDING);

    auto choice = read_unsigned("Selection: ");

    if (!choice.has_value()) {
        if (choice.error() == UnsignedError::EndOfFile ||
            choice.error() == UnsignedError::StreamFailure) {
            current_state = AppState::Exit;
        } else {
            std::println(std::cerr, "Invalid input. Please try again.");
            wait_for_enter();
        }
        return;
    }

    if (choice.value() == 0) {
        std::println(std::cerr, "Invalid choice. Out of range.");
        wait_for_enter();
        return;
    }

    uint32_t index = choice.value() - 1;
    if (index < menu.size()) {
        menu[index].action();
    } else {
        std::println(std::cerr, "Invalid choice. Out of range.");
        wait_for_enter();
    }
}

void main_menu(AppState& current_state) {
    std::vector<MenuItem> menu{
        {"Manage Books",
         [&current_state]() { current_state = AppState::BookMenu; }},
        {"Manage Members",
         [&current_state]() { current_state = AppState::MemberMenu; }},
        {"Manage Loans",
         [&current_state]() { current_state = AppState::LoanMenu; }},
        {"Exit", [&current_state]() { current_state = AppState::Exit; }},
    };

    menu_builder("Main Menu", menu, current_state);
}

void book_menu(Library& library, AppState& current_state) {
    std::vector<MenuItem> menu{
        {"Add a Book",
         [&library]() {
             std::println("Please enter the following informations.");
             auto title = read_string("Book Title: ");
             if (!title) return;

             auto author = read_string("Book Author: ");
             if (!author) return;

             auto date_published = read_date("Date Published (MM-DD-YYYY): ");
             if (!date_published) return;

             library.add_book(std::move(title.value()),
                              std::move(author.value()),
                              date_published.value());

             std::println("Successfully added the book.");
             wait_for_enter();
         }},
        {"Remove a Book",
         [&library]() {
             auto book_id = read_unsigned("Enter Book ID: ");
             if (!book_id) return;

             auto result = library.remove_book(book_id.value());

             if (!result) {
                 switch (result.error()) {
                     case RemoveBookError::NotFound:
                         std::println(std::cerr,
                                      "Error: No book found with ID #{}.",
                                      book_id.value());
                         break;
                     case RemoveBookError::CurrentlyOnLoan:
                         std::println(std::cerr,
                                      "Error: Cannot remove. Book is currently "
                                      "borrowed.");
                         break;
                 }
             } else {
                 std::println("Success: Book with ID #{} has been removed.",
                              book_id.value());
             }

             wait_for_enter();
         }},

        {"Find a Book by ID",
         [&library]() {
             auto book_id = read_unsigned("Enter Book ID: ");
             if (!book_id) return;

             auto result = library.find_book_by_id(book_id.value());
             if (result) {
                 const Book& book = result.value();
                 std::println("  Book #{}", book.id);
                 std::println("   {}Title: {}", TREE_BRANCH, book.title);
                 std::println("   {}Author: {}", TREE_BRANCH, book.author);
                 std::println("   {}Date Published: {}", TREE_CORNER,
                              book.date_published);
                 std::println();
             } else {
                 std::println("Did not find a book with the ID #{}",
                              book_id.value());
             }
             wait_for_enter();
         }},

        {"List all Books",
         [&library]() {
             const auto& books = library.list_all_books();

             if (books.empty()) {
                 std::println("There are no books in the library. {}", FROWN);
             } else {
                 for (const Book& book : books) {
                     std::println("  Book #{}", book.id);
                     std::println("   {}Title: {}", TREE_BRANCH, book.title);
                     std::println("   {}Author: {}", TREE_BRANCH, book.author);
                     std::println("   {}Date Published: {}", TREE_CORNER,
                                  book.date_published);
                     std::println();
                 }
             }
             wait_for_enter();
         }},

        {"Back to Main Menu",
         [&current_state]() { current_state = AppState::MainMenu; }},
    };

    menu_builder("Book Menu", menu, current_state);
}

void member_menu(Library& library, AppState& current_state) {
    std::vector<MenuItem> menu{
        {"Add a Member",
         [&library]() {
             auto name = read_string("Enter New Member Name: ");
             if (!name) return;

             auto result = library.add_member(std::move(name.value()));
             if (!result.has_value() &&
                 result.error() == AddMemberError::NameAlreadyExists) {
                 std::println(
                     "Error: A member with the same name already exists.");
             } else {
                 std::println("Successfully added a new member.");
             }

             wait_for_enter();
         }},
        {"Remove a Member",
         [&library]() {
             auto member_id = read_unsigned("Enter Member ID: ");
             if (!member_id) return;

             auto result = library.remove_member(member_id.value());

             if (!result) {
                 switch (result.error()) {
                     case RemoveMemberError::NotFound:
                         std::println(std::cerr,
                                      "Error: No member found with ID #{}.",
                                      member_id.value());
                         break;
                     case RemoveMemberError::HasActiveLoans:
                         std::println(std::cerr,
                                      "Error: Cannot remove. Member has active "
                                      "loan(s).");
                         break;
                 }
             } else {
                 std::println("Success: Member with ID #{} has been removed.",
                              member_id.value());
             }

             wait_for_enter();
         }},

        {"Find a Member by ID",
         [&library]() {
             auto member_id = read_unsigned("Enter Member ID: ");
             if (!member_id) return;

             auto result = library.find_member_by_id(member_id.value());
             if (result) {
                 const Member& member = result.value();
                 std::println("  Member #{}", member.id);
                 std::println("   {}Name: {}", TREE_CORNER, member.name);
                 std::println();
             } else {
                 std::println("Did not find a member with the ID #{}",
                              member_id.value());
             }
             wait_for_enter();
         }},

        {"List all Members",
         [&library]() {
             const auto& members = library.list_all_members();

             if (members.empty()) {
                 std::println("There are no members of the library. {}", FROWN);
             } else {
                 for (const Member& member : members) {
                     std::println("  Member #{}", member.id);
                     std::println("   {}Name: {}", TREE_CORNER, member.name);
                     std::println();
                 }
             }
             wait_for_enter();
         }},

        {"Back to Main Menu",
         [&current_state]() { current_state = AppState::MainMenu; }},
    };

    menu_builder("Member Menu", menu, current_state);
}

void loan_menu(Library& library, AppState& current_state) {
    std::vector<MenuItem> menu{
        {"Borrow a Book",
         [&library]() {
             std::println("Please enter the following informations.");
             auto member_id = read_unsigned("Borrower's Member ID: ");
             if (!member_id) return;

             std::vector<uint32_t> borrowed_books{};
             int current_total = 0;
             const int MAX_BOOKS = 5;

             while (current_total < MAX_BOOKS) {
                 auto book_id = read_unsigned("Book ID (0 to finish): ");
                 if (!book_id || book_id == 0) break;

                 borrowed_books.push_back(book_id.value());
                 ++current_total;
             }

             if (borrowed_books.size() == MAX_BOOKS) {
                 std::println("Maximum limit of {} books reached.", MAX_BOOKS);
             }

             std::println("Processing loan for {} book(s)...",
                          borrowed_books.size());

             std::vector<Loan> successful_loans{};

             for (auto id : borrowed_books) {
                 auto loan = library.borrow_book(member_id.value(), id);
                 if (!loan.has_value()) {
                     switch (loan.error()) {
                         case BorrowError::MemberNotFound:
                             std::println(
                                 std::cerr,
                                 "Error: Did not find a member with ID #{}.",
                                 member_id.value());
                             wait_for_enter();
                             return;
                         case BorrowError::BookNotFound:
                             std::println(
                                 std::cerr,
                                 "Error: Did not find a book with ID #{}.", id);
                             break;
                         case BorrowError::AlreadyOnLoan:
                             std::println(
                                 std::cerr,
                                 "Book #{} is already on loan. Skipping.", id);
                             break;
                     }
                 } else {
                     successful_loans.push_back(loan.value());
                 }
             }

             for (const auto& loan : successful_loans) {
                 std::println("  Loan: Member #{} {} Book #{}: success.",
                              loan.member_id, EM_DASH, loan.book_id);
                 std::println("  {}Borrow Date: {}", TREE_BRANCH,
                              loan.borrow_date);
                 std::println("  {}Due Date:    {}", TREE_CORNER,
                              loan.due_date);
             }

             std::println(
                 "\nNotice: Failure to return the book on time will incur "
                 "a penalty.");

             wait_for_enter();
         }},

        {"Return a Book",
         [&library]() {
             auto member_id = read_unsigned("Member ID: ");
             if (!member_id) return;

             auto book_id = read_unsigned("Book ID to return: ");
             if (!book_id) return;

             auto result =
                 library.return_book(member_id.value(), book_id.value());

             if (!result) {
                 switch (result.error()) {
                     case ReturnError::MemberNotFound:
                         std::println(std::cerr,
                                      "Error: No member found with ID #{}.",
                                      member_id.value());
                         break;
                     case ReturnError::BookNotFound:
                         std::println(std::cerr,
                                      "Error: No book found with ID #{}.",
                                      book_id.value());
                         break;
                     case ReturnError::NotOnLoan:
                         std::println(
                             std::cerr,
                             "Error: Book #{} is not currently on loan.",
                             book_id.value());
                         break;
                     case ReturnError::NotYourLoan:
                         std::println(std::cerr,
                                      "Error: Book #{} was not borrowed by "
                                      "Member #{}.",
                                      book_id.value(), member_id.value());
                         break;
                 }
             } else {
                 const Loan& loan = result.value();
                 std::println("Success: Book #{} has been returned.",
                              book_id.value());
                 std::println("  {}Borrowed on: {}", TREE_BRANCH,
                              loan.borrow_date);
                 std::println("  {}Returned on: {}", TREE_CORNER,
                              loan.return_date.value());
             }

             wait_for_enter();
         }},

        {"View Active Loans",
         [&library]() {
             auto loans = library.get_all_active_loans();

             if (loans.empty()) {
                 std::println("There are no active loans.");
             } else {
                 std::println("Active loans ({}):", loans.size());
                 for (const Loan& loan : loans) {
                     std::println("  Member #{} {} Book #{}", loan.member_id,
                                  EM_DASH, loan.book_id);
                     std::println("  {}Borrow Date: {}", TREE_BRANCH,
                                  loan.borrow_date);
                     std::println("  {}Due Date:    {}", TREE_CORNER,
                                  loan.due_date);
                     std::println();
                 }
             }

             wait_for_enter();
         }},

        {"View Overdue Loans",
         [&library]() {
             auto loans = library.get_all_overdue_loans();

             if (loans.empty()) {
                 std::println("There are no overdue loans.");
             } else {
                 std::println("Overdue loans ({}):", loans.size());
                 for (const Loan& loan : loans) {
                     std::println("  Member #{} {} Book #{}", loan.member_id,
                                  EM_DASH, loan.book_id);
                     std::println("  {}Borrow Date: {}", TREE_BRANCH,
                                  loan.borrow_date);
                     std::println("  {}Due Date:    {}", TREE_CORNER,
                                  loan.due_date);
                     std::println();
                 }
             }

             wait_for_enter();
         }},

        {"Back to Main Menu",
         [&current_state]() { current_state = AppState::MainMenu; }},
    };

    menu_builder("Loan Menu", menu, current_state);
}

int main() {
    Library library{};
    AppState state = AppState::MainMenu;

    std::println("{:=<{}}", "", TITLE_PADDING);
    std::println("{:^{}}", "The Library", TITLE_PADDING);
    std::println("{:=<{}}", "", TITLE_PADDING);

    while (state != AppState::Exit) {
        switch (state) {
            case AppState::MainMenu:
                main_menu(state);
                break;
            case AppState::BookMenu:
                book_menu(library, state);
                break;
            case AppState::MemberMenu:
                member_menu(library, state);
                break;
            case AppState::LoanMenu:
                loan_menu(library, state);
                break;
            case AppState::Exit:
                break;
        }
    }

    std::println("Thank you for using the library!");
    std::println("Exiting.");

    return 0;
}
