#include "package.h"
#include "crt.h"

using namespace code;

struct parsestate {
	const char*				p;
	struct symbol*			module;
	struct symbol*			member;
	parsestate();
	~parsestate();
};

static parsestate			ps;
static int					errors;

static void status(msg m, ...) {
	if(m < msg::Options) {
		errors++;
		ps.p = "";
	} else {
	}
}

static void declare_status(msg m, const symbol* member, ...) {
}

static unsigned binary_operation(unsigned t1, unsigned t2) {
	return 0;
}

static void error_sym(const char* name, char sym) {
}

static bool skipcm() {
	if(ps.p[0] == '/' && ps.p[1] == '/') {
		// Line comment
		ps.p += 2;
		while(*ps.p && *ps.p != '\n' && *ps.p != '\r')
			ps.p++;
		return true;
	} else if(ps.p[0] == '/' && ps.p[1] == '*') {
		// Comment
		ps.p += 2;
		while(*ps.p && !(ps.p[0] == '*' && ps.p[1] == '/'))
			ps.p++;
		return true;
	}
	return false;
}

static void next(const char* p1) {
	ps.p = p1;
	while(true) {
		// 1 - skip all white spaces
		ps.p = skipspcr(ps.p);
		// 2 - skip all comments
		if(skipcm())
			continue;
		break;
	}
}

static void skip(char sym) {
	if(*ps.p == sym)
		next(ps.p + 1);
	else
		error_sym("expected symbol \'%1\'", sym);
}

static bool ischab(char sym) {
	return (sym >= 'A' && sym <= 'Z')
		|| (sym >= 'a' && sym <= 'z')
		|| (sym == '_');
}

static bool ischa(char sym) {
	return (sym >= '0' && sym <= '9')
		|| (sym >= 'A' && sym <= 'Z')
		|| (sym >= 'a' && sym <= 'z')
		|| (sym == '_');
}

static bool match(char sym) {
	if(*ps.p == sym) {
		next(ps.p + 1);
		return true;
	}
	return false;
}

static bool match(const char* name) {
	auto p1 = ps.p;
	auto p2 = name;
	while(name[0] && *p1) {
		if(*name++ != *p1++)
			return false;
	}
	if(name[0] || ischa(*p1))
		return false;
	next(p1);
	status(Keyword, (int)p2);
	return true;
}

static const char* identifier() {
	if(!ischab(*ps.p))
		return 0;
	static char temp[256];
	auto s = temp;
	while(ischa(*ps.p))
		*s++ = *ps.p++;
	*s++ = 0;
	next(ps.p);
	return temp;
}

static void expression(evalue& e1) {
	logical_or(e1);
	while(*ps.p == '?') {
		next(ps.p + 1);
		int i = jumpforward();
		expression(e1);
		skip(':');
		int f = testcondition(1);
		label(i);
		evalue e2;
		expression(e1);
		label(f);
	}
}

static void expression() {
	evalue e1;
	expression(e1);
}

static void expression_nocode() {
	genstate push;
	gen.code = false;
	evalue e1;
	expression(e1);
}

static int expression_const() {
	genstate push;
	gen.code = false;
	evalue e1;
	expression(e1);
	if(!e1.isconst())
		status(ErrorNeedConstantExpression);
	return e1.offset;
}

static type* expression_const_type() {
	genstate push;
	gen.code = false;
	evalue e1;
	expression(e1);
	if(!e1.isconst() || !e1.lvalue)
		status(ErrorNeedConstantExpression);
	auto result = e1.lvalue;
	if(!result || !result->istype())
		result = e1.result;
	return result;
}

static type* parse_pointer(type* declared) {
	while(*ps.p == '*') {
		next(ps.p + 1);
		declared = declared->reference();
	}
	return declared;
}

