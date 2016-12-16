#include <iostream>
using namespace std;

typedef struct{
    int a;
    int b;
    char c;
}S;

int main()
{
    int a[3] = { 1, 2, 3 };
    int (*p)[3] = &a;
    cout << (*p)[2] << endl;


    S s{ 1, 2, 'f' };
    int *p2 = (int *)&s;
    cout << *p2 << endl;
}