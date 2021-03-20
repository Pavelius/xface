#include "crt.h"
#include "color.h"
#include "point.h"

#pragma once

enum group_s : unsigned char {
	IllegalSymbol,
	WhiteSpace, Operator, Keyword, Comment,
	Number, String, Identifier, Type,
	OpenParam, CloseParam, OpenBlock, CloseBlock, OpenScope, CloseScope, OpenUser, CloseUser,
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
	vector<word>			words;
	constexpr lexer() : name(0), extensions(0), words() {}
	void					add(const char* keyword_name, group_s type);
	const word*				find(const char* sym) const;
	void					sort();
};
struct codepos {
	group_s					type;
	int						from, count;
	int						line, column;
};
struct typei {
	const char*				name;
	unsigned				size;
};
class parseri {
	vector<typei>			types;
public:
	void					addtype(const char* id);
	const typei*			find(const char* sym) const;
};
class codemodel : public vector<char> {
	const lexer*			lex;
	const parseri*			parser;
public:
	bool					isidentifier(char sym) const;
	bool					isidentifier(const char* source, const char** v) const;
	bool					iskeyword(const char* source, const lexer::word** v) const;
	bool					isliteral(const char* source, const char** pv) const;
	bool					isnextline(const char* source, const char** pv) const;
	bool					istype(const char* source, const typei** pv) const;
	bool					iswhitespace(char sym) const;
	bool					iswhitespace(const char* sym, const char** v) const;
	const char*				get(int index) const { return ptr(index); }
	int						getindex(point pt) const;
	int						getlenght() const;
	int						getnext(const char* ps, point& from, group_s& group) const;
	void					getstate(int p1, point& pos1, int p2, point& pos2, point& size, const point origin, int& origin_index) const;
	int						lineb(int index) const;
	int						linee(int index) const;
	void					set(const char* source);
	void					set(const lexer* v) { lex = v; }
	void					set(const parseri* v) { parser = v; }
	int						skipsp(int index) const;
	int						skipnsp(int index) const;
};