static bool istype(symbol** declared, unsigned& flags) {
	type* e = type::i32;
	bool result = false;
	int m_public = 0;
	int m_static = 0;
	int m_unsigned = 0;
	while(*ps.p) {
		if(match("static")) {
			result = true;
			m_static++;
			continue;
		} else if(match("public")) {
			result = true;
			m_public++;
			continue;
		} else if(match("unsigned")) {
			result = true;
			m_unsigned++;
			continue;
		} else if(ischab(*ps.p)) {
			const char* p1 = ps.p;
			auto id = szdup(identifier());
			// Теперь надо найти тип, и убедиться, что это действительно определение
			bool change_back = true;
			e = ps.module->findmembertype(id, m_unsigned);
			if(e) {
				if(*ps.p == '*' || ischa(*ps.p)) {
					change_back = false;
					result = true;
				}
			}
			if(change_back)
				ps.p = p1;
		}
		if(result) {
			if(m_static > 1)
				status(Error1pDontUse2pTimes, "static", m_static);
			if(m_public > 1)
				status(Error1pDontUse2pTimes, "public", m_public);
			if(m_unsigned > 1)
				status(Error1pDontUse2pTimes, "unsigned", m_unsigned);
			if(m_static)
				flags |= 1 << Static;
			if(m_public)
				flags |= 1 << Private;
			*declared = e;
		}
		break;
	}
	return result;
}

static void initialize(type* sym) {
	if(sym->count && *ps.p == '{') {
		next(ps.p + 1);
		if(sym->count == -1) {
			sym->count = 0;
			while(true) {
				sym->count++;
				initialize(sym->result);
				if(*ps.p == '}')
					break;
				skip(',');
				if(*ps.p == '}')
					break;
			}
		} else {
			int i = 0;
			while(true) {
				if(i++ >= sym->count) {
					genstate push;
					gen.code = false;
					initialize(sym->result);
				} else
					initialize(sym->result);
				if(*ps.p == '}')
					break;
				skip(',');
				if(*ps.p == '}')
					break;
			}
			if(i >= sym->count)
				status(ErrorArrayOverflow, i, sym->count);
		}
		skip('}');
	} else if(sym->child) {
		skip('{');
		auto t = sym->child;
		while(t) {
			if(t->ismember() && !t->ismethod()) {
				initialize(t);
				if(*ps.p == '}')
					break;
				skip(',');
				if(*ps.p == '}')
					break;
			}
			t = t->next;
		}
		while(t) {
			if(t->ismember() && !t->ismethod()) {
				//
			}
			t = t->next;
		}
		skip(',');
	} else if(sym->result)
		initialize(sym->result);
	else {
		// simple type
		int s = sym->size;
		int v = expression_const();
		if(sym->ispointer()) {
			//gen::add32(v, Static);
		} else {
			while(s > 0) {
				segments[Data]->add((unsigned char)(v & 255));
				v >>= 8;
				s--;
			}
		}
	}
}

static void instance(type* variable) {
	if(*ps.p == '=') {
		next(ps.p + 1);
		variable->value = segments[Data]->get();
		initialize(variable);
	} else {
		//variable->set(NoInitialized);
		//gen::addz(variable->size, Variable);
	}
	variable->size = variable->result->size * variable->count;
	//// PARAM
	//variable->value = proc::param;
	//proc::param += (variable->size + 3) & 0xFFFFFFFC;
	//// AUTO
	//proc::local += (variable->size + 3) & 0xFFFFFFFC;
	//variable->value = -proc::local;
	//variable->size = variable->csize();
	//if(analize_code && *p == '=')
	//{
	//	gen::push(variable);
	//	next(p + 1);
	//	assigment();
	//	gen::oper('=');
	//	gen::pop();
	//}
	//if(!disable::size)
	//{
	//	// Члены не статического модуля имеют смещение
	//	// максимального размера своего родителя и изменяют его размер
	//	// на свой размер.
	//	variable->value = variable->parent->size;
	//	variable->parent->size += variable->size;
	//}
}

