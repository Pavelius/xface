#include "main.h"

using namespace code;

struct expression_info {
	const char*			name;
	operator_s			operands;
	bool				extended;
};

expression_info expression_data[] = {{""},
{"Number"},
{"Text"},
{"Requisit"},
{"Metadata"},
{"+", Binary},
{"-", Binary},
{"*", Binary},
{"/", Binary},
{"%", Binary},
{"&&", Binary},
{"||", Binary},
{"==", Binary},
{"!=", Binary},
{"<", Binary},
{"<=", Binary},
{">", Binary},
{">=", Binary},
{",", Binary},
{"()", Binary},
{"Var", Statement},
{"List", Statement},
{"While", Statement, true},
{"If", Statement, true},
{"For", Statement, true},
{"Return", Statement},
};

static agrw<statement, 256 * 4 * 16> statement_base;
static agrw<expression, 256 * 4 * 16> expression_base;

void* statement::operator new(unsigned size) {
	return expression_base.add();
}

void statement::operator delete(void* p, unsigned size) {}

void* expression::operator new(unsigned size) {
	return expression_base.add();
}

void expression::operator delete(void* p, unsigned size) {}

operator_s expression::getoperands() const {
	return expression_data[type].operands;
}

expression* expression::getnext() const {
	if(expression_data[type].operands==Statement)
		return next;
	return 0;
}

void expression::add(expression* v) {
	if(v->getoperands()==Statement && getoperands() == Statement) {
		auto push = next;
		next = v;
		v->next = push;
	}
}

void expression::add(stringcreator& sc) {
	switch(expression_data[type].operands) {
	case Determinal:
		switch(type) {
		case End: break;
		case Number: sc.add("%1i", value); break;
		case Text: sc.add("\"%1\"", text); break;
		case Requisit: sc.add(req->id); break;
		case Metadata: sc.add(met->id); break;
		}
		break;
	case Binary:
		op1->add(sc);
		sc.add(expression_data[type].name);
		op2->add(sc);
		break;
	default:
		sc.add(expression_data[type].name);
		if(expression_data[type].operands==Statement)
			sc.add(" ");
		if(op1) {
			if(expression_data[type].extended) {

			} else
				op1->add(sc);
		}
		break;
	}
}