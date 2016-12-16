#include <iostream>
using namespace std;

typedef struct
{
    int a;
    int b;
}A;

typedef struct
{
    int a;
}B;

int main()
{
    A x{ 1, 2 };
    B *y = (B*)&x;
    cout << y->a << endl;
}