#include <iostream>
#include <chrono>
#include <math.h>
using namespace std;
void main()
{
	int n = 999999, sum = 0;
	auto nStart = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
	for (int i = 1; i <= n; i++)
		sum += sqrt(i);
	auto nEnd = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
	printf("Time: %d ms\n", nEnd - nStart);
	cout << "sum = " << sum << endl;
}