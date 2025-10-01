// library_system.cpp
// Simple Online Library Management System (C++17)
// Single-file example with classes: Book, User, Library
// Includes a small test-suite in main() demonstrating positive & negative cases.

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>
#include <cassert>

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::endl;

/* ---------------------------
   Book class
   --------------------------- */
class Book {
private:
    string isbn;
    string title;
    string author;
    bool available;

public:
    Book() = default;
    Book(string isbn_, string title_, string author_)
        : isbn(std::move(isbn_)), title(std::move(title_)), author(std::move(author_)), available(true) {}

    // getters
    string getISBN() const { return isbn; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    bool isAvailable() const { return available; }

    // state modifiers
    void setAvailable(bool v) { available = v; }

    // display
    void display() const {
        cout << "ISBN: " << isbn << ", Title: " << title << ", Author: " << author
             << ", Available: " << (available ? "Yes" : "No") << endl;
    }
};

/* ---------------------------
   User class
   --------------------------- */
class User {
private:
    string userId;
    string name;
    // store borrowed ISBNs
    unordered_set<string> borrowedBooks;

public:
    User() = default;
    User(string id_, string name_) : userId(std::move(id_)), name(std::move(name_)) {}

    string getId() const { return userId; }
    string getName() const { return name; }

    bool hasBorrowed(const string &isbn) const {
        return borrowedBooks.find(isbn) != borrowedBooks.end();
    }

    void borrowBook(const string &isbn) {
        borrowedBooks.insert(isbn);
    }

    void returnBook(const string &isbn) {
        borrowedBooks.erase(isbn);
    }

    vector<string> listBorrowed() const {
        vector<string> out;
        out.reserve(borrowedBooks.size());
        for (auto &s : borrowedBooks) out.push_back(s);
        return out;
    }

    void display() const {
        cout << "User ID: " << userId << ", Name: " << name << ", Borrowed count: " << borrowedBooks.size() << endl;
    }
};

/* ---------------------------
   Library class
   --------------------------- */
class Library {
private:
    // Map ISBN -> Book
    unordered_map<string, Book> books;
    // Map userId -> User
    unordered_map<string, User> users;

public:
    Library() = default;

    // --- Book management ---
    void addBook(const Book &b) {
        const string &isbn = b.getISBN();
        if (isbn.empty()) throw std::invalid_argument("ISBN cannot be empty");
        if (books.find(isbn) != books.end()) throw std::runtime_error("Book with this ISBN already exists");
        books.emplace(isbn, b);
    }

    void removeBook(const string &isbn) {
        auto it = books.find(isbn);
        if (it == books.end()) throw std::runtime_error("Book not found");
        if (!it->second.isAvailable()) throw std::runtime_error("Cannot remove a book that is currently borrowed");
        books.erase(it);
    }

    // search functions
    vector<Book> searchByTitle(const string &partial) const {
        vector<Book> res;
        string low = partial;
        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
        for (const auto &p : books) {
            string t = p.second.getTitle();
            string tl = t; std::transform(tl.begin(), tl.end(), tl.begin(), ::tolower);
            if (tl.find(low) != string::npos) res.push_back(p.second);
        }
        return res;
    }

    vector<Book> searchByAuthor(const string &partial) const {
        vector<Book> res;
        string low = partial;
        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
        for (const auto &p : books) {
            string a = p.second.getAuthor();
            string al = a; std::transform(al.begin(), al.end(), al.begin(), ::tolower);
            if (al.find(low) != string::npos) res.push_back(p.second);
        }
        return res;
    }

    Book getBook(const string &isbn) const {
        auto it = books.find(isbn);
        if (it == books.end()) throw std::runtime_error("Book not found");
        return it->second;
    }

    // --- User management ---
    void addUser(const User &u) {
        const string &id = u.getId();
        if (id.empty()) throw std::invalid_argument("User ID cannot be empty");
        if (users.find(id) != users.end()) throw std::runtime_error("User already exists");
        users.emplace(id, u);
    }

    void removeUser(const string &id) {
        auto it = users.find(id);
        if (it == users.end()) throw std::runtime_error("User not found");
        if (!it->second.listBorrowed().empty()) throw std::runtime_error("User still has borrowed books");
        users.erase(it);
    }

    User getUser(const string &id) const {
        auto it = users.find(id);
        if (it == users.end()) throw std::runtime_error("User not found");
        return it->second;
    }

