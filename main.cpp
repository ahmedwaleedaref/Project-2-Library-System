#include <iostream>
#include <climits>
#include <ctime>
#include <cmath>
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

struct user
{
    int id;
    string name;
    string national_id;
    string email;
    string address;
    string phone_number;
    bool is_deleted;
    bool is_band_from_borrowing;
    user()
    {
        id = -1;
        name = "";
        national_id = "";
        email = "";
        address = "";
        phone_number = "";
        is_deleted = false;
        is_band_from_borrowing = false;
    }
    user(int _id, string _name, string _national_id, string _email, string _address, string _phone_number)
    {
        id = _id;
        name = _name;
        national_id = _national_id;
        email = _email;
        address = _address;
        phone_number = _phone_number;
        is_deleted = false;
        is_band_from_borrowing = false;
    }
    void delete_user()
    {
        is_deleted = true;
    }
    void ban_user()
    {
        is_band_from_borrowing = true;
    }
    void unban_user()
    {
        is_band_from_borrowing = false;
    }
};
struct Borrowed_Book
{
    int user_id;
    int book_id;
    time_t borrow_date;
    time_t supposed_return_date;
    time_t real_return_data;
    float fees;
    float what_is_paid;
    bool is_fully_paid;
    bool is_finished;
    Borrowed_Book()
    {
        user_id = -1;
        book_id = -1;
        borrow_date = 0;
        supposed_return_date = 0;
        real_return_data = 0;
        fees = -1;
        what_is_paid = -1;
        is_fully_paid = false;
        is_finished = false;
    }
    Borrowed_Book(int _user_id, int _book_id, int Year, int month, int month_day, float _fees, bool _is_fully_paid, float _what_is_paid = -1)
    {
        user_id = _user_id;
        book_id = _book_id;
        // now lets handle time stuff carfully borrow_data should be current data that admin is constructing this object
        borrow_date = time(NULL); // number of second from 1900 ever thing in the library time ctime calclute time from (jan , 1 , 1900) this is not humen readable
        struct tm datetime{};
        datetime.tm_year = Year - 1900; // Number of years since 1900
        datetime.tm_mon = month - 1;    // Number of months since January so it is zero index
        datetime.tm_mday = month_day;   // not zero indexed from
        datetime.tm_hour = 23;          // make this always 11:59 pm
        datetime.tm_min = 59;
        datetime.tm_sec = 0;
        datetime.tm_isdst = -1;
        supposed_return_date = mktime(&datetime);
        real_return_data = 0;
        fees = _fees;
        is_fully_paid = _is_fully_paid;
        if (is_fully_paid == false)
        {

            what_is_paid = _what_is_paid;
        }
        else
        {
            what_is_paid = _fees;
        }
        is_finished = false;
    }
    bool get_is_fully_paid()
    {
        return is_fully_paid;
    }
    float check_remaing_that_user_need_to_pay()
    {
        if (is_fully_paid)
        {
            return 0;
        }
        return fees - what_is_paid;
    }
    bool increment_what_is_paid(float money)
    {
        if (is_fully_paid || money < 0 || (what_is_paid + money) > fees)
        {
            return false;
        }
        what_is_paid += money;
        if (fabs(fees - what_is_paid) < 0.001)
        {
            is_fully_paid = true;
        }
        return true;
    }

    bool pay_full_fees()
    {
        if (!is_fully_paid)
        {
            is_fully_paid = true;
            what_is_paid = fees;
            return true;
        }
        return false;
    }
    bool user_return_book()
    {
        if (!is_fully_paid)
        {
            return false;
        }
        real_return_data = time(NULL);
        is_finished = true;
        return true;
    }
};

