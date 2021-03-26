#include "lexer.h"

static lexer::word words_c3[] = {
	{"int", Type}, // Types
	{"unsigned", Type},
	{"short", Type},
	{"ushort", Type},
	{"char", Type},
	{"uchar", Type},
	{"void", Type},
	{"bool", Type},
	{"while", Keyword}, // Statements
	{"if", Keyword},
	{"else", Keyword},
	{"for", Keyword},
	{"do", Keyword},
	{"switch", Keyword},
	{"case", Keyword},
	{"break", Keyword},
	{"continue", Keyword},
	{"default", Keyword},
	{"import", Keyword}, // Directive
	{"enum", Keyword},
	{"return", Keyword},
};
static lexer source = {"c2", "*.c2", "><|&~=!+-/*,.;", ">=<=!===>><<", words_c3, sizeof(words_c3) / sizeof(words_c3[0]),
	{'{', '}'},
	{'(', ')'},
	{'[', ']'},
};

void setlexer(lexer* v);

void initialize_lexer() {
	setlexer(&source);
}