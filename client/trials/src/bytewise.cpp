#include <iostream>

using namespace std;

template <class T>
void byBitRTL(T val) {
	uint8_t size = sizeof(val);
	
	//T mask = 1 << (size * CHAR_BIT - 1);
	T mask = 1;
	
	cout << "byBit " << (unsigned long)val
		<< endl << "size: " << (unsigned long)size
		<< endl << "mask: " << (unsigned long)mask
		<< endl;
	
	for(size_t i = 0; i < size * CHAR_BIT; i++) {
		cout << (unsigned long)mask << " & " << (unsigned long)val << " = " << (mask & val ? '1' : '0') << endl;
		//cout << (mask & val ? '1' : '0');
		mask = mask << 1;
	}
	cout << endl;
	
}

template <class T>
void byBit(T val) {
	size_t shift = CHAR_BIT * sizeof(val);
	T mask = 1 << (shift - 1);
	//T mask = 1;
	
	/*cout << "byBit " << (unsigned long)val
		<< endl << "size: " << (unsigned long)size
		<< endl << "mask: " << (unsigned long)mask
		<< endl << "mask shift " << shift
		<< endl;
	*/
	
	for(size_t i = 0; i < shift; i++) {
		//cout << (unsigned long)mask << " & " << (unsigned long)val << " = " << (mask & val ? '1' : '0') << endl;
		cout << (mask & val ? '1' : '0');
		mask = mask >> 1;
	}
	cout << endl;
	
}

template <class T>
void byByte(T val) {
	uint8_t size = sizeof(val);
	T mask = 255;
	for(uint8_t i = size; i > 0;) {
		i--;
		T m1 = val >> (8 * i);
		cout << " " << (int)(m1 & mask);
	}	
}

int main() {
	// cout << "hello\n";
	
	// uint8_t b = 1;
	
	//b = (128 + 64) >> 2;
	
	//cout << "b:" << (int)b << endl << "b & x = " << (int)(b & 2) << endl;
	
	byBit((unsigned int)128);
	//byBitRTL((unsigned int)128);
	byByte(65128);
	
	//byByte(123456789123456789);
}