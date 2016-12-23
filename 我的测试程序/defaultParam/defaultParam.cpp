#include <iostream>
using namespace std;

void fun(int a, int b = 0, const char *c = NULL)
{
    cout << "a = " << a << endl;
    if (b != 0)
        cout << "b = " << b << endl;

    const char* tmp = c;
}

int main()
{
    fun(9);
    fun(9, 0);
    fun(9, 2);
}
