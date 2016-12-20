#include <iostream>
using namespace std;
#include "Book.h"

int Book::B = 2;
Book::Book()
{
    cout << "This is the book constructer." << endl;
}

Book::~Book()
{
    cout << "This is the book destoryer." << endl;
}
