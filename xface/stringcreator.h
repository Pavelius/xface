#pragma once

#define xva_start(v) ((const char*)&v + sizeof(v))

struct stringcreator {
	constexpr stringcreator(char* pb, const char* pe) : pb(pb), p(pb), pe(pe) { pb[0] = 0; }
	template<unsigned N> constexpr stringcreator(char(&result)[N]) : stringcreator(result, result + N) {}
	constexpr operator char*() const { return pb; }
	void				add(const char* format, ...) { addv(format, xva_start(format)); }
	void				addicon(const char* id, int value);
	virtual void		addidentifier(const char* identifier);
	static char*		addint(char* result, const char* result_maximum, int value, int precision, const int radix);
	void				addn(const char* format, ...) { addx("\n", format, xva_start(format)); }
	void				adds(const char* format, ...);
	void				addsz() { if(p < pe) *p++ = 0; }
	void				addv(const char* format, const char* format_param);
	void				addx(const char* separator, const char* format, const char* format_param);
	static char*		adduint(char* result, const char* result_maximum, unsigned value, int precision, const int radix);
	char*				begin() { return pb; }
	const char*			begin() const { return pb; }
	void				clear() { pb[0] = 0; p = pb; }
	const char*			end() const { return pe; }
	char*				get() const { return p; }
	static bool			ischa(unsigned char sym) { return (sym >= 'A' && sym <= 'Z') || (sym >= 'a' && sym <= 'z') || sym>=0xC0; }
	static bool			isnum(unsigned char sym) { return sym >= '0' && sym <= '9'; }
	bool				ispos(const char* v) const { return p == v; }
	static unsigned char lower(unsigned char sym);
	static unsigned char upper(unsigned char sym);
private:
	char*				p;
	char*				pb;
	const char*			pe;
	const char*			readformat(const char* format, const char* format_param);
	const char*			readvariable(const char* format);
};