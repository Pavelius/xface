#include "crt.h"
#include "stringcreator.h"

void stringcreator::parseidentifier(char* result, const char* result_max, const char* identifier) {
	auto len = zlen(identifier) + 3;
	if((result_max - result) >= len) {
		zcat(result, "[-");
		zcat(result, identifier);
		zcat(result, "]");
	}
}

void stringcreator::parsevariable(char* result, const char* result_max, const char** format) {
	char temp[260];
	auto src = *format;
	int s = 0;
	if(*src == '(') {
		*format += 1;
		while(*src && *src != ')')
			src++;
		s = src - *format;
		src++;
	} else {
		while(*src) {
			const char* s1 = src;
			unsigned ch = szget(&src);
			if(!ischa(ch) && !isnum(ch) && ch != '_') {
				src = s1;
				break;
			}
		}
		s = src - *format;
	}
	temp[0] = 0;
	if(s != 0 && s<int(sizeof(temp) - 1)) {
		memcpy(temp, *format, s);
		temp[s] = 0;
	}
	*format = src;
	parseidentifier(result, result_max, temp);
}

char* stringcreator::parsenumber(char* dst, const char* result_max, unsigned value, int precision, const int radix) {
	char temp[32]; int i = 0;
	if(!value) {
		zcpy(dst, "0");
		return dst;
	}
	if(!result_max)
		result_max = dst + 32;
	while(value) {
		temp[i++] = (value % radix);
		value /= radix;
	}
	while(precision-- > i) {
		if(dst<result_max)
			*dst++ = '0';
	}
	while(i) {
		auto v = temp[--i];
		if(dst < result_max) {
			if(v < 10)
				*dst++ = '0' + v;
			else
				*dst++ = 'A' + (v-10);
		}
	}
	dst[0] = 0;
	return dst;
}

char* stringcreator::parseint(char* dst, const char* result_max, int value, int precision, const int radix) {
	if(value < 0) {
		if(dst<result_max)
			*dst++ = '-';
		value = -value;
	}
	return parsenumber(dst, result_max, value, precision, radix);
}

const char* stringcreator::parseformat(char* dst, const char* result_max, const char* src, const char* vl) {
	if(*src == '%') {
		auto sym = *src++;
		if(dst < result_max)
			*dst++ = sym;
		*dst = 0;
		return src;
	}
	*dst = 0;
	bool prefix_plus = false;
	if(*src == '+') {
		prefix_plus = true;
		src++;
	}
	if(*src >= '1' && *src <= '9') {
		// ≈сли число, просто подставим нужный параметр
		int pn = 0, pnp = 0;
		while(isnum(*src))
			pn = pn * 10 + (*src++) - '0';
		if(src[0] == '.' && (src[1] >= '0' && src[1] <= '9')) {
			src++;
			while(*src >= '0' && *src <= '9')
				pnp = pnp * 10 + (*src++) - '0';
		}
		if(*src == 'i') {
			src++;
			auto value = ((int*)vl)[pn - 1];
			if(prefix_plus && value >= 0) {
				if(dst<result_max)
					*dst++ = '+';
			}
			dst = parseint(dst, result_max, value, pnp, 10);
		} else if(*src == 'h') {
			src++;
			dst = parsenumber(dst, result_max, (unsigned)(((int*)vl)[pn - 1]), pnp, 16);
		} else {
			if(((char**)vl)[pn - 1])
				zcpy(dst, ((char**)vl)[pn - 1], result_max - dst);
		}
	} else
		parsevariable(dst, result_max, &src);
	return src;
}

void stringcreator::printv(char* result, const char* result_maximum, const char* src, const char* vl) {
	if(!result)
		return;
	if(!src) {
		result[0] = 0;
		return;
	}
	while(true) {
		switch(*src) {
		case 0:
			*result = 0;
			return;
		case '%':
			src = parseformat(result, result_maximum, src + 1, vl);
			result = zend(result);
			if(result > result_maximum)
				result = const_cast<char*>(result_maximum);
			break;
		default:
			if(result < result_maximum)
				*result++ = *src;
			src++;
			break;
		}
	}
}

void stringcreator::print(char* result, const char* result_maximum, const char* src, ...) {
	printv(result, result_maximum, src, xva_start(src));
}

char* szprintvs(char* result, const char* result_maximum, const char* src, const char* vl) {
	stringcreator e;
	e.printv(result, result_maximum, src, vl);
	return result;
}

char* szprint(char* result, const char* result_maximum, const char* src, ...) {
	stringcreator e;
	e.printv(result, result_maximum, src, xva_start(src));
	return result;
}