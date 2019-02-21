#include "main.h"

using namespace code;

struct expression_info {
	const char*			name;
	operator_s			operands;
	bool				extended;
};

expression_info expression_data[] = {{"Do Nothing", Statement},
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

void expression::add(expression* v) {
	if(v->getoperands()==Statement && getoperands() == Statement) {
		auto push = next;
		next = v;
		v->next = push;
	}
}

void expression::builder::add(const expression* context, int v) {
	char temp[32]; stringcreator sc(temp);
	sc.add("%1i", v);
	add(NumberToken, context, temp);
}

void expression::addsingle(expression::builder& b) const {
	switch(expression_data[type].operands) {
	case Determinal:
		switch(type) {
		case DoNothing: break;
		case Number:
			b.add(this, value);
			break;
		case Text:
			b.add(OpenTag, this, "\"");
			b.add(TextToken, this, text);
			b.add(CloseTag, this, "\"");
			break;
		case Requisit:
			b.add(RequisitToken, this, req->id);
			break;
		case Metadata:
			b.add(MetadataToken, this, met->id);
			break;
		}
		break;
	case Binary:
		op1->add(b);
		b.add(Keyword, this, expression_data[type].name);
		op2->add(b);
		break;
	case Unary:
		b.add(Keyword, this, expression_data[type].name);
		if(op1)
			op1->add(b);
		break;
	case Statement:
		b.add(Keyword, this, expression_data[type].name);
		b.add(Whitespace, this, " ");
		if(op1) {
			if(expression_data[type].extended) {

			} else
				op1->add(b);
		}
		break;
	}
}

void expression::add(expression::builder& b) const {
	if(expression_data[type].operands == Statement) {
		for(auto p = this; p; p = p->next) {
			p->addsingle(b);
			b.addline();
		}
	} else
		addsingle(b);
}

void expression::zero() {
	switch(type) {
	case Number: value = 0; break;
	case Text: text = szdup(""); break;
	}
}

void expression::select(valuelist& v, expression_s t) const {
	switch(t) {
	case Requisit:
		for(auto& e : requisit_data) {
			if(e.parent != req->parent)
				continue;
			v.add(e.id, (int)&e, Requisit, 0);
		}
		break;
	case Metadata:
		for(auto& e : metadata_data)
			v.add(e.id, (int)&e, Metadata, 2);
		break;
	case While:
		for(auto& e : expression_data) {
			if(e.operands != Statement)
				continue;
			v.add(e.name, t, t, 5);
		}
		break;
	}
}

void expression::select(valuelist& v) const {
	auto t = getoperands();
	switch(t) {
	case Determinal:
		select(v, Requisit);
		select(v, Metadata);
		break;
	default:
		select(v, Metadata);
		select(v, While);
		break;
	}
}