#include <stdio.h>
#include <iostream>

using namespace std;

// ok now lets design the code so it go along with all our design chcoses

struct Book
{
    int id;
    string name;
    int quantity;

    Book()
    {
        id = -1;
        name = "";
        quantity = 0;
    }

    Book(int _id, string _name, int _quantity)
    {
        id = _id;
        name = _name;
        quantity = _quantity;
    }
};
/*
Using global variables is generally considered bad practice because they offer no control or boundaries; any class or function across your codebase can access and modify them without restrictions.

For example, if you encapsulate an id_generator variable and its logic inside a struct or class, any component in your codebase that needs an ID must explicitly call the generate_id function. It can no longer directly access or alter the underlying max_id variable. By forcing access through a function, you gain the ability to implement necessary guards and controls—such as checking if you have reached the maximum possible ID limit.

On a deeper level, this encapsulation makes it much easier to implement and reason about synchronization and race conditions. Ultimately, it provides the logical control required to safely manage and understand how your variable changes state.

int const MAX_Books_Number = 1000;
int curr_book = 0;
int current_id = 0;
int generate_id()
{
    current_id++;
    return current_id - 1;
}
*/

/*
Of course, this approach is much better than using a global variable for the reasons we discussed. The question now is: should I make this static or not?

Making it static means that the max_id variable and the generate_id function will be shared across the entire codebase. This would be perfectly fine if we only ever have one library, meaning we would only need a single ID generator for the books in that library.

However, what if we want to scale? Even though scaling won't actually happen in this specific project, I have another reason to make it non-static: logically, an ID generator is not a universal tool. It is specifically tied to generating IDs for a single, distinct library. Therefore, it makes much more sense to encapsulate it directly within the library instance itself.
*/
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

struct Library_Books
{
    int MAX_Books_Number;
    int curr_book;
    int library_id;
    string library_name;
    Id_Generator id_generator;
    // i think this should an pointer to array (you cannot set size of the array before user enter it)
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
        /*
        I believe my previous approach was incorrect because it allocated memory on the stack
        instead of the heap. If I initialize a temporary array like 'Book temp_book[MAX_Books_Number]{};',
        it is placed on the stack frame of the constructor.

        Since stack variables are destroyed when their enclosing function exits, and a constructor
        is fundamentally a function that finishes executing after initialization, any temporary stack
        variable created inside it will be destroyed. Therefore, I cannot safely assign a local
        stack array to 'book_array'. Should I use dynamic memory allocation to allocate this array
        on the heap instead?
        */
        book_array = new Book[MAX_Books_Number]{};
    }

    bool add_book(string name, int quantity)
    {
        // basic checks over input
        if (name.size() < 3 || name.size() > 70 || quantity <= 0)
        {
            return false;
        }
        if(curr_book >= MAX_Books_Number){
            return false ; 
        }
        // make it simple you will just add book
        book_array[curr_book] = Book(id_generator.generate_id(), name , quantity);
        curr_book++;
        return true ;
    }
};

int main()
{
    return 0;
}