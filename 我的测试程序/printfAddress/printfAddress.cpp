#include <stdio.h>
void main()
{
	int a = 1;
	printf("%d\n", &a);
	printf("%x\n", &a);
	printf("%p\n", &a);
	printf("%d\n", sizeof(&a));
}
