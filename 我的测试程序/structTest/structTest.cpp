#include <iostream>
using namespace std;

typedef struct
{
    int a;
    int b;
    bool c; 
    float d;
}A;

typedef struct
{
    int a;
}B;

int main()
{
    //A x{ 1, 2 };
    //B *y = (B*)&x;
    //cout << y->a << endl;

    A a1{1,2, true,0.2}, a2;
    a2 = a1;
    cout << a1.a << endl << a1.b << endl << a1.c << endl << a1.d << endl;
    cout << a2.a << endl << a2.b << endl <<a2.c << endl << a2.d << endl;
}