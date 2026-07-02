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

/*
lets talk a little bit about the requirment :
rember that app is admin side so admin :
 1.admin can add user (so the user is allowed to borrow books latter on )
 2.user can borrow a book
 3.user can return the book it borrow
 -lets talk more about borrowing a book :
 user can borrow a book from start date with specific duaraion from the start date and duration it should know the return data ,and of course there is a fee
 the fee can be (fully paid - partialy paid- not paid) idk if this is allowed in most libraries but lets be general i guess , and of course we need to make sure the
 quntity of book that user want to borrow is not 0 simple , now lets take a deeper look at building the object borrowed book we can make each user have an array of borrowed books
 that represent all the books that user borrowed this is a solution but from DB prespective bcs relation of user and books are to many to many it make sense to make it as seperate object
 that have user id and book id and i think this make the code much cleaner even if we are not using db lets go with this approach i think after i learn a little about ctime that i will use doing the idea of duration is
 pain in the ass so i decide that admin will make the borrowed day of the book and i actually belive it makes more sense .
 -lets talk more deeply about the return book function when a user return a book what should happen it will only change the state of object borrowed_book to complete or done and it will record
 the data that user return book  (this is of course is much better from simple just delete the borrowed book object bro it allow first of all know to know which books are most borrowed , which user returns book on time
 calclute all the money that come in last month and so on as ml engineer/studen data like this is valiable)
*/
struct user
{
    int id;
    string name;
    string national_id;
    string email;
    string address;
    string phone_number;
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
    // creating this object means that user borrow the book now lets
    Borrowed_Book(int _user_id, int _book_id, int Year, int month, int month_day, float _fees, bool _is_fully_paid, float _what_is_paid = -1)
    {
        // so we will assume that all the inputs are valid and the constructor will never be called if not all the inputs are valid so there is another function that will be responsible
        // to call this constructor or start object of type Borrowed_Book
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
        // now we have the correct supposed return that admin enter now conver it to time_t to make it easier to represent .
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
    // we can impose a behavour here and it is check if the user still have full pay fees or not
    // it can indicate it pay the remaing money seperate it from book return
    // why the reason i build this part like this the problem is that you cannot depend on float in comparsion that what i belive bcs of c++ approixmation this is cruatal thing in
    // banking system i do not it is here in our case but i make is fully paid varaible to make the admin decide when fees is fully paid
    /*
    lets think more about fees while i feel like it is uncessary and we just give control to varaible what_is_paid and fees and is_fully_paid full control to the admin but lets
    do it , we want to make to handle all cases that could happen
    first at constructor user could fully_paid the fee or partially bad it , if the admin chose fully paid that means he does not need to enter what_is_paid and if admin choose not fully paid
    he will set the what_is_paid varaible (he must set it) we can impose this by making -1 by defualt . (we already done this )!
    now admin should be apple to check how much user have to pay if fees is not fully paid (we did this)
    the admin could set the what_is_paid and fees varaible wrongly this actually get me into fundamental problem i did not think of i think admin should have control over all
    varaible (almost all) in this project he already have bcs in struct every variable is public by defualt but we should not let him access them immedaitly we should put guard around them
    for example national number and email have clear constrains to be written and phone number too and and , but i did not take this as consiern in this project
    any way in this project i will let him only update the what_is_paid varaible
    i also choice that if admin update the _what_is_paid he need to click it is fully paid or we maybe do another change that admin can do 2 things
    1.update the what is paid and if it become eql to fees it fully_paid varaible become true and notify admin also admin can add money to already what is paid and
    if it becomeq eql to fees it updated it self to true and notify him is this good or over engineering
    the admin can click fully paid button(function and it will just mark user as fully paid and _what_is_paid eql to fees make it easy and short cut )
    */
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
    // we will do both increment pay and full update what is paid and of admin add money to user what is paid and it become
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
    // i will actually remove the set function bcs it increase complexity for nothing and it does not really represent reallity i put it for another reason i will tell you why
    bool pay_full_fees()
    { // i will keep bcs i think it is a cool feature for admin in busy day of work
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
        // now here mean that user return this as finished aka user return the book
        if (!is_fully_paid)
        {
            return false;
        }
        real_return_data = time(NULL);
        is_finished = true;
        return true;
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