    // --- Borrowing / returning ---
    void borrowBook(const string &userId, const string &isbn) {
        auto uit = users.find(userId);
        if (uit == users.end()) throw std::runtime_error("User not found");
        auto bit = books.find(isbn);
        if (bit == books.end()) throw std::runtime_error("Book not found");
        if (!bit->second.isAvailable()) throw std::runtime_error("Book not available");

        // mark book unavailable and add to user's borrowed list
        bit->second.setAvailable(false);
        // We need to update the user stored in the map (not a copy)
        users.at(userId).borrowBook(isbn);
    }

    void returnBook(const string &userId, const string &isbn) {
        auto uit = users.find(userId);
        if (uit == users.end()) throw std::runtime_error("User not found");
        auto bit = books.find(isbn);
        if (bit == books.end()) throw std::runtime_error("Book not found");
        if (!users.at(userId).hasBorrowed(isbn)) throw std::runtime_error("This user did not borrow this book");

        users.at(userId).returnBook(isbn);
        books.at(isbn).setAvailable(true);
    }

    // display helpers
    void displayBooks() const {
        cout << "Library Books (" << books.size() << "):" << endl;
        for (const auto &p : books) {
            p.second.display();
        }
    }

    void displayUsers() const {
        cout << "Users (" << users.size() << "):" << endl;
        for (const auto &p : users) {
            p.second.display();
        }
    }
};

/* ---------------------------
   Small test-suite
   --------------------------- */
void runTests() {
    cout << "Running tests..." << endl;
    Library lib;

    // Create sample books & users
    Book b1("ISBN-001", "Introduction to C++", "Bjarne Stroustrup");
    Book b2("ISBN-002", "Programming Principles", "Jane Doe");
    Book b3("ISBN-003", "Algorithms in Depth", "Robert Sedgewick");

    lib.addBook(b1);
    lib.addBook(b2);
    lib.addBook(b3);

    User u1("U001", "Alice");
    User u2("U002", "Bob");

    lib.addUser(u1);
    lib.addUser(u2);

    // Positive case: search
    auto res = lib.searchByTitle("c++");
    assert(res.size() == 1);
    assert(res[0].getISBN() == "ISBN-001");

    // Positive case: borrow book
    lib.borrowBook("U001", "ISBN-001");
    // now book should be unavailable
    Book afterBorrow = lib.getBook("ISBN-001");
    assert(!afterBorrow.isAvailable());

    // Negative case: borrow already borrowed
    bool threw = false;
    try {
        lib.borrowBook("U002", "ISBN-001"); // should throw
    } catch (...) {
        threw = true;
    }
    assert(threw);

    // Positive case: return book
    lib.returnBook("U001", "ISBN-001");
    assert(lib.getBook("ISBN-001").isAvailable());

    // Negative case: returning a book not borrowed by the user
    threw = false;
    try {
        lib.returnBook("U002", "ISBN-002"); // Bob did not borrow ISBN-002
    } catch (...) {
        threw = true;
    }
    assert(threw);

    // Negative case: remove book that doesn't exist
    threw = false;
    try {
        lib.removeBook("ISBN-999");
    } catch (...) {
        threw = true;
    }
    assert(threw);

    // Can't remove a borrowed book
    lib.borrowBook("U002", "ISBN-002");
    threw = false;
    try {
        lib.removeBook("ISBN-002");
    } catch (...) {
        threw = true;
    }
    assert(threw);
    // cleanup
    lib.returnBook("U002", "ISBN-002");
    lib.removeBook("ISBN-002");
    cout << "All tests passed." << endl;
}

/* ---------------------------
   Minimal interactive demo (optional)
   --------------------------- */
void demoInteractive() {
    Library lib;
    // pre-load a few items
    lib.addBook(Book("ISBN-A", "Learn C++", "Author A"));
    lib.addBook(Book("ISBN-B", "Data Structures", "Author B"));
    lib.addBook(Book("ISBN-C", "Databases", "Author C"));
    lib.addUser(User("U100", "Charlie"));

    cout << "\n=== Simple interactive demo ===\n";
    lib.displayBooks();
    lib.displayUsers();

    cout << "\nCharlie (U100) borrows ISBN-A...\n";
    lib.borrowBook("U100", "ISBN-A");
    lib.displayBooks();

    cout << "\nCharlie returns ISBN-A...\n";
    lib.returnBook("U100", "ISBN-A");
    lib.displayBooks();

    cout << "\nSearch for 'Data':\n";
    auto res = lib.searchByTitle("Data");
    for (auto &b : res) b.display();
}

/* ---------------------------
   main
   --------------------------- */
int main() {
    try {
        runTests();
        demoInteractive();
    } catch (const std::exception &ex) {
        cout << "Exception: " << ex.what() << endl;
        return 1;
    } catch (...) {
        cout << "Unknown exception occurred." << endl;
        return 1;
    }
    return 0;
}
