#ifndef __PREDEAL__
#define __PREDEAL__ 1


#define ABC
#ifndef ABC
gjskl
#endif

#include <iostream>
using namespace std;
#define g(a) a
#define f(a) #a
void main()
{
	int number	= 10;
	cout << "__PREDEAL__ = " << __PREDEAL__ << endl;
	cout << g(number) << endl;
	cout << f(number) << endl;
	cout << __LINE__ << endl;
    cout << __FILE__ << endl;
    cout << ABC << endl;
}



#endif