#include "stringbuilder.h"

extern "C" int		memcmp(const void* p1, const void* p2, unsigned size);
extern "C" void*	memmove(void* destination, const void* source, unsigned size);
extern "C" void*	memcpy(void* destination, const void* source, unsigned size);
static const char	spaces[] = " \n\t\r.,!?;:";
static constexpr const char* zend(const char* p) { while(*p) p++; return p; }
static constexpr unsigned zlen(const char* p) { return zend(p) - p; }

static const char* word_end(const char* ps) {
	while(*ps) {
		for(auto e : spaces) {
			if(*ps == e)
				return ps;
		}
		ps++;
	};
	return ps;
}

static const char* skip_space(const char* ps) {
	while(*ps) {
		for(auto e : spaces) {
			if(*ps != e)
				return ps;
		}
		ps++;
	};
	return ps;
}

static inline bool is_space(char sym) {
	for(auto e : spaces) {
		if(sym == e)
			return true;
	}
	return false;
}

const char* szskipcr(const char* p) {
	if(*p == '\n') {
		p++;
		if(*p == '\r')
			p++;
	} else if(*p == '\r') {
		p++;
		if(*p == '\n')
			p++;
	}
	return p;
}

const char* szskipcrr(const char* p0, const char* p) {
	if(!p)
		return 0;
	if(p0 >= p)
		return p;
	if(p[-1] == '\n') {
		p--;
		if(p0 >= p)
			return p;
		if(p[-1] == '\r')
			p--;
	} else if(p[-1] == '\r') {
		p--;
		if(p0 >= p)
			return p;
		if(p[-1] == '\n')
			p--;
	}
	return p;
}

//const char* psnum16(const char* p, int& value) {
//	int result = 0;
//	const int radix = 16;
//	while(*p) {
//		char a = *p;
//		if(a >= '0' && a <= '9') {
//			result = result * radix;
//			result += a - '0';
//		} else if(a >= 'a' && a <= 'f') {
//			result = result * radix;
//			result += a - 'a' + 10;
//		} else if(a >= 'A' && a <= 'F') {
//			result = result * radix;
//			result += a - 'A' + 10;
//		} else
//			break;
//		p++;
//	}
//	value = result;
//	return p;
//}
//
//const char* psnum10(const char* p, int& value) {
//	int result = 0;
//	const int radix = 10;
//	while(*p) {
//		char a = *p;
//		if(a >= '0' && a <= '9') {
//			result = result * radix;
//			result += a - '0';
//		} else
//			break;
//		p++;
//	}
//	value = result;
//	return p;
//}
//
//// Parse string to number
//const char* psnum(const char* p, int& value) {
//	value = 0;
//	if(!p)
//		return 0;
//	bool sign = false;
//	// Установка знака
//	if(*p == '-') {
//		sign = true;
//		p++;
//	}
//	// Перегрузка числовой системы
//	if(p[0] == '0' && p[1] == 'x') {
//		p += 2;
//		p = psnum16(p, value);
//	} else
//		p = psnum10(p, value);
//	if(sign)
//		value = -value;
//	return p;
//}
//
//// Parse string to string (from c/json format)
//const char* psstr(const char* p, char* r, char end_symbol) {
//	r[0] = 0;
//	if(!p)
//		return 0;
//	while(*p) {
//		if(*p == end_symbol) {
//			*r++ = 0;
//			return p + 1;
//		} else if(*p != '\\') {
//			*r++ = *p++;
//			continue;
//		}
//		p++;
//		int value;
//		switch(*p) {
//		case 'n':
//			*r++ = '\n';
//			p++;
//			break;
//		case 'r':
//			*r++ = '\r';
//			p++;
//			break;
//		case 't':
//			*r++ = '\t';
//			p++;
//			break;
//		case 'b':
//			*r++ = '\b';
//			p++;
//			break;
//		case 'f':
//			*r++ = '\f';
//			p++;
//			break;
//		case 'v':
//			*r++ = '\v';
//			p++;
//			break;
//			// Число в кодировке UNICODE
//		case '0':
//		case '1':
//		case '2':
//		case '3':
//		case '4':
//		case '5':
//		case '6':
//		case '7':
//		case '8':
//		case '9':
//			p = psnum10(p, value);
//			r = szput(r, value);
//			break;
//			// Число в кодировке UNICODE (16-ричная система)
//		case 'x':
//		case 'u':
//			p = psnum16(p + 1, value);
//			r = szput(r, value);
//			break;
//		case '\n':
//		case '\r':
//			// Перевод строки в конце
//			while(*p == '\n' || *p == '\r')
//				p = szskipcr(p);
//			break;
//		default:
//			// Любой символ, который будет экранирован ( \', \", \\)
//			*r++ = *p++;
//			break;
//		}
//	}
//	return p;
//}

