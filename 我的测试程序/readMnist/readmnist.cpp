#include <stdio.h>
#include <stdint.h>

void main()
{
	FILE *fp;

	fopen_s(&fp, "exerciseImage.txt", "rb");
	if (!fp) printf("There is no file!\n");
	int magic, num_images, rows, cols;
	uint8_t pixel;
	fread(&magic, sizeof(int), 1, fp);
	fread(&num_images, sizeof(int), 1, fp);
	fread(&rows, sizeof(int), 1, fp);
	fread(&cols, sizeof(int), 1, fp);
	fread(&pixel, sizeof(uint8_t), 1, fp);
	printf("%d\t%d\t%d\t%d\t%d\n", magic, num_images, rows, cols, pixel);
	fclose(fp);

	///////////////////////////////////////////////////////////////////////////////////

	fopen_s(&fp, "train-labels.idx1-ubyte", "rb");       //¶Á±êÇ©
	if (!fp) printf("There is no file!\n");
	uint8_t label;
	fread(&magic, sizeof(int), 1, fp);
	fread(&num_images, sizeof(int), 1, fp);
	fread(&label, sizeof(uint8_t), 1, fp);
	printf("%d\t%d\t%d\n", magic, num_images, label);
	fread(&label, sizeof(uint8_t), 1, fp);
	printf("%d\t%d\t%d\n", magic, num_images, label);
	fclose(fp);

	///////////////////////////////////////////////////////////////////////////////////

	fopen_s(&fp, "exerciseLabel.txt", "rb");       //¶Á±êÇ©
	if (!fp) printf("There is no file!\n");

	fread(&magic, sizeof(int), 1, fp);
	fread(&num_images, sizeof(int), 1, fp);
	fread(&label, sizeof(uint8_t), 1, fp);
	printf("%d\t%d\t%d\n", magic, num_images, label);
	fread(&label, sizeof(uint8_t), 1, fp);
	printf("%d\t%d\t%d\n", magic, num_images, label);
	fclose(fp);
	
	
}