/*
the huge copy past we did lead to us discover and understnad templates deeply i did some search about this and thier concept instead of you doing the copy paste by hand for a class
that will have exactly same ligoc (syntax) except the data type we could make class type agnostic aka every type used by this class will be called T you want for example a SearchResult with
type user you send type as parameter T=user and compiler will do the job of replacing each T with user make the compiler at compile time do the task of copy past .
-now i have deeper understanding about sysntax vector<int> , queue<double> in general .
*/
template <class T>
struct Dynamic_arr
{
    T *data_pointer;
    int count;
    int max_size;
    Dynamic_arr()
    {
        count = 0;
        max_size = 1;
        data_pointer = new T[max_size];
    }
    bool resize_to_double()
    {
        if (max_size == INT_MAX)
        {
            return false;
        }
        max_size *= 2;
        T *new_data_pointer = new T[max_size];
        for (int i = 0; i < count; i++)
        {
            new_data_pointer[i] = data_pointer[i];
        }
        delete[] data_pointer;
        data_pointer = new_data_pointer;
        return true;
    }
    bool add_element(T element)
    {
        if (count == max_size)
        {
            bool is_every_thing_is_ok = resize_to_double();
            if (!is_every_thing_is_ok)
            {
                return false;
            }
        }
        data_pointer[count] = element;
        count++;
        return true;
    }
    Dynamic_arr(Dynamic_arr const &other)
    {
        this->max_size = other.max_size;
        this->count = other.count;
        this->data_pointer = new T[this->max_size];
        for (int i{0}; i < other.count; i++)
        {
            this->data_pointer[i] = other.data_pointer[i];
        }
    }
    Dynamic_arr &operator=(Dynamic_arr const &other)
    {
        if (this == &other)
        {
            return *this;
        }
        delete[] this->data_pointer;
        this->max_size = other.max_size;
        this->count = other.count;
        this->data_pointer = new T[this->max_size];
        for (int i{0}; i < other.count; i++)
        {
            this->data_pointer[i] = other.data_pointer[i];
        }
        return *this;
    }
    ~Dynamic_arr()
    {
        delete[] data_pointer;
    }
};
/*
i will add is finished and real_return_data it make more sense this bcs now this dto can deal with display only unfinshed book borrows for the user and display all book borrwed be user
*/
struct DTO_User_Borrowed_Book
{
    string Book_Name;
    string User_name;
    time_t borrow_date;
    time_t supposed_return_date;
    time_t real_return_date;
    bool is_late;
    float fees;
    float what_is_paid;
    float what_is_remaning_to_pay;
    bool is_fully_paid;
    bool is_finished;

    DTO_User_Borrowed_Book()
    {
        Book_Name = "";
        User_name = "";
        borrow_date = 0;
        supposed_return_date = 0;
        real_return_date = 0;
        is_late = false;
        fees = -1;
        what_is_paid = -1;
        what_is_remaning_to_pay = 0;
        is_fully_paid = false;
        is_finished = false;
    }
    DTO_User_Borrowed_Book(string _Book_Name, string _User_name, time_t _borrow_date, time_t _supposed_return_date, time_t _real_return_date, float _fees, float _what_is_paid, bool _is_fully_paid, bool _is_finished)
    {
        Book_Name = _Book_Name;
        User_name = _User_name;
        borrow_date = _borrow_date;
        supposed_return_date = _supposed_return_date;
        real_return_date = _real_return_date;
        fees = _fees;
        what_is_paid = _what_is_paid;
        is_fully_paid = _is_fully_paid;
        what_is_remaning_to_pay = calc_remaning_to_pay();
        is_finished = _is_finished;
        is_late = ini_is_late();
    }
    bool ini_is_late()
    {
        time_t curr_time{time(NULL)};
        // this is allowed ?
        if (curr_time > supposed_return_date && is_finished == false)
        {
            return true;
        }
        return false;
    }
    float calc_remaning_to_pay()
    {
        if (!is_fully_paid)
        {
            return fees - what_is_paid;
        }
        return 0;
    }
};

