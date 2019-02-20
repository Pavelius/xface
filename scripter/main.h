#include "xface/collection.h"
#include "xface/stringcreator.h"

const char*				szdup(const char *text);

#pragma once

namespace code {

struct metadata;
struct requisit;
struct expression;
struct statement;

extern metadata void_meta[];
extern metadata int_meta[];
extern metadata sint_meta[];
extern metadata usint_meta[];
extern metadata char_meta[];
extern metadata text_meta[];
extern metadata proc_meta[];
extern metadata section_meta[];

metadata*				addtype(const char* id);
metadata*				findtype(const char* id);

enum operator_s : unsigned char {
	Determinal,
	Unary, Binary, Statement,
};
enum expression_s {
	End,
	Number, Text, Requisit, Metadata,
	Add, Sub, Mul, Div, Mod,
	And, Or,
	Equal, NotEqual, Lesser, LesserEqual, Greater, GreaterEqual,
	Coma, Call,
	Var, List, While, If, For, Return,
};
enum token_s {
	NoToken,
	Whitespace, Keyword, OpenTag, CloseTag,
	NumberToken, TextToken, RequisitToken, MetadataToken,
};
struct statement {
	expression*				condition;
	expression*				body;
	constexpr statement() : condition(), body() {}
	void* operator			new(unsigned size);
	void operator			delete(void* p, unsigned size);
	void					add(stringcreator& sc);
};
struct expression {
	expression_s			type;
	union {
		struct {
			expression*		op1;
			expression*		op2;
		};
		struct {
			statement*		extended;
			expression*		next;
		};
		int					value;
		const char*			text;
		requisit*			req;
		metadata*			met;
	};
	constexpr expression() : type(End), op1(0), op2(0) {}
	constexpr expression(expression_s type) : type(type), op1(0), op2(0) {}
	constexpr expression(int v) : type(Number), value(v) {}
	constexpr expression(const char* v) : type(Text), text(v) {}
	constexpr expression(requisit* v) : type(Requisit), req(v) {}
	constexpr expression(metadata* v) : type(Metadata), met(v) {}
	constexpr expression(expression_s type, expression* e1) : type(type), op1(e1), op2(0) {}
	constexpr expression(expression_s type, expression* e1, expression* e2) : type(type), op1(e1), op2(e2) {}
	void* operator			new(unsigned size);
	void operator			delete(void* p, unsigned size);
	struct builder {
		virtual void		add(token_s id, const expression* context, const char* v) = 0;
		virtual void		addline() = 0;
		void				add(const expression* context, int v);
	};
	void					add(expression* v);
	void					add(builder& b) const;
	operator_s				getoperands() const;
	void					zero();
private:
	void					addsingle(expression::builder& b) const;
};
struct metadata {
	const char*				id;
	metadata*				type;
	unsigned				size;
	constexpr operator bool() const { return id != 0; }
	requisit*				add(const char* id, metadata* type);
	metadata*				dereference();
	bool					ispointer() const { return id[0] == '*' && id[1] == 0; }
	requisit*				find(const char* id) const;
	metadata*				reference();
};
struct requisit {
	const char*				id;
	metadata*				parent;
	metadata*				type;
	unsigned				offset;
	unsigned				count;
	expression*				code;
	constexpr operator bool() const { return id != 0; }
	constexpr unsigned		getsize() const { return type ? type->size : 0; }
	constexpr unsigned		getsizeof() const { return getsize() * count; }
	requisit*				setcount(int v) { if(this) count = v; return this; }
	requisit*				set(expression* v) { if(this) code = v; return this; }
};
void					logmsg(const char* format, ...);
extern adat<requisit, 256 * 16>	requisit_data;
extern adat<metadata, 256 * 4>	metadata_data;
}
void						choose_metadata(code::metadata* v);
void						run_main();