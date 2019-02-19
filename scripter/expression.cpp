#include "main.h"

using namespace code;

struct expression_info {
	const char*			name;
	int					operands;
	bool				statement;
};

expression_info expression_data[] = {{""},
{"Number"},
{"Text"},
{"Requisit"},
{"Metadata"},
{"+", 2},
{"-", 2},
{"*", 2},
{"/", 2},
{"%", 2},
{"&&", 2},
{"||", 2},
{"==", 2},
{"!=", 2},
{"<", 2},
{"<=", 2},
{">", 2},
{">=", 2},
{",", 2},
{"Var", 0, true},
{"List", 0, true},
{"While", 0, true},
{"If", 0, true},
{"For", 0, true},
{"Return", 1, true},
};

void expression::add(stringcreator& sc) {
	switch(type) {
	case End:
		break;
	case Number: sc.add("%1i", value); break;
	case Text: sc.add("\"%1\"", text); break;
	case Requisit: sc.add(req->id); break;
	case Metadata: sc.add(met->id); break;
	default:
		if(expression_data[type].operands == 2) {
			op1->add(sc);
			sc.add(expression_data[type].name);
			op2->add(sc);
		} else if(expression_data[type].operands == 1) {
			sc.add(expression_data[type].name);
			if(expression_data[type].statement)
				sc.add(" ");
			op1->add(sc);
		}
		break;
	}
}