struct stringbuilder::grammar {
	const char*		name;
	const char*		change;
	unsigned		name_size;
	unsigned		change_size;
	constexpr grammar() : name(0), change(0), name_size(0), change_size(0) {}
	grammar(const char* name, const char* change) :
		name(name), change(change), name_size(zlen(name)), change_size(zlen(change)) {}
	operator bool() const { return name != 0; }
};

unsigned char stringbuilder::upper(unsigned char u) {
	if(u >= 0x61 && u <= 0x7A)
		return u - 0x61 + 0x41;
	else if(u >= 0xE0)
		return u - 0xE0 + 0xC0;
	return u;
}

unsigned char stringbuilder::lower(unsigned char u) {
	if(u >= 0x41 && u <= 0x5A)
		return u - 0x41 + 0x61;
	else if(u >= 0xC0 && u <= 0xDF)
		return u - 0xC0 + 0xE0;
	return u;
}

void stringbuilder::addidentifier(const char* identifier) {
	addv("[-", 0);
	addv(identifier, 0);
	addv("]", 0);
}

const char* stringbuilder::readidn(const char* p, char* ps, const char* pe) {
	if(*p == '(') {
		p++;
		while(*p && *p != ')') {
			if(ps < pe)
				*ps++ = *p;
		}
		if(*p == ')')
			p++;
	} else {
		while(*p && (ischa(*p) || isnum(*p) || *p == '_')) {
			if(ps < pe)
				*ps++ = *p++;
			else
				break;
		}
	}
	*ps = 0;
	return p;
}

const char* stringbuilder::readvariable(const char* p) {
	char temp[260];
	p = readidn(p, temp, temp + sizeof(temp) - 1);
	addidentifier(temp);
	return p;
}

void stringbuilder::adduint(unsigned value, int precision, const int radix) {
	char temp[32]; int i = 0;
	if(!value)
		temp[i++] = 0;
	else {
		while(value) {
			temp[i++] = (value % radix);
			value /= radix;
		}
	}
	while(precision-- > i)
		add("0");
	while(i) {
		auto v = temp[--i];
		if(p < pe) {
			if(v < 10)
				*p++ = '0' + v;
			else
				*p++ = 'A' + (v - 10);
		}
	}
	p[0] = 0;
}

void stringbuilder::addint(int value, int precision, const int radix) {
	if(value < 0) {
		add("-");
		value = -value;
	}
	adduint(value, precision, radix);
}

const char* stringbuilder::readnum(const char* p1, int& result) {
	result = 0;
	bool sign = false;
	const int radix = 10;
	while(*p1 && *p1 != '-' && (*p1 < '0' || *p1 > '9'))
		p1++;
	if(*p1 == '-') {
		sign = true;
		p1++;
	}
	while(*p1) {
		char a = *p1;
		if(a < '0' || a > '9')
			break;
		result = result * radix;
		result += a - '0';
		p1++;
	}
	if(sign)
		result = -result;
	return p1;
}

const char* stringbuilder::readformat(const char* src, const char* vl) {
	if(*src == '%') {
		src++;
		if(p < pe)
			*p++ = '%';
		*p = 0;
		return src;
	}
	*p = 0;
	char prefix = 0;
	if(*src == '+' || *src == '-')
		prefix = *src++;
	if(*src >= '0' && *src <= '9') {
		// Если число, просто подставим нужный параметр
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
			addint(value, pnp, 10);
		} else if(*src == 'h') {
			src++;
			adduint((unsigned)(((int*)vl)[pn - 1]), pnp, 16);
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
	} else {
		auto p0 = p;
		src = readvariable(src);
		switch(prefix) {
		case '-': *p0 = lower(*p0); break;
		case '+': *p0 = upper(*p0); break;
		default: break;
		}
	}
	return src;
}

