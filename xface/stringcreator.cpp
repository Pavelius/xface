#include "stringcreator.h"

extern "C" void* memcpy(void* destination, const void* source, unsigned size);

unsigned char stringcreator::upper(unsigned char u) {
	if(u >= 0x61 && u <= 0x7A)
		return u - 0x61 + 0x41;
	else if(u >= 0xE0)
		return u - 0xE0 + 0xC0;
	return u;
}

unsigned char stringcreator::lower(unsigned char u) {
	if(u >= 0x41 && u <= 0x5A)
		return u - 0x41 + 0x61;
	else if(u >= 0xC0 && u <= 0xDF)
		return u - 0xC0 + 0xE0;
	return u;
}

void stringcreator::addidentifier(const char* identifier) {
	addv("[-", 0);
	addv(identifier, 0);
	addv("]", 0);
}

const char* stringcreator::readvariable(const char* p) {
	char temp[260];
	auto ps = temp;
	auto pe = temp + sizeof(temp) - 1;
	if(*p == '(') {
		p++;
		while(*p && *p != ')') {
			if(ps < pe)
				*ps++ = *p;
		}
		if(*p == ')')
			p++;
	} else {
		while(*p || ischa(*p) || isnum(*p) || *p=='_') {
			if(ps < pe)
				*ps++ = *p;
		}
	}
	*ps = 0;
	addidentifier(temp);
	return p;
}

char* stringcreator::adduint(char* dst, const char* result_max, unsigned value, int precision, const int radix) {
	char temp[32]; int i = 0;
	if(!value) {
		if(dst<result_max)
			*dst++ = '0';
		if(dst<result_max)
			*dst = 0;
		return dst;
	}
	if(!result_max)
		result_max = dst + 32;
	while(value) {
		temp[i++] = (value % radix);
		value /= radix;
	}
	while(precision-- > i) {
		if(dst < result_max)
			*dst++ = '0';
	}
	while(i) {
		auto v = temp[--i];
		if(dst < result_max) {
			if(v < 10)
				*dst++ = '0' + v;
			else
				*dst++ = 'A' + (v - 10);
		}
	}
	dst[0] = 0;
	return dst;
}

char* stringcreator::addint(char* dst, const char* result_max, int value, int precision, const int radix) {
	if(value < 0) {
		if(dst < result_max)
			*dst++ = '-';
		value = -value;
	}
	return adduint(dst, result_max, value, precision, radix);
}

const char* stringcreator::readformat(const char* src, const char* vl) {
	if(*src == '%') {
		auto sym = *src++;
		if(p < pe)
			*p++ = sym;
		*p = 0;
		return src;
	}
	*p = 0;
	char prefix = 0;
	if(*src == '+' || *src == '-')
		prefix = *src++;
	if(*src >= '0' && *src <= '9') {
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
			if(prefix == '+' && value >= 0) {
				if(p < pe)
					*p++ = '+';
			}
			p = addint(p, pe, value, pnp, 10);
		} else if(*src == 'h') {
			src++;
			p = adduint(p, pe, (unsigned)(((int*)vl)[pn - 1]), pnp, 16);
		} else {
			if(((char**)vl)[pn - 1]) {
				auto p0 = p;
				auto p1 = ((char**)vl)[pn - 1];
				while(*p1 && p < pe)
					*p++ = *p1++;
				if(p < pe)
					*p = 0;
				switch(prefix) {
				case '-': *p0 = lower(*p0); break;
				case '+': *p0 = upper(*p0); break;
				default: break;
				}
			}
		}
	} else
		src = readvariable(src);
	return src;
}

void stringcreator::addv(const char* src, const char* vl) {
	if(!p)
		return;
	if(!src) {
		// Error: No source string
		p[0] = 0;
		return;
	}
	while(true) {
		switch(*src) {
		case 0:
			*p = 0;
			return;
		case '%':
			src = readformat(src + 1, vl);
			break;
		default:
			if(p < pe)
				*p++ = *src;
			src++;
			break;
		}
	}
}

void stringcreator::addx(const char* separator, const char* format, const char* format_param) {
	if(p > pb)
		addv(separator, 0);
	addv(format, format_param);
}

void stringcreator::adds(const char* format, ...) {
	if(p > pb)
		addv(" ", 0);
	addv(format, xva_start(format));
}

void stringcreator::addicon(const char* id, int value) {
	if(value < 0)
		adds(":%1:[-%2i]", id, -value);
	else
		adds(":%1:%2i", id, value);
}

char* szprintvs(char* result, const char* result_maximum, const char* src, const char* vl) {
	stringcreator e(result, result_maximum);
	e.addv(src, vl);
	return e;
}

char* szprint(char* result, const char* result_maximum, const char* src, ...) {
	stringcreator e(result, result_maximum);
	e.addv(src, xva_start(src));
	return e;
}