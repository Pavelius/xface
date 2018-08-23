#include "crt.h"
#include "dice.h"

dice dice::create(int c, int d, int b, int m) {
	return{(unsigned char)c, (unsigned char)d, (char)b, (char)m};
}

int dice::roll(int c, int d) {
	if(!d)
		return c;
	int result = 0;
	for(int i = 0; i < c; i++)
		result += 1 + (rand() % d);
	return result;
}

int dice::roll() const {
	return (m ? roll(c, d)*m : roll(c, d)) + b;
}

char* dice::print(char* result, const char* result_maximum) const {
	szprints(result, result_maximum, "%1id%2i", c, d);
	if(b != 0)
		szprints(zend(result), result_maximum, "%+1i", b);
	return result;
}

char* dice::range(char* result, const char* result_maximum) const {
	szprints(result, result_maximum, "%1i-%2i", c + b, c*d + b);
	return result;
}