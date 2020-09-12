#include "crt.h"
#include "color.h"

#pragma once

enum group_s : unsigned char {
	IllegalSymbol,
	WhiteSpace, Operator, Keyword, Comment,
	Number, String, Identifier,
	OpenParam, CloseParam, OpenBlock, CloseBlock, OpenScope, CloseScope,
};
struct groupi {
	struct visuali {
		color				present;
		unsigned			flags;
	};
	const char*				name;
	visuali					visual;
};
struct lexer {
	struct word {
		const char*			name;
		unsigned			size;
		group_s				type;
	};
	const char*				name;
	const char*				extensions;
	arem<word>				words;
	const char*				whitespaces;
	constexpr lexer() : name(0), extensions(0), whitespaces(0), words() {}
	void					add(const char* keyword_name, group_s type);
	const word*				find(const char* sym) const;
	void					sort();
};
struct codepos {
	group_s					type;
	int						from, count;
	int						line, column;
};
class codemodel : arem<char> {
	const lexer*			lex;
	int						p1, p2;
	void					correct();
public:
	virtual void			changing() {}
	void					clear();
	bool					isidentifier(const char* source, const char** v) const;
	bool					iskeyword(const char* source, const lexer::word** v) const;
	bool					isnextline(const char* source, const char** pv) const;
	bool					isselected() const { return p2 != -1 && p1 != -1; }
	bool					iswhitespace(char sym) const;
	bool					iswhitespace(const char* sym, const char** v) const;
	void					left(bool shift, bool ctrl);
	const char*				get(int index) const { return data + index; }
	int						getbegin() const;
	int						getcurrent() const { return p1; }
	int						getend() const;
	int						getlenght() const;
	void					paste(const char* v);
	void					right(bool shift, bool ctrl);
	void					set(const char* source);
	void					set(const lexer* v) { lex = v; }
	void					set(int index, bool shift);
	void					getnext(codepos& e) const;
};