struct Library_Books
{
    int MAX_Books_Number;
    int MAX_Users_Number;
    int MAX_Borrowed_Books_Number;
    int curr_book;
    int curr_user;
    int Number_of_borrowed_transaction;
    int library_id;
    string library_name;
    Id_Generator book_id_generator;
    Id_Generator user_id_generator;
    Book *book_array;
    user *library_users_array;
    Borrowed_Book *library_borrowed_books;

    Library_Books()
    {
        MAX_Books_Number = -1;
        MAX_Users_Number = -1;
        MAX_Borrowed_Books_Number = -1;
        curr_book = -1;
        curr_user = -1;
        Number_of_borrowed_transaction = -1;
        library_id = -1;
        library_name = "";
        book_array = nullptr;
        library_users_array = nullptr;
        library_borrowed_books = nullptr;
    }

    Library_Books(int _MAX_Books_Number, int _MAX_Users_Number, int _MAX_Borrowed_Books_Number, int _library_id, string _library_name)
    {
        MAX_Books_Number = _MAX_Books_Number;
        MAX_Users_Number = _MAX_Users_Number;
        MAX_Borrowed_Books_Number = _MAX_Borrowed_Books_Number;
        curr_book = 0;
        curr_user = 0;
        Number_of_borrowed_transaction = 0;
        library_id = _library_id;
        library_name = _library_name;
        book_array = new Book[MAX_Books_Number]{};
        // we must also apply rule of three for those 2 new arrays
        library_users_array = new user[MAX_Users_Number]{};
        library_borrowed_books = new Borrowed_Book[MAX_Borrowed_Books_Number]{};
    }

    ~Library_Books()
    {
        delete[] book_array;
        delete[] library_users_array;
        delete[] library_borrowed_books;
    }

