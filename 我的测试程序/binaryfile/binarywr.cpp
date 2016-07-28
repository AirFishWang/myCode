#include<stdio.h>
#include <stdint.h>
#define N 10
void main()
{
	FILE *fp;
	fopen_s(&fp,"data.txt","wb");
	if (!fp) printf("There is no file!\n");
	uint8_t a[N];
	for (int i = 0; i < N; i++)
	{
		a[i] = i;
		fwrite(a+i, sizeof(uint8_t), 1, fp);

	}
	fclose(fp);

	fopen_s(&fp, "data.txt", "rb");
	if (!fp) printf("There is no file!\n");
	uint8_t *p = new uint8_t;
	for (int i = 0; i < N; i++)
	{
		fread(p, sizeof(uint8_t), 1, fp);
		printf("%d\n", *p);
	}
	fclose(fp);
}