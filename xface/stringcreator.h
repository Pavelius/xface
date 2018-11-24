#pragma once

struct stringcreator {
	const char*			parseformat(char* result, const char* result_max, const char* format, const char* format_param);
	virtual void		parseidentifier(char* result, const char* result_max, const char* identifier);
	static char*		parseint(char* result, const char* result_max, int value, int precision, const int radix);
	static char*		parsenumber(char* result, const char* result_max, unsigned value, int precision, const int radix);
	virtual void		parsevariable(char* result, const char* result_max, const char** format);
	void				print(char* result, const char* result_maximum, const char* format, ...);
	void				printv(char* result, const char* result_max, const char* format, const char* format_param);
};