#include "xface/collection.h"
#include "xface/stringcreator.h"
#include "xface/valuelist.h"

const char*				szdup(const char *text);

#pragma once

namespace code {

struct metadata;
struct requisit;
struct expression;
struct statement;

enum operator_s : unsigned char {
	Determinal,
	Unary, Binary, Statement,
};
enum expression_s : unsigned char {
	DoNothing,
	Number, Text, Requisit, Metadata,
	Add, Sub, Mul, Div, Mod,
	And, Or,
	Equal, NotEqual, Lesser, LesserEqual, Greater, GreaterEqual,
	Coma, Call,
	Var, List, While, If, For, Return,
};
enum token_s : unsigned char {
	NoToken,
	Whitespace, Keyword, OpenTag, CloseTag,
	NumberToken, TextToken, RequisitToken, MetadataToken,
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
	constexpr expression() : type(DoNothing), op1(0), op2(0) {}
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
		virtual void		begin() = 0;
		virtual void		end() = 0;
	};
	void					add(expression* v);
	void					add(builder& b) const;
	void					clear();
	operator_s				getoperands() const;
	void					select(valuelist& v) const;
	void					select(valuelist& v, expression_s t) const;
	void					set(expression_s v);
	void					zero();
private:
	void					addsingle(expression::builder& b) const;
};
struct statement {
	expression				condition;
	expression				body;
	constexpr statement() : condition(1), body() {}
	void* operator			new(unsigned size);
	void operator			delete(void* p, unsigned size);
};
struct requisit {
	const char*				id;
	metadata*				type;
	unsigned				offset;
	unsigned				count;
	expression*				code;
	constexpr requisit() : id(0), type(0), offset(0), count(0), code(0) {}
	constexpr requisit(const char* id, metadata* type) : id(id), type(type), offset(0), count(1), code(0) {}
	constexpr requisit(const char* id, metadata* type, unsigned count) : id(id), type(type), offset(0), count(count), code(0) {}
	constexpr operator bool() const { return id != 0; }
	unsigned				getsize() const;
	unsigned				getsizeof() const { return getsize() * count; }
	void*					ptr(void* object) const { return (char*)object + offset; }
	void*					ptr(void* object, unsigned index) const { return (char*)object + offset + index * getsize(); }
	requisit*				setcount(int v) { if(this) count = v; return this; }
	requisit*				set(expression* v) { if(this) code = v; return this; }
};
struct requisitc : arem<requisit> {
	constexpr requisitc() : arem() {}
	template<unsigned N> constexpr requisitc(requisit(&data)[N]) : arem(data, N) {}
	const requisit*			find(const char* id) const;
};
struct metadata {
	const char*				id;
	metadata*				type;
	unsigned				size;
	requisitc				requisits;
	constexpr operator bool() const { return id != 0; }
	requisit*				add(const char* id, metadata* type);
	void					addto(arem<metadata*>& source) const;
	static void				initialize();
	bool					is(const char* id) const;
	bool					isarray() const { return id[0] == '[' && id[1] == ']' && id[2] == 0; }
	bool					ismeta() const;
	bool					isnumber() const;
	bool					isreference() const { return id[0] == '*' && id[1] == 0; }
	bool					ispredefined() const;
	bool					istext() const;
	requisit*				find(const char* id) const { return const_cast<requisit*>(requisits.find(id)); }
	const requisit*			getid() const;
	void					update();
	void					write(const char* url) const;
	static void				write(const char* url, arem<metadata*>& types);
};
struct metadatac : agrw<metadata, 64> {
	metadata*				add(const char* id);
	metadata*				add(const char* id, metadata* type, unsigned size);
	metadata*				array(metadata* type);
	metadata*				dereference(metadata* type);
	metadata*				find(const char* id) const;
	metadata*				find(const char* id, const metadata*) const;
	metadata*				reference(metadata* type);
};
struct configi {
	metadatac				types;
	static metadata*		standart[];
};
extern configi				config;
void						logmsg(const char* format, ...);
}
void						choose_metadata(code::metadata* v);
void						run_main();