// Чтобы можно было в секции данных размещать ресурсы и подтягивать их под переменные.
static bool declaration(type* parent, unsigned flags, bool allow_functions = true, bool allow_variables = true) {
	type* declared;
	const char* p1 = ps.p;
	if(!istype(&declared, flags))
		return false;
	while(*ps.p) {
		c2::type* result = parse_pointer(declared);
		auto id = szdup(identifier());
		if(*ps.p == '(' && !allow_functions) {
			ps.p = p1;
			return false;
		} else if(*ps.p != '(' && !allow_variables) {
			ps.p = p1;
			return false;
		}
		if(gen.unique) {
			// Test unique members
			for(auto pm = parent->child; pm; pm = pm->next) {
				if(pm->isforward())
					continue;
				if(pm->id == id) {
					status(Error1p2pAlreadyDefined, "identifier", id);
					break;
				}
			}
		}
		type* m2 = parent->create(id, result, flags);
		declare_status(StatusDeclare, m2);
		if(*ps.p == '(') {
			// Если объявляется функция она становится
			// новым контекстом
			ps.member = m2;
			next(ps.p + 1);
			m2->setmethod();
			m2->count = 0;
			while(*ps.p) {
				type* result;
				unsigned pflags = 0;
				if(istype(&result, pflags)) {
					result = parse_pointer(result);
					auto id = szdup(identifier());
					result = m2->create(id, result, pflags);
					m2->count++;
					//gen::instance(result, false);
				}
				if(*ps.p == ')') {
					next(ps.p + 1);
					break;
				}
				skip(',');
			}
			if(*ps.p == ';') {
				// Forward declaration
				next(ps.p + 1);
			} else {
				m2->content = ps.p;
				prologue(m2);
				statement(0, 0, 0, 0);
				epilogue(m2);
				//gen::result(m2);
			}
			return true;
		} else if(*ps.p == '[') {
			next(ps.p + 1);
			if(*ps.p == ']') {
				next(ps.p + 1);
				m2->count = -1;
				if(*ps.p != '=')
					m2->count = 0;
			} else {
				m2->count = expression_const();
				skip(']');
			}
		}
		instance(m2);
		if(*ps.p == ';') {
			next(ps.p + 1);
			break;
		}
		skip(',');
	}
	return true;
}

static bool direct_cast(evalue& e1) {
	type* declared;
	unsigned flags = 0;
	const char* p1 = ps.p;
	if(!istype(&declared, flags))
		return false;
	declared = parse_pointer(declared);
	if(*ps.p != ')') {
		ps.p = p1;
		return false;
	}
	next(ps.p + 1);
	e1.result = declared;
	return true;
}

static char next_string_symbol() {
	while(*ps.p) {
		if(*ps.p != '\\')
			return *ps.p++;
		ps.p++;
		switch(*ps.p++) {
		case 0:
			return 0;
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 'r':
			return '\r';
		case '\\':
			return '\\';
		case '\'':
			return '\'';
		case '\"':
			return '\"';
		case '\n':
			if(ps.p[0] == '\r')
				ps.p++;
			break;
		case '\r':
			if(ps.p[0] == '\n')
				ps.p++;
			break;
		default:
		{
			char temp[2] = {ps.p[-1], 0};
			status(ErrorInvalidEscapeSequence, temp);
		}
		break;
		}
	}
	return 0;
}

static int next_char() {
	char result[5];
	char* d1 = result;
	char* d2 = result + sizeof(result) / sizeof(result[0]) - 1;
	memset(result, 0, sizeof(result));
	while(*ps.p) {
		if(*ps.p == '\'') {
			next(ps.p + 1);
			break;
		}
		if(d1 < d2)
			*d1++ = next_string_symbol();
	}
	*d1++ = 0;
	return *((int*)&result);
}

static int next_string() {
	int result = segments[DataStrings]->get();
	while(*ps.p) {
		if(*ps.p == '\"') {
			next(ps.p + 1);
			if(*ps.p == '\"') {
				// if two string in row
				next(ps.p + 1);
				continue;
			} else if(*ps.p == '+' && ps.p[1] != '+') {
				// checking '+' between two strings
				const char* p1 = ps.p;
				next(ps.p + 1);
				if(*ps.p == '\"') {
					next(ps.p + 1);
					continue;
				}
				ps.p = p1;
			}
			break;
		}
		segments[DataStrings]->add((unsigned char)next_string_symbol());
	}
	segments[DataStrings]->add(0);
	return result;
}

static int next_number() {
	int num = 0;
	if(ps.p[0] == '0') {
		if(ps.p[1] == 'x') {
			ps.p += 2;
			while(true) {
				char s = *ps.p;
				if(s >= 'a' && s <= 'f')
					s = s - 'a' + 10;
				else if(s >= 'A' && s <= 'F')
					s = s - 'A' + 10;
				else if(s >= '0' && s <= '9')
					s = s - '0';
				else
					break;
				num = num * 16 + s;
				ps.p++;
			}
		} else {
			while(*ps.p >= '0' && *ps.p <= '7') {
				num = num * 8 + *ps.p - '0';
				ps.p++;
			}
		}
	} else {
		while(*ps.p >= '0' && *ps.p <= '9') {
			num = num * 10 + *ps.p - '0';
			ps.p++;
		}
	}
	if(*ps.p == 'f' || *ps.p == 'e')
		ps.p++;
	next(ps.p);
	return num;
}

