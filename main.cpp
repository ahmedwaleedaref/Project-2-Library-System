#include <stdio.h>
#include <iostream>
#include <climits>

using namespace std;

bool is_prefix(string const str, string const prefix)
{
    if (prefix.length() > str.length())
    {
        return false;
    }
    for (int i{0}; i < (int)prefix.length(); i++)
    {
        if (str[i] != prefix[i])
        {
            return false;
        }
    }
    return true;
}

struct Book
{
    int id;
    string name;
    int quantity;
    bool is_deleted;

    Book()
    {
        id = -1;
        name = "";
        quantity = 0;
        is_deleted = false;
    }

    Book(int _id, string _name, int _quantity)
    {
        id = _id;
        name = _name;
        quantity = _quantity;
        is_deleted = false;
    }

    void delete_book()
    {
        is_deleted = true;
    }
};

struct Id_Generator
{
    int max_id;
    Id_Generator()
    {
        max_id = 0;
    }

    int generate_id()
    {
        max_id++;
        return max_id - 1;
    }
};

struct SearchResult
{
    Book **Pointer_to_pointer_array;
    int count;
    int max_size;
    SearchResult()
    {
        count = 0;
        max_size = 1;
        Pointer_to_pointer_array = new Book *[max_size];
    }
    bool resize_to_double()
    {
        if (max_size == INT_MAX)
        {
            return false;
        }
        max_size *= 2;
        Book **new_Pointer_to_pointer_array = new Book *[max_size];
        for (int i = 0; i < max_size / 2; i++)
        {
            new_Pointer_to_pointer_array[i] = Pointer_to_pointer_array[i];
        }
        delete[] Pointer_to_pointer_array;
        Pointer_to_pointer_array = new_Pointer_to_pointer_array;
        return true;
    }
    bool add_pointer_to_book(Book *Book_ptr)
    {
        if (count == max_size)
        {
            bool is_every_thing_is_ok = resize_to_double();
            if (!is_every_thing_is_ok)
            {
                return false;
            }
        }
        Pointer_to_pointer_array[count] = Book_ptr;
        count++;
        return true;
    }
    SearchResult(SearchResult const &other)
    {
        this->max_size = other.max_size;
        this->count = other.count;
        this->Pointer_to_pointer_array = new Book *[this->max_size];
        for (int i{0}; i < other.count; i++)
        {
            this->Pointer_to_pointer_array[i] = other.Pointer_to_pointer_array[i];
        }
    }
    SearchResult &operator=(SearchResult const &other)
    {
        if (this == &other)
        {
            return *this;
        }
        delete[] this->Pointer_to_pointer_array;
        this->max_size = other.max_size;
        this->count = other.count;
        this->Pointer_to_pointer_array = new Book *[this->max_size];
        for (int i{0}; i < other.count; i++)
        {
            this->Pointer_to_pointer_array[i] = other.Pointer_to_pointer_array[i];
        }
        return *this;
    }
    ~SearchResult()
    {
        delete[] Pointer_to_pointer_array;
    }
};

struct Library_Books
{
    int MAX_Books_Number;
    int curr_book;
    int library_id;
    string library_name;
    Id_Generator id_generator;
    Book *book_array;

    Library_Books()
    {
        MAX_Books_Number = -1;
        curr_book = -1;
        library_id = -1;
        library_name = "";
        book_array = nullptr;
    }

    Library_Books(int _MAX_Books_Number, int _library_id, string _library_name)
    {
        MAX_Books_Number = _MAX_Books_Number;
        curr_book = 0;
        library_id = _library_id;
        library_name = _library_name;
        book_array = new Book[MAX_Books_Number]{};
    }

    ~Library_Books() { delete[] book_array; }

    Library_Books(const Library_Books &other)
    {
        MAX_Books_Number = other.MAX_Books_Number;
        curr_book = other.curr_book;
        library_id = other.library_id;
        library_name = other.library_name;
        id_generator = other.id_generator;
        if (other.book_array == nullptr)
        {
            book_array = nullptr;
        }
        else
        {
            book_array = new Book[MAX_Books_Number];
            for (int i{0}; i < curr_book; i++)
            {
                book_array[i] = other.book_array[i];
            }
        }
    }

    Library_Books &operator=(const Library_Books &other)
    {
        if (this == &other)
        {
            return *this;
        }
        this->MAX_Books_Number = other.MAX_Books_Number;
        this->curr_book = other.curr_book;
        this->library_id = other.library_id;
        this->library_name = other.library_name;
        this->id_generator = other.id_generator;
        delete[] this->book_array;
        this->book_array = new Book[MAX_Books_Number];
        for (int i{0}; i < curr_book; i++)
        {
            this->book_array[i] = other.book_array[i];
        }
        return *this;
    }

    bool add_book(string name, int quantity)
    {
        // basic checks over input
        if (name.size() < 3 || name.size() > 70 || quantity <= 0)
        {
            return false;
        }
        if (curr_book >= MAX_Books_Number)
        {
            return false;
        }
        book_array[curr_book] = Book(id_generator.generate_id(), name, quantity);
        curr_book++;
        return true;
    }

    int get_book_index_by_id(int Book_id)
    {
        if (curr_book <= 0)
        {
            return -1;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (book_array[i].id == Book_id)
            {
                return i;
            }
        }
        return -1;
    }

    bool delete_book(int Book_id)
    {
        int idx{get_book_index_by_id(Book_id)};
        if (idx == -1 || (book_array[idx].is_deleted == true))
        {
            return false;
        }
        book_array[idx].is_deleted = true;
        return true;
    }
    void get_books_by_prefix_console(string Book_prefix)
    {
        if (curr_book <= 0)
        {
            cout << "No Books exist in this library\n";
            return;
        }
        bool is_there_is_a_match{false};
        for (int i{0}; i < curr_book; i++)
        {
            if (is_prefix(book_array[i].name, Book_prefix) && (book_array[i].is_deleted == false))
            {
                cout << "Book name : " << book_array[i].name << " Book id : " << book_array[i].id << " \n";
                is_there_is_a_match = true;
            }
        }
        if (!is_there_is_a_match)
        {
            cout << "No matches exist for this prefix \n";
        }
    }

    void get_all_books_in_library_console()
    {
        if (curr_book <= 0)
        {
            cout << "No Books exist in this library\n";
            return;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (book_array[i].is_deleted == false)
            {
                cout << "Book id : " << book_array[i].id << " Book name : " << book_array[i].name << " Book Quantity : " << book_array[i].quantity << " \n";
            }
        }
    }

    SearchResult get_books_by_prefix(string Book_prefix)
    {
        SearchResult query_res = SearchResult();
        if (curr_book <= 0)
        {
            return query_res;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (is_prefix(book_array[i].name, Book_prefix) && (book_array[i].is_deleted == false))
            {
                query_res.add_pointer_to_book(&book_array[i]);
            }
        }
        return query_res;
    }

    SearchResult get_books_by_prefix(string Book_prefix)
    {
        SearchResult query_res = SearchResult();
        if (curr_book <= 0)
        {
            return query_res;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (is_prefix(book_array[i].name, Book_prefix) && (book_array[i].is_deleted == false))
            {
                query_res.add_pointer_to_book(&book_array[i]);
            }
        }
        return query_res;
    }
    
    SearchResult get_all_books()
    {
        SearchResult query_res = SearchResult();
        if (curr_book <= 0)
        {
            return query_res;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (book_array[i].is_deleted == false)
            {
                query_res.add_pointer_to_book(&book_array[i]);
            }
        }
        return query_res;
    }
};

int main()
{
    return 0;
}