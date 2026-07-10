#include <stdio.h>
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
        fees = -1;
        what_is_paid = -1;
        is_late = false;
    }
    DTO_User_Borrowed_Book(string _Book_Name, time_t _borrow_date, time_t _supposed_return_date, time_t _real_return_date, float _fees, float _what_is_paid, bool _is_fully_paid, bool _is_finished)
    {
        Book_Name = _Book_Name;
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
    bool borrow_book(int user_id, int book_id, int Year, int month, int month_day, float fees, bool is_fully_paid, float what_is_paid = -1)
    {
        /*
        lets focus on logic of this function first we need to check :
        1.book id exist and it is not deleted and quntitu is larger thatn 0 (will not decrement quntity here ).
        2.user exist and not deleted or banned .
        --we must check input is valid before calling constructor .
        3.check that year , month , month_day are not illogical or wrong ex : year : 3200 (not wrong but illogical there must be upper limit for library you cannot borrow book for more than 3 month or something ) and month within 1-12 , days 0-31 (day maybe more complicated bcs range of day depend on month) i will skip this part
        4.check fees is not negative number , then we need to check is fully paid or not if is fully paid is true we do not care about what_is_paid other wise if is_fully_paid is false we need to check that what is paid is pos and lower that fees
        5.construct object , put it in array and decrement quntity .
        */
        if (Number_of_borrowed_transaction == MAX_Borrowed_Books_Number)
        {
            return false;
        }
        bool book_does_not_exist{true};
        bool is_book_deleted{true};
        bool is_book_not_avalaible{true};
        int found_book_idx{-1};
        for (int i{0}; i < curr_book; i++)
        {
            if (book_array[i].id == book_id)
            {
                book_does_not_exist = false;
                found_book_idx = i;
                if (book_array[i].is_deleted == false)
                {
                    is_book_deleted = false;
                    if (book_array[i].quantity > 0)
                    {
                        is_book_not_avalaible = false;
                    }
                }
                break;
            }
        }
        if (book_does_not_exist || is_book_deleted || is_book_not_avalaible)
        {
            return false;
        }
        bool user_does_not_exist{true};
        bool is_user_deleted{true};
        bool is_user_banned{true};
        for (int i{0}; i < curr_user; i++)
        {
            if (library_users_array[i].id == user_id)
            {
                user_does_not_exist = false;
                if (library_users_array[i].is_deleted == false)
                {
                    is_user_deleted = false;
                    if (library_users_array[i].is_band_from_borrowing == false)
                    {
                        is_user_banned = false;
                    }
                }
                break;
            }
        }
        if (user_does_not_exist || is_user_deleted || is_user_banned)
        {
            return false;
        }
        // ceck input is valid
        bool is_input_valid{true};
        if (fees < 0)
        {
            is_input_valid = false;
        }
        else
        {
            if (!is_fully_paid)
            {
                // here i need to check the what_is_paid too
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
        library_borrowed_books[Number_of_borrowed_transaction] = Borrowed_Book(user_id, book_id, Year, month, month_day, fees, is_fully_paid, what_is_paid);
        Number_of_borrowed_transaction++;
        book_array[found_book_idx].quantity--;
        return true;
    }
};

int main()
{
    return 0;
}