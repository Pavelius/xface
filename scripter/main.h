#include "agrw.h"
#include "crt.h"
#include "point.h"
#include "stringbuilder.h"
#include "valuelist.h"
#include "markup.h"

#pragma once

#define APLNK(e,t) DGLNK(e,t) BSLNK(e,t)

namespace code {
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
enum metatype_s : unsigned char {
	Static, Public, Dimension, ScalarType, Method
};
struct metadata;
struct requisit;
struct expression;
struct statement;
typedef cflags<metatype_s> metatypea;
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
struct metadata {
	const char*				id;
	metadata*				type;
	unsigned				count;
	unsigned				size;
	metatypea				flags;
	operator bool() const { return id != 0; }
	void					add(stringbuilder& sb, char sep = 0) const;
	requisit*				add(const char* id, const metadata* type);
	requisit*				add(array* p);
	requisit*				addm(const char* id, const metadata* type);
	bool					is(const char* id) const;
	constexpr bool			is(metatype_s v) const { return flags.is(v); }
	bool					isarray() const;
	static bool				isbasetype(const void* object);
	bool					isnumber() const { return flags.is(ScalarType); }
	bool					isreference() const;
	bool					ispredefined() const;
	bool					istext() const;
	requisit*				find(const char* id) const;
	static const metadata*	find(const metadata& e);
	int						getid() const;
	array*					getelements() const;
	void					getname(stringbuilder& sb) const;
	const metadata*			gettype() const;
	static void				read(const char* url);
	metadata*				records() const;
	metadata*				reference() const;
	static const metadata*	type_void;
	static const metadata*	type_i8;
	static const metadata*	type_u8;
	static const metadata*	type_i16;
	static const metadata*	type_u16;
	static const metadata*	type_i32;
	static const metadata*	type_u32;
	static const metadata*	type_sizet;
	static const metadata*	type_text;
	static const metadata*	type_metadata;
	static const metadata*	type_metadata_ptr;
	static const metadata*	type_metadata_array;
	static const metadata*	type_requisit;
	static const metadata*	type_import;
	static const metadata*	type_project;
	static const char*		classes_url;
	static const char*		projects_url;
	void					update();
	void					write(const char* url) const;
};
struct requisit {
	const char*				id;
	metadata*				type;
	unsigned				offset;
	unsigned				count;
	metadata*				parent;
	metatypea				flags;
	constexpr operator bool() const { return id != 0; }
	requisit*				add(metatype_s v) { flags.add(v); return this; }
	void					clear();
	bool					edit();
	constexpr bool			is(metatype_s v) const { return flags.is(v); }
	bool					ispredefined() const;
	unsigned				getlenght() const { return type->size*count; }
	void					getname(stringbuilder& sb) const;
	void					getnameonly(stringbuilder& sb) const;
	unsigned				getsize() const;
	unsigned				getsizeof() const { return getsize() * count; }
	void					geturl(stringbuilder& sb) const;
	constexpr void*			ptr(void* object) const { return (char*)object + offset; }
	void*					ptr(void* object, unsigned index) const { return (char*)object + offset + index * getsize(); }
	requisit*				setcount(int v) { if(this) count = v; return this; }
	requisit*				setoffset(unsigned v) { offset = v; return this; }
	requisit*				set(expression* v) {/* if(this) code = v;*/ return this; }
	requisit*				set(metatype_s v) { flags.add(v); return this; }
};
struct project {
	const char*				name;
	const char*				description;
	vector<const char*>		modules;
	static void				import();
	static project			main;
	void					write(const char* url);
};
struct importi {
	const metadata*			parent;
	const char*				url;
};
struct metatypei {
	const char*				id;
	const char*				name;
};
metadata*					addtype(const char* id);
metadata*					addtype(const char* id, const metadata* type, unsigned size, unsigned count = 1);
metadata*					findtype(const char* id);
void						initialize();
}
void						run_main();
void						logmsg(const char* format, ...);
APLNK(code::metatype_s, code::metatypei)