static void function_call(evalue& e1) {
	auto sym = e1.lvalue;
	if(!sym->ismethod())
		status(ErrorNeedFunctionMember);
	skip('(');
	int	count = 0;
	evalue parameters[96];
	while(*ps.p) {
		if(*ps.p == ')') {
			next(ps.p + 1);
			break;
		}
		parameters[count].clear();
		parameters[count].next = (count ? &parameters[count - 1] : &e1);
		expression(parameters[count]);
		count++;
		if(*ps.p == ')') {
			next(ps.p + 1);
			break;
		}
		skip(',');
	}
	// default parameters
	// parameters back order
	for(int i = 0; i < count; i++) {
		//gen::param();
		//gen::pop();
	}
	if(sym->getparametercount() != count)
		status(ErrorWrongParamNumber, sym->id, sym->getparametercount(), count);
	calling(sym, parameters, count);
	// function return value
	e1.lvalue = 0;
	e1.offset = 0;
	e1.reg = Eax;
}

static type* forward_declare(type* sym, type* parent, const char* id) {
	if(sym || !id || !parent)
		return sym;
	if(*ps.p != '(') {
		// Не найдена переменная
		status(ErrorCantFind1pWithName2p, "variable", id);
		return 0;
	}
	sym = parent->create(id, type::i32, 0);
	sym->setmethod();
	return sym;
}

static void postfix(evalue& e1) {
	while(*ps.p) {
		if(*ps.p == '(')
			function_call(e1);
		else if(*ps.p == '.') {
			next(ps.p + 1);
			const char* n = szdup(identifier());
			auto sym = forward_declare(e1.result->findmembers(n), e1.result, n);
			evalue e2(&e1); e2.set(sym);
			binary_operation(e2, '.');
		} else if(*ps.p == '[') {
			next(ps.p + 1);
			if(!e1.result->ispointer())
				status(ErrorNeedPointerOrArray);
			//gen::rvalue(0);
			evalue e2(&e1);
			expression(e2);
			skip(']');
			binary_operation(e2, '+');
			unary_operation(e2, '*');
		} else if(ps.p[0] == '-' && ps.p[1] == '-') {
			next(ps.p + 2);
			if(!e1.islvalue())
				status(ErrorNeedLValue);
			//gen::dup();
			//gen::rvalue(-1);
			evalue e2(&e1); e2.set(1);
			binary_operation(e2, '=', '-');
			//gen::pop();
		} else if(ps.p[0] == '+' && ps.p[1] == '+') {
			next(ps.p + 2);
			if(!e1.islvalue())
				status(ErrorNeedLValue);
			//gen::dup();
			//gen::rvalue(-1);
			evalue e2(&e1); e2.set(1);
			binary_operation(e2, '=', '+');
			//gen::pop();
		} else
			break;
	}
}

static void unary(evalue& e1) {
	switch(ps.p[0]) {
	case '-':
		ps.p++;
		if(ps.p[0] == '-') {
			next(ps.p + 1);
			unary(e1);
			evalue e2(&e1); e2.set(1);
			binary_operation(e2, '=', '-');
		} else {
			next(ps.p + 1);
			unary(e1);
			unary_operation(e1, '-');
		}
		break;
	case '+':
		ps.p++;
		if(ps.p[0] == '+') {
			next(ps.p + 1);
			unary(e1);
			evalue e2(&e1); e2.set(1);
			binary_operation(e2, '=', '+');
		} else {
			next(ps.p + 1);
			unary(e1);
		}
		break;
	case '!':
		next(ps.p + 1);
		unary(e1);
		unary_operation(e1, '!');
		break;
	case '*':
		next(ps.p + 1);
		unary(e1);
		unary_operation(e1, '*');
		break;
	case '&':
		next(ps.p + 1);
		unary(e1);
		unary_operation(e1, '&');
		break;
	case '(':
		if(!direct_cast(e1)) {
			next(ps.p + 1);
			expression(e1);
			skip(')');
		}
		break;
	case '\"':
		ps.p++;
		e1.set(next_string());
		e1.result = type::i8[0].reference();
		break;
	case '\'':
		ps.p++;
		e1.set(next_string());
		e1.result = type::i8;
		break;
	case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
		e1.set(next_number());
		break;
	default:
		if(match("sizeof")) {
			skip('(');
			c2::type* sym = expression_const_type();
			e1.set(sym->size);
			skip(')');
		} else if(match("this"))
			e1.set(ps.module);
		else if(match("true"))
			e1.set(1);
		else if(match("false"))
			e1.set(0);
		else if(ischa(*ps.p)) {
			const char* n = szdup(identifier());
			c2::type* sym = 0;
			if(!sym && ps.member)
				sym = ps.member->findmembers(n);
			if(!sym && ps.module)
				sym = ps.module->findmembers(n);
			if(!sym && ps.module)
				sym = ps.module->findmembertype(n);
			if(!sym)
				status(ErrorNotFound1p2p, "identifier", n);
			e1.set(sym);
		}
		break;
	}
	postfix(e1);
}

