#pragma once

#define xva_start(v) ((const char*)&v + sizeof(v))

class stringbuilder {
	struct grammar;
	char*				p;
	char*				pb;
	const char*			pe;
	const char*			readformat(const char* format, const char* format_param);
	const char*			readvariable(const char* format);
	void				add(const char* s, const grammar* source, const char* def = 0);
public:
	constexpr stringbuilder(char* pb, const char* pe) : p(pb), pb(pb), pe(pe) {}
	template<unsigned N> constexpr stringbuilder(char(&result)[N]) : stringbuilder(result, result + N - 1) {}
	constexpr operator char*() const { return pb; }
	void				add(const char* format, ...) { addv(format, xva_start(format)); }
	void				add(const char sep) { char temp[2] = {sep, 0}; addv(temp, 0); }
	void				addby(const char* s);
	virtual void		addidentifier(const char* identifier);
	void				addicon(const char* id, int value);
	void				addint(int value, int precision, const int radix);
	void				addn(const char* format, ...) { addx('\n', format, xva_start(format)); }
	void				addof(const char* s);
	void				adds(const char* format, ...) { addx(' ', format, xva_start(format)); }
	void				addsep(char separator);
	void				addsz() { if(p < pe) *p++ = 0; }
	void				addto(const char* s);
	void				addv(const char* format, const char* format_param);
	void				addx(char separator, const char* format, const char* format_param);
	void				addx(const char* separator, const char* format, const char* format_param);
	void				adduint(unsigned value, int precision, const int radix);
	char*				begin() { return pb; }
	const char*			begin() const { return pb; }
	void				change(char s1, char s2);
	void				change(const char* s1, const char* s2);
	void				clear() { pb[0] = 0; p = pb; }
	const char*			end() const { return pe; }
	char*				get() const { return p; }
	unsigned			getlenght() const { return pb - p; }
	unsigned			getmaximum() const { return pe - pb - 1; }
	static bool			ischa(unsigned char sym) { return (sym >= 'A' && sym <= 'Z') || (sym >= 'a' && sym <= 'z') || sym >= 0xC0; }
	static bool			isnum(unsigned char sym) { return sym >= '0' && sym <= '9'; }
	bool				ispos(const char* v) const { return p == v; }
	static unsigned char lower(unsigned char sym);
	void				lower();
	static const char*	read(const char* p, char* result, const char* result_end);
	static const char*	read(const char* p, int& result);
	void				set(char* v) { p = v; p[0] = 0; }
	static unsigned char upper(unsigned char sym);
	void				upper();
};
// Callback function for title, header or getting name
typedef const char* (*fntext)(const void* object, stringbuilder& sb);
template<class T> const char* getnm(const void* object, stringbuilder& sb) { return ((T*)object)->name; }