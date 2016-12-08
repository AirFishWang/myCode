#include "../MyDll/MyDll.h"
#include <iostream>
using namespace std;

//extern "C" _declspec(dllimport) int dlladd(int, int);

int main()
{
    cout << "2 + 3 = " << dlladd(2, 3) << endl;  
}