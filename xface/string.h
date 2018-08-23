#pragma once

class string {
	const char*	pb;
	const char*	pe;
	constexpr const char* zend(const char* p) { while(*p) p++; return p; }
public:
	constexpr string(const char* p) : pb(p), pe(zend(p)) {}
	constexpr string(const char* pb, const char* pe) : pb(pb), pe(pe) {}
	constexpr string() : pb(""), pe("") {}
	char& operator[] (unsigned pos) { return ((char*)pb)[pos]; }
	explicit operator bool() const { return pb < pe; }
	constexpr const char* begin() const { return pb; }
	void clear() { pb = pb = ""; }
	constexpr string& get(unsigned pos, unsigned n) const { return string((pb + pos > pe) ? pe : pb + pos, (pb + pos + n > pe) ? pe : pb + pos + n); }
	char* get(char* result) const { auto n = lenght(); for(unsigned i = 0; i < n; i++) result[i] = pb[i]; }
	constexpr const char* end() const { return pe; }
	constexpr string& left(unsigned n) const { return string(pb, (pb + n > pe) ? pe : pb + n); }
	constexpr unsigned lenght() const { return pe - pb; }
	constexpr string& right(unsigned n) const { return string(pb, (pe - n < pb) ? pb : pe - n); }
};