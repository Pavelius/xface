#include "crt.h"

char* sznum(char* result, int num, int precision, const char* empthy, int radix) {
	char* p1 = result;
	if(num == 0) {
		if(empthy)
			zcpy(p1, empthy);
		else {
			zcpy(p1, "0");
			while(--precision > 0)
				zcat(p1, "0");
		}
		p1 = zend(p1);
	} else {
		char temp[32];
		int p = 0;
		if(num < 0) {
			*p1++ = '-';
			num = -num;
		}
		switch(radix) {
		case 16:
			while(num) {
				int a = (num % radix);
				if(a > 9)
					temp[p++] = 'A' - 10 + a;
				else
					temp[p++] = '0' + a;
				num /= radix;
			}
			break;
		default:
			while(num) {
				temp[p++] = '0' + (num % radix);
				num /= radix;
			}
			break;
		}
		while(precision-- > p)
			*p1++ = '0';
		while(p)
			*p1++ = temp[--p];
		p1[0] = 0;
	}
	return result;
}

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