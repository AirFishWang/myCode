//#include <stdio.h>
//#include <string.h>
#include <iostream>
//using namespace std;


int main()
{
	char a[3] = {'e'};
	std::cout << strlen(a) << std::endl;  //计算到NULL之前， 所以是不确定的长度

	char src[] = "ag";
	char dst[] = "abc";
	strcpy(dst, src);            //运行错误

    char *p = "hello world";
    //p[0] = 'k';  //会运行出错
	return 0;
}