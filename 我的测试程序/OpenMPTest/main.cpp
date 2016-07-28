#include <iostream>
#include <time.h>
#include <omp.h>
using namespace std;
void test()
{
	int a = 0;
	for (int i = 0; i<100000000; i++)
		a++;

}

bool loop()
{
#pragma omp parallel for
	for (int i = 0; i < 21; i++)
		test();
	return true;
}
int main()
{
	clock_t t1 = clock();
	loop();
	clock_t t2 = clock();
	int coreNum = omp_get_num_procs();//获得处理器个数
	cout << "time: " << t2 - t1 << "ms" << endl;
	cout << "处理器的数量：" << coreNum << endl;
}