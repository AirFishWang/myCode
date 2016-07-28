#include <iostream>
using namespace std;

template<class T1, class T2> T1 add(T1 x, T2 y)
{
	return x + y;
}



void main()
{
	cout << add(9, 4.0)/3 << endl;
}