#pragma once

struct converter {
	const char*						name;
	converter*						next;
	static converter*				first;
	converter(const char* name);
	virtual unsigned				decode(void* output, int output_size, const void* input, int input_size) const = 0;
	virtual unsigned				encode(void* output, int output_size, const void* input, int input_size) const = 0;
	static converter*				find(const char* name);
};
unsigned							xdecode(const char* name, void* output, unsigned output_size, const void* input, unsigned input_size);
