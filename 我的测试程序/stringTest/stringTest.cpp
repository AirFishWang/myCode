//#include <stdio.h>
//#include <string.h>
#include <iostream>
//using namespace std;


int main()
{
	char a[3] = {'e'};
	std::cout << strlen(a) << std::endl;  //计算到NULL之前， 所以是不确定的长度

	char src[] = "abcdefg";
	char *dst = "abc";
	strcpy(dst, src);            //运行错误
	return 0;
}