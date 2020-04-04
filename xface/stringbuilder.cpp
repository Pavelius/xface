#include "stringbuilder.h"

extern "C" int		memcmp(const void* p1, const void* p2, unsigned size);
extern "C" void*	memmove(void* destination, const void* source, unsigned size);
extern "C" void*	memcpy(void* destination, const void* source, unsigned size);
static const char	spaces[] = {" \n\t\r.,!?;:"};
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

const char* stringbuilder::readvariable(const char* p) {
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
		while(*p && (ischa(*p) || isnum(*p) || *p == '_')) {
			if(ps < pe)
				*ps++ = *p++;
			else
				break;
		}
	}
	*ps = 0;
	addidentifier(temp);
	return p;
}

void stringbuilder::adduint(unsigned value, int precision, const int radix) {
	char temp[32]; int i = 0;
	if(!value) {
		add("0");
		return;
	}
	while(value) {
		temp[i++] = (value % radix);
		value /= radix;
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