#include <iostream>
#include <stdint.h>

using namespace std;

void main()
{
	cout << "sizeof(uchar) = " << sizeof(unsigned char) << endl;
	cout << "sizeof(uint8_t) = " << sizeof(uint8_t) << endl;

	unsigned char a = 255;
	uint8_t b = 255;
	if (a == b) cout << "a = b = 255" << endl;
}