static void multiplication(evalue& e1) {
	unary(e1);
	while((ps.p[0] == '*' || ps.p[0] == '/' || ps.p[0] == '%') && ps.p[1] != '=') {
		char s = ps.p[0];
		next(ps.p + 1);
		evalue e2(&e1);
		unary(e2);
		//gen::cast();
		binary_operation(e2, s);
	}
}

static void addiction(evalue& e1) {
	multiplication(e1);
	while((ps.p[0] == '+' || ps.p[0] == '-') && ps.p[1] != '=') {
		char s = ps.p[0];
		next(ps.p + 1);
		evalue e2(&e1);
		multiplication(e2);
		//gen::cast();
		binary_operation(e2, s);
	}
}

static void binary_cond(evalue& e1) {
	addiction(e1);
	while((ps.p[0] == '>' && ps.p[1] != '>')
		|| (ps.p[0] == '<' && ps.p[1] != '<')
		|| (ps.p[0] == '=' && ps.p[1] == '=')
		|| (ps.p[0] == '!' && ps.p[1] == '=')) {
		char t1 = *ps.p++;
		char t2 = 0;
		if(ps.p[0] == '=')
			t2 = *ps.p++;
		next(ps.p);
		evalue e2(&e1);
		addiction(e2);
		//gen::cast();
		binary_operation(e2, t1, t2);
	}
}

static void binary_and(evalue& e1) {
	binary_cond(e1);
	while(ps.p[0] == '&' && ps.p[1] != '&') {
		next(ps.p + 2);
		evalue e2(&e1);
		binary_cond(e2);
		//gen::cast();
		binary_operation(e2, '&');
	}
}

static void binary_xor(evalue& e1) {
	binary_and(e1);
	while(ps.p[0] == '^') {
		next(ps.p + 1);
		evalue e2(&e1);
		binary_and(e2);
		//gen::cast();
		binary_operation(e2, '^');
	}
}

static void binary_or(evalue& e1) {
	binary_xor(e1);
	while(ps.p[0] == '|' && ps.p[1] != '|') {
		next(ps.p + 1);
		evalue e2(&e1);
		binary_or(e2);
		//gen::cast();
		binary_operation(e2, '|');
	}
}

static void binary_shift(evalue& e1) {
	binary_or(e1);
	while((ps.p[0] == '>' && ps.p[1] == '>') || (ps.p[0] == '<' && ps.p[1] == '<')) {
		char t1 = ps.p[0];
		char t2 = ps.p[1];
		next(ps.p + 2);
		evalue e2(&e1);
		binary_or(e2);
		//gen::cast();
		binary_operation(e2, t1, t2);
	}
}

static void logical_and(evalue& e1) {
	binary_shift(e1);
	int t = 0;
	bool was = false;
	while(ps.p[0] == '&' && ps.p[1] == '&') {
		next(ps.p + 2);
		t = testcondition(1, t);
		evalue e2(&e1);
		binary_shift(e2);
		was = true;
	}
	//if(was)
	//	gen::pushl(FJmpI, t);
}

static void logical_or(evalue& e1) {
	logical_and(e1);
	int t = 0;
	bool was = false;
	while(ps.p[0] == '|' && ps.p[1] == '|') {
		next(ps.p + 2);
		t = testcondition(0, t);
		evalue e2(&e1);
		logical_and(e2);
		was = true;
	}
	//if(was)
	//	gen::pushl(FJmp, t);
}

static void assigment(evalue& e1) {
	expression(e1);
	if((ps.p[0] == '=' && ps.p[1] != '=')
		|| ((ps.p[0] == '+' || ps.p[0] == '-' || ps.p[0] == '/' || ps.p[0] == '*') && ps.p[1] == '=')) {
		char t2 = *ps.p++;
		char t1 = '=';
		if(t2 == '=')
			t2 = 0;
		next(ps.p + 1);
		evalue e2(&e1);
		assigment(e2);
		binary_operation(e2, t1, t2);
	}
}

