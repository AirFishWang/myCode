#include <iostream>
using namespace std;
#include <time.h>
void main(){
	clock_t t1, t2, t3;
	int n = 10000000;
	int sum1 = 0;
	int sum2 = 0;
	int _ = 9;
	t1 = clock();
	for (int i = 0; i < n; i++)
		sum1 += i;
	t2 = clock();

	sum2 = n * (n + 1) / 2;

	t3 = clock();

	cout << "t2 - t1 = " << t2 - t1 << "ms" << endl;
	cout << "t3 - t2 = " << t3 - t2 << "ms" << endl;
}