void stringbuilder::addv(const char* src, const char* vl) {
	if(!p)
		return;
	p[0] = 0;
	if(!src)
		return;
	while(true) {
		switch(*src) {
		case 0: *p = 0; return;
		case '%': src = readformat(src + 1, vl); break;
		default:
			if(p < pe)
				*p++ = *src;
			src++;
			break;
		}
	}
}

void stringbuilder::addsep(char separator) {
	if(p <= pb || p >= pe)
		return;
	if(p[-1] == separator)
		return;
	switch(separator) {
	case ' ':
		if(p[-1] == '\n' || p[-1] == '\t')
			return;
		break;
	case '.':
		if(p[-1] == '?' || p[-1] == '!' || p[-1] == ':')
			return;
		break;
	}
	*p++ = separator;
}

void stringbuilder::addx(char separator, const char* format, const char* format_param) {
	if(!format || format[0] == 0)
		return;
	addsep(separator);
	addv(format, format_param);
}

void stringbuilder::addx(const char* separator, const char* format, const char* format_param) {
	if(!format || format[0] == 0)
		return;
	if(p != pb)
		add(separator);
	addv(format, format_param);
}

void stringbuilder::addicon(const char* id, int value) {
	if(value < 0)
		adds(":%1:[-%2i]", id, -value);
	else
		adds(":%1:%2i", id, value);
}

void stringbuilder::add(const char* s, const grammar* source, const char* def) {
	auto ps = skip_space(s);
	while(*ps) {
		auto pw = word_end(ps);
		unsigned s1 = pw - ps;
		auto found = false;
		for(auto pg = source; *pg; pg++) {
			auto s2 = pg->name_size;
			if(pg->name_size > s1)
				continue;
			if(memcmp(pw - s2, pg->name, s2) == 0) {
				auto s3 = pg->change_size;
				memcpy(p, ps, s1 - s2);
				memcpy(p + (s1 - s2), pg->change, s3);
				p += (s1 - s2 + s3);
				if(pw[0] == 0)
					def = 0;
				found = true;
				break;
			}
		}
		if(!found) {
			memcpy(p, ps, pw - ps);
			p += pw - ps;
		}
		ps = pw;
		while(*ps) {
			if(is_space(*ps))
				*p++ = *ps++;
			break;
		}
	}
	p[0] = 0;
	if(def)
		add(def);
}

void stringbuilder::addof(const char* s) {
	static grammar map[] = {{"ый", "ого"},
	{"ий", "ого"},
	{"ое", "ого"},
	{"ая", "ой"},
	{"би", "би"},
	{"ты", "т"},
	{"сы", "сов"},
	{"ны", "н"},
	{"а", "ы"},
	{"ь", "и"},
	{"о", "а"},
	{"я", "и"},
	{}};
	add(s, map, "а");
}

void stringbuilder::addby(const char* s) {
	static grammar map[] = {{"ая", "ой"},
	{"ый", "ым"}, {"ое", "ым"}, {"ой", "ым"},
	{"би", "би"},
	{"ий", "им"},
	{"ец", "цем"},
	{"ки", "ками"},
	{"й", "ем"}, {"ь", "ем"}, {"е", "ем"},
	{"а", "ой"},
	{"ч", "чем"},
	{}};
	add(s, map, "ом");
}

void stringbuilder::addto(const char* s) {
	static grammar map[] = {{"а", "е"},
	{"о", "у"},
	{"ы", "ам"},
	{}
	};
	add(s, map, "у");
}

char* szprint(char* result, const char* result_maximum, const char* src, ...) {
	stringbuilder e(result, result_maximum);
	e.addv(src, xva_start(src));
	return e;
}

char* szprintv(char* result, const char* result_maximum, const char* format, const char* format_param) {
	stringbuilder e(result, result_maximum);
	e.addv(format, format_param);
	return e;
}