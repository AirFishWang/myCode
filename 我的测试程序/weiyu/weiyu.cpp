#include<iostream>
using namespace std;

typedef struct test
{   
    int a : 7;
    int b : 1;
}test;

int main()
{
    test t;
    t.b = 1;
    cout << t.b << endl;
}