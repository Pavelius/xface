#include "crt.h"
#include "grammar.h"

static bool end(const char* s, const char* v) {
	while(*v) {
		if(*s++ != *v++)
			return false;
	}
	return *s == *v;
}

struct grammar_map {
	const char*		name;
	const char*		change;
	operator bool() const { return name != 0; }
};

static void change(char* r, const char* s, const grammar_map* source, const char* def) {
	auto ps = s;
	auto pe = zend(s);
	auto s1 = pe - ps;
	for(auto p = source; *p; p++) {
		auto s2 = zlen(p->name);
		if(s2 > s1)
			continue;
		if(memcmp(pe - s2, p->name, s2) == 0) {
			auto s3 = zlen(p->change);
			memcpy(r, s, s1 - s2);
			memcpy(r + (s1-s2), p->change, s3);
			r[s1 - s2 + s3] = 0;
			return;
		}
	}
	if(def) {
		zcpy(r, s);
		zcat(r, def);
	}
}

static char* adv_by(char* r, const char* s) {
	static grammar_map map[] = {
		{"ый", "ым"},
		{"ий", "им"},
		{}
	};
	change(r, s, map, "ом");
	return r;
}

char* grammar::of(char* r, const char* s) {
	static grammar_map map[] = {
		{"би", "би"},
		{"ты", "т"},
		{"сы", "сов"},
		{"ны", "н"},
		{"а", "ы"},
		{"ь", "и"},
		{"о", "а"},
		{"я", "и"},
		{}
	};
	change(r, s, map, "а");
	return r;
}

char* grammar::by(char* r, const char* s) {
	static grammar_map map[] = {
		{"би", "би"},
		{"ий", "ием"},
		{"й", "ем"}, {"ь", "ем"}, {"е", "ем"},
		{"а", "ой"},
		{"ч", "чем"},
		{}
	};
	change(r, s, map, "ом");
	return r;
}

char* grammar::to(char* r, const char* s) {
	static grammar_map map[] = {
		{"а", "е"},
		{"о", "у"},
		{"ы", "ам"},
		{}
	};
	change(r, s, map, "у");
	return r;
}

char* grammar::pluar::of(char* r, const char* s) {
	static grammar_map map[] = {
		{"ты", "т"},
		{"ы", ""},
		{"а", ""},
		{"и", "и"},
		{"ч", "чей"},
		{"ж", "жей"},
		{}
	};
	change(r, s, map, "ов");
	return r;
}

char* grammar::get(char* result, const char* result_maximum, const char* word, int count) {
	if(count <= 1) {
		zcpy(result, word);
	} else {
		szprint(result, result_maximum, "%1i ", count);
		auto p = zend(result);
		if(count <= 4)
			of(p, word);
		else
			pluar::of(p, word);
	}
	return result;
}