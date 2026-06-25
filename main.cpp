#include<stdio.h>
#include<iostream>

using namespace std ; 

struct Book
{
    int id ;
    string name ; 
    int quantity ;

    Book(){
        id = -1 ; 
        name = "" ;
        quantity = 0 ; 
    }

    Book(int _id , string _name , int _quantity){
        id = _id ;
        name = _name ;
        quantity = _quantity ; 
    } 
};
//maybe it is better to put all of this in struct 
int const MAX_Books_Number = 1000 ;
int curr_book = 0 ; 
int current_id = 0 ; 

int generate_id(){
    current_id++;
    return current_id - 1 ; 
}

struct Library_Books{
    int  MAX_Books_Number;
    int curr_book;
    //i think this should an pointer to array (you cannot set size of the array before user enter it)
    Book *book_array  ;

    Library_Books(){
        int  MAX_Books_Number = -1 ;
        int curr_book = -1; 
        book_array = nullptr ;
    }

    Library_Books(int _MAX_Books_Number , int _curr_book){
        int  MAX_Books_Number = _MAX_Books_Number ;
        int curr_book = _curr_book; 
        Book temp_book[MAX_Books_Number] {} ;
        book_array = (temp_book) ;
    }

    bool add_book(string name , int quantity){
    //basic checks over input 
    if(name.size() < 3 || name.size() > 70 || quantity <= 0){
        return false ; 
    }
    //which one is better you know that book_array had is full of book object 
    book_array[curr_book].id = generate_id() ;
    book_array[curr_book].name = name ;
    book_array[curr_book].quantity = quantity ;
    //make it simple you will just add book
    book_array[curr_book] = Book(generate_id() , name , quantity);
    curr_book ++ ;
} 
};


int main(){
    return 0 ; 
}