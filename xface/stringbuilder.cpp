#include "crt.h"
#include "stringbuilder.h"

static const char spaces[] = " \n\t\r.,!?;:";

static const char* psnum16(const char* p, int& value) {
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

static const char* psnum10(const char* p, int& value) {
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
const char* stringbuilder::read(const char* p, int& value) {
	value = 0;
	if(!p)
		return 0;
	bool sign = false;
	// ”становка знака
	if(*p == '-') {
		sign = true;
		p++;
	}
	// ѕерегрузка числовой системы
	if(p[0] == '0' && p[1] == 'x') {
		p += 2;
		p = psnum16(p, value);
	} else
		p = psnum10(p, value);
	if(sign)
		value = -value;
	return p;
}

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

const char* skipcr(const char* p) {
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
			// „исло в кодировке UNICODE
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			p = psnum10(p, value);
			r = szput(r, value);
			break;
			// „исло в кодировке UNICODE (16-рична€ система)
		case 'x': case 'u':
			p = psnum16(p + 1, value);
			r = szput(r, value);
			break;
		case '\n': case '\r':
			// ѕеревод строки в конце
			while(*p == '\n' || *p == '\r')
				p = skipcr(p);
			break;
		default:
			// Ћюбой символ, который будет экранирован ( \', \", \\)
			*r++ = *p++;
			break;
		}
	}
	return p;
}

bool szstart(const char* text, const char* name) {
	while(*name) {
		if(*name++ != *text++)
			return false;
	}
	return true;
}

bool szmatch(const char* text, const char* name) {
	while(*name) {
		if(*name++ != *text++)
			return false;
	}
	if(ischa(*text))
		return false;
	return true;
}

static bool szpmatch(const char* text, const char* s, const char* s2) {
	while(true) {
		const char* d = text;
		while(s < s2) {
			if(*d == 0)
				return false;
			unsigned char c = *s;
			if(c == '?') {
				s++;
				d++;
			} else if(c == '*') {
				s++;
				if(s == s2)
					return true;
				while(*d) {
					if(*d == *s)
						break;
					d++;
				}
			} else {
				if(*d++ != *s++)
					return false;
			}
		}
		return true;
	}
}

bool szpmatch(const char* text, const char* pattern) {
	const char* p = pattern;
	while(true) {
		const char* p2 = zchr(p, ',');
		if(!p2)
			p2 = zend(p);
		if(szpmatch(text, p, p2))
			return true;
		if(*p2 == 0)
			return false;
		p = skipsp(p2 + 1);
	}
}

struct stringbuilder::grammar {
	const char*		name;
	const char*		change;
	unsigned		name_size;
	unsigned		change_size;
	constexpr grammar() : name(0), change(0), name_size(0), change_size(0) {}
	constexpr grammar(const char* name, const char* change) :
		name(name), change(change), name_size(zlen(name)), change_size(zlen(change)) {
	}
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

void stringbuilder::lower() {
	for(auto p = pb; *p; p++)
		*p = lower(*p);
}

void stringbuilder::upper() {
	for(auto p = pb; *p; p++)
		*p = upper(*p);
}

void stringbuilder::addidentifier(const char* identifier) {
	addv("[-", 0);
	addv(identifier, 0);
	addv("]", 0);
}

const char* stringbuilder::read(const char* p, char* ps, const char* pe) {
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
	p = read(p, temp, temp + sizeof(temp) - 1);
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
		int pn = 0, pnp = 0;
		if(isnum(*src))
			src = psnum10(src, pn);
		if(src[0] == '.' && isnum(src[1]))
			src = psnum10(src + 1, pnp);
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

void stringbuilder::change(char s1, char s2) {
	for(auto p = pb; p < pe; p++) {
		if(*p == 0)
			break;
		if(*p == s1)
			*p++ = s2;
	}
}

void stringbuilder::change(const char* s1, const char* s2) {
	if(!s1 || s1[0] == 0)
		return;
	auto n1 = zlen(s1);
	auto n2 = zlen(s2);
	for(auto p = pb; p < pe; p++) {
		if(*p == 0)
			break;
		if(*p == s1[0]) {
			auto n = this->pe - p;
			if(n1 > n)
				return;
			if(memcmp(p, s1, n1) != 0)
				continue;
			auto c = n2;
			if(c > (pe - this->p - 1))
				c = pe - this->p - 1;
			if(c > n1)
				memmove(p + c, p + n1, pe - p - c);
			memcpy(p, s2, n2);
			p += n2 - 1;
		}
	}
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
	{"а€", "ой"},
	{"би", "би"},
	{"ты", "т"},
	{"сы", "сов"},
	{"ны", "н"},
	{"а", "ы"},
	{"ь", "и"},
	{"о", "а"},
	{"€", "и"},
	{}};
	add(s, map, "а");
}

void stringbuilder::addby(const char* s) {
	static grammar map[] = {{"а€", "ой"},
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