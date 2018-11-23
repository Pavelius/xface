#include "crt.h"

char* sznum(char* outbuf, float f, int precision, const char* empthy) {
	typedef union {
		long	L;
		float	F;
	} LF_t;
	long mantissa, int_part, frac_part;
	short exp2;
	LF_t x;
	outbuf[0] = 0;
	if(f == 0.0) {
		if(empthy)
			zcpy(outbuf, empthy);
		return outbuf;
	}
	x.F = f;

	exp2 = (unsigned char)(x.L >> 23) - 127;
	mantissa = (x.L & 0xFFFFFF) | 0x800000;
	frac_part = 0;
	int_part = 0;

	if(exp2 >= 31)
		return 0;
	else if(exp2 < -23)
		return 0;
	else if(exp2 >= 23)
		int_part = mantissa << (exp2 - 23);
	else if(exp2 >= 0) {
		int_part = mantissa >> (23 - exp2);
		frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
	} else
		frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);

	auto p = outbuf;

	if(x.L < 0)
		*p++ = '-';

	if(int_part == 0)
		*p++ = '0';
	else {
		sznum(p, int_part, 0, 0, 10);
		while(*p)
			p++;
	}
	if(frac_part == 0) {
		if(precision) {
			*p++ = '.';
			for(int i = 0; i < precision; i++)
				*p++ = 0;
		}
	} else {
		int max = 7;
		*p++ = '.';
		if(precision)
			max = precision;
		/* print BCD */
		for(int m = 0; m < max; m++) {
			/* frac_part *= 10;	*/
			frac_part = (frac_part << 3) + (frac_part << 1);
			*p++ = (frac_part >> 24) + '0';
			frac_part &= 0xFFFFFF;
		}
		if(precision == 0) {
			for(--p; p[0] == '0' && p[-1] != '.'; --p);
			++p;
		}
	}
	*p = 0;
	return outbuf;
}