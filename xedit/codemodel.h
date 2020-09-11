#include "crt.h"
#include "color.h"

#pragma once

enum group_s : unsigned char {
	IllegalSymbol,
	WhiteSpace, Operator, Keyword,
	Number, String, Comment, Identifier,
	OpenParam, CloseParam, OpenBlock, CloseBlock, OpenScope, CloseScope,
};
struct visuali {
	color					present;
	unsigned				flags;
};
struct groupi {
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
	int						from, to;
	int						line, column;
};
class codemodel : arem<char> {
	const lexer*			lex;
	int						p1, p2;
	bool					modified;
	void					correct();
public:
	void					clear();
	bool					isidentifier(const char* source, const char** v) const;
	bool					iskeyword(const char* source, const lexer::word** v) const;
	bool					ismodified() const { return modified; }
	bool					iswhitespace(char sym) const;
	int						getbegin() const;
	int						getend() const;
	int						getlenght() const;
	void					paste(const char* v);
	void					set(const char* source);
	void					set(const lexer& v) { lex = &v; }
	void					set(int index, bool shift);
	void					setmodified(bool v) { modified = v; }
	void					getnext(codepos& e) const;
};