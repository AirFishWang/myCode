#include <iostream>
using namespace std;

typedef enum{
    A = -1,
    B,
    C = 9,
    D,
    E,
    F
}letter;

int main()
{
    letter l;
    l = B;
    cout << A << endl;
    cout << B << endl;
    cout << C << endl;
    cout << D << endl;
    cout << E << endl;
    cout << F << endl;
}