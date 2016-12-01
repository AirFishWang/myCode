#include <iostream>
using namespace std;

int f()
{
    cout << "this is f\n";
    return 1;
}

int g()
{
    cout << "this is g\n";
    return 2;
}

int h()
{
    cout << "this is h\n";
    return 3;
}

void main()
{
    int x = f() + g() + h();
    cout << "x = " << x << endl;
}