    Library_Books(const Library_Books &other)
    {
        MAX_Books_Number = other.MAX_Books_Number;
        MAX_Users_Number = other.MAX_Users_Number;
        MAX_Borrowed_Books_Number = other.MAX_Borrowed_Books_Number;
        curr_book = other.curr_book;
        curr_user = other.curr_user;
        Number_of_borrowed_transaction = other.Number_of_borrowed_transaction;
        library_id = other.library_id;
        library_name = other.library_name;
        book_id_generator = other.book_id_generator;
        user_id_generator = other.user_id_generator;
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
        if (other.library_users_array == nullptr)
        {
            library_users_array = nullptr;
        }
        else
        {
            library_users_array = new user[MAX_Users_Number];
            for (int i{0}; i < curr_user; i++)
            {
                library_users_array[i] = other.library_users_array[i];
            }
        }
        if (other.library_borrowed_books == nullptr)
        {
            library_borrowed_books = nullptr;
        }
        else
        {
            library_borrowed_books = new Borrowed_Book[MAX_Borrowed_Books_Number];
            for (int i{0}; i < Number_of_borrowed_transaction; i++)
            {
                library_borrowed_books[i] = other.library_borrowed_books[i];
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
        this->MAX_Users_Number = other.MAX_Users_Number;
        this->MAX_Borrowed_Books_Number = other.MAX_Borrowed_Books_Number;
        this->curr_book = other.curr_book;
        this->curr_user = other.curr_user;
        this->Number_of_borrowed_transaction = other.Number_of_borrowed_transaction;
        this->library_id = other.library_id;
        this->library_name = other.library_name;
        this->book_id_generator = other.book_id_generator;
        this->user_id_generator = other.user_id_generator;
        delete[] this->book_array;
        this->book_array = new Book[MAX_Books_Number];
        for (int i{0}; i < curr_book; i++)
        {
            this->book_array[i] = other.book_array[i];
        }
        delete[] this->library_users_array;
        this->library_users_array = new user[MAX_Users_Number];
        for (int i{0}; i < curr_user; i++)
        {
            this->library_users_array[i] = other.library_users_array[i];
        }
        delete[] this->library_borrowed_books;
        this->library_borrowed_books = new Borrowed_Book[MAX_Borrowed_Books_Number];
        for (int i{0}; i < Number_of_borrowed_transaction; i++)
        {
            this->library_borrowed_books[i] = other.library_borrowed_books[i];
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
        book_array[curr_book] = Book(book_id_generator.generate_id(), name, quantity);
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
        // symmetric with rule 2: cannot delete a book that still has copies out
        // on an active loan -- deletion must not erase a party to an active loan.
        if (has_active_loan_for_book(Book_id))
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
    // now lets work around update we make
    Dynamic_arr<Book *> get_books_by_prefix(string Book_prefix)
    {
        Dynamic_arr<Book *> query_res = Dynamic_arr<Book *>();
        if (curr_book <= 0)
        {
            return query_res;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (is_prefix(book_array[i].name, Book_prefix) && (book_array[i].is_deleted == false))
            {
                query_res.add_element(&book_array[i]);
            }
        }
        return query_res;
    }

    Dynamic_arr<Book *> get_all_books()
    {
        Dynamic_arr<Book *> query_res = Dynamic_arr<Book *>();
        if (curr_book <= 0)
        {
            return query_res;
        }
        for (int i{0}; i < curr_book; i++)
        {
            if (book_array[i].is_deleted == false)
            {
                query_res.add_element(&book_array[i]);
            }
        }
        return query_res;
    }
    // lets write the function add user
    bool Add_User(string name, string national_id, string email, string address, string phone_number)
    {
        /*
        1.check if library reach is limited number of users(array cannot have more)
        2.create user object then add to array
        -care : this function should be resposible about checking that user informations are correct before calling constructor .
        */
        if (MAX_Users_Number == curr_user)
        {
            return false;
        }
        // generate id
        int user_id = user_id_generator.generate_id();
        library_users_array[curr_user] = user(user_id, name, national_id, email, address, phone_number);
        curr_user++;
        return true;
    }
    /*
    rule 2: cannot delete a user who still has an active loan. a loan means a
    physical book is in their hands; deleting the borrower would leave the book
    checked out to a "gone" user. soft-delete only marks is_deleted, so this keeps
    "deleted" meaning what it says.
    */
    bool delete_user(int user_id)
    {
        int idx{get_user_idx_by_id(user_id)};
        if (idx == -1 || library_users_array[idx].is_deleted)
        {
            return false;
        }
        if (has_active_loan_for_user(user_id))
        {
            return false;
        }
        library_users_array[idx].delete_user();
        return true;
    }
    /*
    ban / unban are pure ADMIN actions (admin decides when). they are deliberately
    NOT coupled to fees or loans: banning a user with an active loan is allowed
    (it only stops NEW borrows), and unban is free. the admin can look at what a
    user owes and decide, but the system does not force that coupling. note also:
    payment (user_add_money_to_a_loan) is intentionally allowed regardless of ban
    status -- money owed must always be payable, or a banned user could deadlock.
    */
    bool ban_user(int user_id)
    {
        int idx{get_user_idx_by_id(user_id)};
        if (idx == -1 || library_users_array[idx].is_deleted)
        {
            return false;
        }
        library_users_array[idx].ban_user();
        return true;
    }
    bool unban_user(int user_id)
    {
        int idx{get_user_idx_by_id(user_id)};
        if (idx == -1 || library_users_array[idx].is_deleted)
        {
            return false;
        }
        library_users_array[idx].unban_user();
        return true;
    }
    int get_user_idx_by_id(int user_id)
    {
        int user_idx{-1};
        for (int i{0}; i < curr_user; i++)
        {
            if (library_users_array[i].id == user_id)
            {
                user_idx = i;
                break;
            }
        }
        return user_idx;
    }
    /*
    shared "is there an active loan?" helpers. one place answers this question so
    delete_user, delete_book, and return all use the SAME logic instead of three
    hand-copied scans (which is where copy-paste bugs kept coming from).
    an active loan = a recorded loan that is not finished (book still out).
    */
    bool has_active_loan_for_user(int user_id)
    {
        for (int i{0}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].user_id == user_id && library_borrowed_books[i].is_finished == false)
            {
                return true;
            }
        }
        return false;
    }
    bool has_active_loan_for_book(int book_id)
    {
        for (int i{0}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].book_id == book_id && library_borrowed_books[i].is_finished == false)
            {
                return true;
            }
        }
        return false;
    }
    bool borrow_book(int user_id, int book_id, int Year, int month, int month_day, float fees, bool is_fully_paid, float what_is_paid = -1)
    {
        /*
        validate first, mutate last. checks in order:
        1. capacity for a new transaction
        2. book exists, not deleted, has an available copy   (finder + guards)
        3. user exists, not deleted, not banned              (finder + guards)
        4. fee inputs are valid
        then: construct the loan, record it, decrement quantity.
        */
        if (Number_of_borrowed_transaction == MAX_Borrowed_Books_Number)
        {
            return false;
        }
        // 2. book: neutral finder answers "where is it?", caller applies "do I accept it?"
        int found_book_idx{get_book_index_by_id(book_id)};
        if (found_book_idx == -1)
        {
            return false;
        }
        if (book_array[found_book_idx].is_deleted || book_array[found_book_idx].quantity <= 0)
        {
            return false;
        }
        // 3. user: same pattern
        int found_user_idx{get_user_idx_by_id(user_id)};
        if (found_user_idx == -1)
        {
            return false;
        }
        if (library_users_array[found_user_idx].is_deleted || library_users_array[found_user_idx].is_band_from_borrowing)
        {
            return false;
        }
        // 4. fee inputs
        bool is_input_valid{true};
        if (fees < 0)
        {
            is_input_valid = false;
        }
        else
        {
            if (!is_fully_paid)
            {
                if (what_is_paid < 0 || what_is_paid > fees)
                {
                    is_input_valid = false;
                }
            }
        }
        if (!is_input_valid)
        {
            return false;
        }
        // all checks passed -> mutate
        library_borrowed_books[Number_of_borrowed_transaction] = Borrowed_Book(user_id, book_id, Year, month, month_day, fees, is_fully_paid, what_is_paid);
        Number_of_borrowed_transaction++;
        book_array[found_book_idx].quantity--;
        return true;
    }
    bool user_add_money_to_a_loan(int book_id, int user_id, float money)
    {
        // find loan
        for (int i{}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].book_id == book_id && library_borrowed_books[i].user_id == user_id && library_borrowed_books[i].is_finished == false)
            {
                if (library_borrowed_books[i].increment_what_is_paid(money))
                {
                    return true;
                }
                break;
            }
        }
        // this means the loan does not exist or thier someting wrong with addition
        return false;
    }
    bool user_return_book(int user_id, int book_id)
    {
        // make sure book exist and not deleted, user exist and not deleted/banned,
        // and an ACTIVE loan for this pair exists. finders answer "where", guards answer "accept?".
        int found_book_idx{get_book_index_by_id(book_id)};
        if (found_book_idx == -1)
        {
            return false;
        }
        if (book_array[found_book_idx].is_deleted)
        {
            return false;
        }
        int found_user_idx{get_user_idx_by_id(user_id)};
        if (found_user_idx == -1)
        {
            return false;
        }
        // policy: a BAN does not block returning a book. a ban stops NEW borrows,
        // but returning is discharging an obligation, not a privilege -- blocking
        // it would strand the book forever (banned user can't return, and rule 2/3
        // won't let us delete the user/book while the loan is active). we only
        // block a deleted user from transacting.
        if (library_users_array[found_user_idx].is_deleted)
        {
            return false;
        }
        // find the active loan for this (user, book) pair
        int transaction_idx{-1};
        for (int i{0}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].book_id == book_id && library_borrowed_books[i].user_id == user_id && library_borrowed_books[i].is_finished == false)
            {
                transaction_idx = i;
                break;
            }
        }
        if (transaction_idx == -1)
        {
            return false;
        }
        /*
        mark transaction finished + set real_return time via the loan's own method
        (single source of truth for "finishing a loan"), then increase quantity.
        note: user_return_book() returns false unless fees are fully paid -- policy
        kept intentionally: a book cannot be returned until its fees are fully paid.
        */
        if (library_borrowed_books[transaction_idx].user_return_book() == false)
        {
            return false;
        }
        book_array[found_book_idx].quantity++;
        return true;
    }
    // lets write 4 queries :
    /*
    query 1 : books borrowed by user we should return
    */
    Dynamic_arr<DTO_User_Borrowed_Book> get_user_active_loans(int user_id)
    {
        // active view: user must exist and not be deleted
        int user_idx{get_user_idx_by_id(user_id)};
        if (user_idx == -1 || library_users_array[user_idx].is_deleted)
        {
            return Dynamic_arr<DTO_User_Borrowed_Book>{};
        }
        Dynamic_arr<DTO_User_Borrowed_Book> res{};
        // search all loans that have id of this user and not finished
        for (int i{}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].user_id == user_id && library_borrowed_books[i].is_finished == false)
            {
                // we need to get book nane
                int book_idx = get_book_index_by_id(library_borrowed_books[i].book_id);
                string book_name = (book_idx == -1) ? "" : book_array[book_idx].name;
                // create a DTO_User variable and add
                DTO_User_Borrowed_Book user_borrow_instance{DTO_User_Borrowed_Book(book_name,
                                                                                   "",
                                                                                   library_borrowed_books[i].borrow_date,
                                                                                   library_borrowed_books[i].supposed_return_date,
                                                                                   library_borrowed_books[i].real_return_data,
                                                                                   library_borrowed_books[i].fees,
                                                                                   library_borrowed_books[i].what_is_paid,
                                                                                   library_borrowed_books[i].is_fully_paid,
                                                                                   library_borrowed_books[i].is_finished)};
                res.add_element(user_borrow_instance);
            }
        }
        return res;
    }
    Dynamic_arr<DTO_User_Borrowed_Book> get_user_history_loans(int user_id)
    {
        // history view: user must exist; deleted/banned does not matter (show everything)
        int user_idx{get_user_idx_by_id(user_id)};
        if (user_idx == -1)
        {
            return Dynamic_arr<DTO_User_Borrowed_Book>{};
        }
        Dynamic_arr<DTO_User_Borrowed_Book> res{};
        // all loans for this user, finished or not
        for (int i{}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].user_id == user_id)
            {
                // we need to get book nane
                int book_idx = get_book_index_by_id(library_borrowed_books[i].book_id);
                string book_name = (book_idx == -1) ? "" : book_array[book_idx].name;
                // create a DTO_User variable and add
                DTO_User_Borrowed_Book user_borrow_instance{DTO_User_Borrowed_Book(book_name,
                                                                                   string(""),
                                                                                   library_borrowed_books[i].borrow_date,
                                                                                   library_borrowed_books[i].supposed_return_date,
                                                                                   library_borrowed_books[i].real_return_data,
                                                                                   library_borrowed_books[i].fees,
                                                                                   library_borrowed_books[i].what_is_paid,
                                                                                   library_borrowed_books[i].is_fully_paid,
                                                                                   library_borrowed_books[i].is_finished)};
                res.add_element(user_borrow_instance);
            }
        }
        return res;
    }
    /*
    query 2 list all users who borrowed a book
    */
    Dynamic_arr<user *> get_all_user_who_actively_borrow_a_book()
    {
        Dynamic_arr<user *> res{};
        for (int i{}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].is_finished == false)
            {
                /*
                from user id get it is index from index you have obj so you can get the address
                */
                int user_idx{get_user_idx_by_id(library_borrowed_books[i].user_id)};
                if (user_idx == -1)
                {
                    // i do not think this is even possible bcs this means that there is a borrow instance wih non-existence user which is not possible bcs i ckeck if user exist or not before borrow
                    continue;
                }
                res.add_element(&library_users_array[user_idx]);
            }
        }
        return res;
    }
    Dynamic_arr<user *> get_all_user_in_system()
    {
        Dynamic_arr<user *> res{};
        for (int i{}; i < curr_user; i++)
        {
            res.add_element(&library_users_array[i]);
        }
        return res;
    }
    /*
    query 3 : same as query 1 but from a book prespective but thier a point here we need to return name of user also instead of the book name
    */
    Dynamic_arr<DTO_User_Borrowed_Book> get_active_loans_for_certain_book(int book_id)
    {
        // make sure book exist and it is not deleted
        Dynamic_arr<DTO_User_Borrowed_Book> res{};
        for (int i{}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].book_id == book_id && library_borrowed_books[i].is_finished == false)
            {
                int user_idx{get_user_idx_by_id(library_borrowed_books[i].user_id)};
                if (user_idx == -1)
                {
                    continue;
                }
                string user_name{library_users_array[user_idx].name};
                DTO_User_Borrowed_Book user_borrow_instance{DTO_User_Borrowed_Book("",
                                                                                   user_name,
                                                                                   library_borrowed_books[i].borrow_date,
                                                                                   library_borrowed_books[i].supposed_return_date,
                                                                                   library_borrowed_books[i].real_return_data,
                                                                                   library_borrowed_books[i].fees,
                                                                                   library_borrowed_books[i].what_is_paid,
                                                                                   library_borrowed_books[i].is_fully_paid,
                                                                                   library_borrowed_books[i].is_finished)};
                res.add_element(user_borrow_instance);
            }
        }
        return res;
    }
    Dynamic_arr<DTO_User_Borrowed_Book> get_all_loans_for_certain_book(int book_id)
    {
        // make sure book exist and it is not deleted
        Dynamic_arr<DTO_User_Borrowed_Book> res{};
        for (int i{}; i < Number_of_borrowed_transaction; i++)
        {
            if (library_borrowed_books[i].book_id == book_id)
            {
                int user_idx{get_user_idx_by_id(library_borrowed_books[i].user_id)};
                if (user_idx == -1)
                {
                    continue;
                }
                string user_name{library_users_array[user_idx].name};
                DTO_User_Borrowed_Book user_borrow_instance{DTO_User_Borrowed_Book("",
                                                                                   user_name,
                                                                                   library_borrowed_books[i].borrow_date,
                                                                                   library_borrowed_books[i].supposed_return_date,
                                                                                   library_borrowed_books[i].real_return_data,
                                                                                   library_borrowed_books[i].fees,
                                                                                   library_borrowed_books[i].what_is_paid,
                                                                                   library_borrowed_books[i].is_fully_paid,
                                                                                   library_borrowed_books[i].is_finished)};
                res.add_element(user_borrow_instance);
            }
        }
        return res;
    }
    /*
    query 4 : all actively-borrowed books
    */
    Dynamic_arr<Book *> get_all_borrowed_book()
    {
        /*
        to make this correct iterate in the book_array and just check if it have a instance of borrow_book that is not finished
        doing in this order will make your life easier
        */
        Dynamic_arr<Book *> res{};
        for (int i{}; i < curr_book; i++)
        {
            if (book_array[i].is_deleted == false)
            {
                // go find if this book
                for (int j{}; j < Number_of_borrowed_transaction; j++)
                {
                    if (library_borrowed_books[j].book_id == book_array[i].id && library_borrowed_books[j].is_finished == false)
                    {
                        res.add_element(&book_array[i]);
                        break;
                    }
                }
            }
        }
        return res;
    }
};

int main()
{

    return 0;
}