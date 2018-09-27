#include "converter.h"
#include "crt.h"

converter* converter::first;

converter::converter(const char* name) :name(name) {
	seqlink(this);
}

converter* converter::find(const char* name) {
	for(auto p = first; p; p = p->next) {
		if(strcmp(p->name, name) == 0)
			return p;
	}
	return 0;
}

unsigned xdecode(const char* name, void* output, unsigned output_size, const void* input, unsigned input_size) {
	for(auto n = converter::first; n; n = n->next) {
		if(strcmp(name, n->name) == 0)
			return n->decode(output, output_size, input, input_size);
	}
	return 0;
}