static void assigment() {
	evalue e1;
	assigment(e1);
}

static void skip_statement(int* ct, int* br, int* cs, int* ds, evalue* cse = 0) {
	genstate push;
	gen.code = false;
	statement(ct, br, cs, ds, cse);
}

static void statement(int* ct, int* br, int* cs, int* ds, evalue* cse) {
	if(match(';')) {
		// Empthy statement
	} else if(match('{')) {
		// Сделаем локальный стэк
		int count = locals.count;
		//int local = proc::local;
		while(*ps.p) {
			if(*ps.p == '}') {
				next(ps.p + 1);
				break;
			}
			auto psp = ps.p;
			statement(ct, br, cs, ds);
			if(psp == ps.p) {
				status(ErrorExpected1p, "keyword");
				break;
			}
		}
		// Очистим локальный стэк переменных
		for(int i = locals.count; i > count; i--) {
			// empty data can be from saved registers in stack
			// just test this and skip this case
			if(locals.data[i - 1]) {
			}
		}
		//if(proc::localmax < proc::local)
		//	proc::localmax = proc::local;
		locals.count = count;
		//proc::local = local;
	} else if(match("break")) {
		if(!br)
			status(ErrorKeyword1pUsedWithout2p, "break", "loop");
		skip(';');
		if(br)
			*br = jumpforward(*br);
	} else if(match("continue")) {
		if(!ct)
			status(ErrorKeyword1pUsedWithout2p, "continue", "loop");
		skip(';');
		if(ct)
			jumpback(*ct);
	} else if(match("return")) {
		//c2::evalue* e1 = top;
		expression();
		//if(ps.member->result == type::v0 && (top - e1))
		//	status(ErrorVoidReturnValue);
		//if(top)
		//{
			//gen::setresult();
			//gen::pop();
		//}
		//gen::result(ps.member);
		skip(';');
	} else if(match("if")) {
		int e = 0;
		while(true) {
			evalue e1;
			skip('(');
			expression(e1);
			skip(')');
			int b = testcondition(1);
			statement(ct, br, cs, ds);
			if(!match("else")) {
				label(b);
				break;
			}
			e = jumpforward(e);
			label(b);
			if(match("if"))
				continue;
			// Else statement - if anything not match
			statement(ct, br, cs, ds);
			break;
		}
		label(e);
	} else if(match("while")) {
		int label_continue = label();
		skip('(');
		expression();
		skip(')');
		int lable_break = testcondition(1);
		statement(&label_continue, &lable_break, 0, 0);
		jumpback(label_continue);
		label(lable_break);
	} else if(match("do")) {
		int label_continue = label();
		int label_break = 0;
		statement(&label_continue, &label_break, 0, 0);
		if(!match("while"))
			status(ErrorExpected1p, "while");
		skip('(');
		expression();
		skip(')');
		testcondition(0, label_continue);
		label(label_break);
	} else if(match("for")) {
		skip('(');
		// Инициализация цикла
		if(!declaration(ps.member, 0, false)) {
			evalue e1;
			assigment(e1);
		}
		if(*ps.p == ';')
			skip(';');
		// Генерируем метку продолжения и проверку условия цикла
		int label_continue = label();
		int label_break = 0;
		expression();
		label_break = testcondition(1, label_break);
		skip(';');
		// Пропустим блок инкремента
		const char* p_step = ps.p;
		expression_nocode();
		skip(')');
		// Генерируем блок тела цикла
		statement(&label_continue, &label_break, 0, 0);
		// Генерируем блок инкремента и переход на проверку условия
		const char* p_next = ps.p;
		next(p_step);
		expression();
		next(p_next);
		jumpback(label_continue);
		// Генерируем метку выхода из цикла
		label(label_break);
	} else if(match("switch")) {
		evalue e1;
		skip('(');
		expression(e1);
		skip(')');
		int break_label = 0;
		int case_label = 0;
		int default_label = 0;
		e1.load();
		statement(ct, &break_label, &case_label, &default_label, &e1);
		label(case_label);
		if(default_label)
			jumpback(default_label);
		label(break_label);
	} else if(match("case")) {
		//gen::dup();
		int v1 = expression_const();
		int v2 = v1;
		skip(':');
		if(cs)
			label(*cs);
		else
			status(ErrorKeyword1pUsedWithout2p, "case", "switch");
		int csm = 0;
		if(v1 == v2) {
			evalue e1(cse); e1.set(v1);
			binary_operation(e1, '=', '=');
			csm = testcondition(0, csm);
		}
		if(cs)
			*cs = csm;
	} else if(match("default")) {
		skip(':');
		if(ds)
			*ds = label();
		else
			status(ErrorKeyword1pUsedWithout2p, "default", "switch");
	} else if(!declaration(ps.member, 0)) {
		assigment();
		skip(';');
	}
}

