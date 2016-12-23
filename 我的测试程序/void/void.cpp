#include <iostream>
using namespace std;

int main()
{
    char *p = new char('a');
    void *p2 = (void *)p;
    char *p3 = (char *)p2;
    cout << *p << endl;
}