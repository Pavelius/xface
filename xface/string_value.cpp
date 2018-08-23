#include "crt.h"

const char* psnum16(const char* p, int& value) {
	int result = 0;
	const int radix = 16;
	while(*p) {
		char a = *p;
		if(a >= '0' && a <= '9') {
			result = result * radix;
			result += a - '0';
		} else if(a >= 'a' && a <= 'f') {
			result = result * radix;
			result += a - 'a' + 10;
		} else if(a >= 'A' && a <= 'F') {
			result = result * radix;
			result += a - 'A' + 10;
		} else
			break;
		p++;
	}
	value = result;
	return p;
}

const char* psnum10(const char* p, int& value) {
	int result = 0;
	const int radix = 10;
	while(*p) {
		char a = *p;
		if(a >= '0' && a <= '9') {
			result = result * radix;
			result += a - '0';
		} else
			break;
		p++;
	}
	value = result;
	return p;
}

// Parse string to number
const char* psnum(const char* p, int& value) {
	value = 0;
	if(!p)
		return 0;
	bool sign = false;
	// Установка знака
	if(*p == '-') {
		sign = true;
		p++;
	}
	// Перегрузка числовой системы
	if(p[0] == '0' && p[1] == 'x') {
		p += 2;
		p = psnum16(p, value);
	} else
		p = psnum10(p, value);
	if(sign)
		value = -value;
	return p;
}

// Parse string to string (from c/json format)
const char* psstr(const char* p, char* r, char end_symbol) {
	r[0] = 0;
	if(!p)
		return 0;
	while(*p) {
		if(*p == end_symbol) {
			*r++ = 0;
			return p + 1;
		} else if(*p != '\\') {
			*r++ = *p++;
			continue;
		}
		p++;
		int value;
		switch(*p) {
		case 'n':
			*r++ = '\n';
			p++;
			break;
		case 'r':
			*r++ = '\r';
			p++;
			break;
		case 't':
			*r++ = '\t';
			p++;
			break;
		case 'b':
			*r++ = '\b';
			p++;
			break;
		case 'f':
			*r++ = '\f';
			p++;
			break;
		case 'v':
			*r++ = '\v';
			p++;
			break;
			// Число в кодировке UNICODE
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			p = psnum10(p, value);
			r = szput(r, value);
			break;
			// Число в кодировке UNICODE (16-ричная система)
		case 'x':
		case 'u':
			p = psnum16(p + 1, value);
			r = szput(r, value);
			break;
		case '\n':
		case '\r':
			// Перевод строки в конце
			while(*p == '\n' || *p == '\r')
				p = szskipcr(p);
			break;
		default:
			// Любой символ, который будет экранирован ( \', \", \\)
			*r++ = *p++;
			break;
		}
	}
	return p;
}

const char* psidn(const char* p, char* r, char* re) {
	*r = 0;
	while(*p) {
		if((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p)) {
			if(r < re)
				*r++ = *p;
			p++;
		} else
			break;
	}
	*r = 0;
	return p;
}