static void block_declaration() {
	while(declaration(ps.module, 1 << Static, false, true));
}

static void block_function() {
	while(declaration(ps.module, 0, true, false));
}

static void block_enums() {
	while(match("enum")) {
		identifier();
		skip('{');
		int num = 0;
		c2::type* result = type::i32;
		c2::type* t = 0;
		while(*ps.p) {
			if(ischab(*ps.p)) {
				t = ps.module->create(identifier(), result, 0);
				t->setconstant(num++);
			} else if(*ps.p == '=') {
				next(ps.p + 1);
				num = expression_const();
				if(t)
					t->setconstant(num);
				else
					status(ErrorAssigmentWithoutEnumeratorMember);
			} else if(*ps.p == ',') {
				t = 0;
				next(ps.p + 1);
			} else if(*ps.p == '}') {
				next(ps.p + 1);
				break;
			} else {
				status(ErrorExpectedEnumeratorMember);
				return;
			}
		}
		skip(';');
	}
}

static void block_imports() {
	// Для того чтобы не есть память стэка
	// сделаем данную переменную статической
	static char temp[260];
	int count = 0;
	while(match("import")) {
		temp[0] = 0;
		typeref e = {0};
		while(ps.p[0]) {
			auto pz = identifier();
			if(pz)
				zcat(temp, pz);
			if(*ps.p == '.') {
				zcat(temp, '.');
				next(ps.p + 1);
				continue;
			}
			if(match("as"))
				pz = identifier();
			auto id = szdup(temp);
			e.id = szdup(pz);
			e.type = type::findtype(id);
			if(!e.type) {
				parsestate push;
				e.type = type::create(id);
				e.type->parse();
			}
			// Проверим а был ли модуль импортирован ранее в блоке импорта
			int level = 0;
			for(auto p = ps.module->refs; p; p = p->next) {
				if(level++ >= count)
					break;
				if(p->type == e.type)
					status(ErrorModuleAlreadyImported, e.type->id);
			}
			// Добавим ссылку импортированного модуля в текущий если ее еще нет
			if(!ps.module->refs->find(e.type)) {
				auto pr = typeref::create(e.id, e.type);
				if(ps.module->refs)
					seqlast(ps.module->refs)->next = pr;
				else
					ps.module->refs = pr;
			}
			count++;
			skip(';');
			break;
		}
	}
}

static void block_start(type* module) {
	ps.p = getfile(module->id);
	if(!ps.p) {
		status(ErrorCantFind1pWithName2p, "import module", module->id);
		return;
	}
	ps.module = module;
	ps.member = 0;
	next(ps.p);
}

void type::parse() {
	block_start(this);
	if(errors)
		return;
	block_imports();
	if(errors)
		return;
	status(StatusStartParse);
	block_enums();
	if(errors)
		return;
	block_declaration();
	if(errors)
		return;
	block_function();
	if(errors)
		return;
	if(*ps.p && !errors)
		status(ErrorUnexpectedSymbols);
}

bool type::setbackend(const char* progid) {
	compiler_backend = backend::find(progid);
	return compiler_backend != 0;
}

static void compile_member(type* member) {
	ps.member = member;
	ps.module = member->parent;
	if(ps.member) {
		ps.p = ps.member->content;
		if(ps.p) {
			prologue(ps.member);
			statement(0, 0, 0, 0);
			epilogue(ps.member);
		}
	}
}

type* type::compile(const char* id, const char* start) {
	auto p = findtype(id);
	if(!p)
		p = create(id);
	genstate push;
	gen.code = false;
	gen.unique = true;
	p->parse();
	if(start) {
		gen.code = true;
		gen.unique = false;
		compile_member(p->findmembers(start));
	}
	return p;
}