#include "xface/io.h"

#pragma once

namespace xml {
enum					tags { NameBegin, NameEnd, Attribute, FileEnd };
struct var {
	const char*			name;
	const char*			value;
	//
	bool				operator==(const char* string) const;
	bool				operator!=(const char* string) const;
};
struct string {
	const char*			text;
	const char*			end;
	explicit operator bool() const { return end > text; }
	void				clear() { text = end; }
};
// Lite xml reader. Can read data from file.
struct reader : string {
	tags				type;		// Current state or element/attribute type
	var					element;	// In/Out element properties (elements name refs to variable "names" in this namespace)
	var					attribute;	// In/Out attribute properties (attribute name refs to same static address)
	//
	reader(const char* url, bool url_is_text_buffer = false);
	~reader();
	//
	const char*			content();			// Get tag content, skipping all attributes and inner groups
	void				next();				// Get next state or attribute
	bool				skip();				// Skip all block content and children block until end of name
	void				skip_attributes();	// Skip all attributes (if any) and go direct to content or end block
	void				skip_block();		// Skip group block
private:
	bool				url_is_text_buffer;
	const char*			pb;
	bool				token(const char* name);
	void				next_value(var& v, bool param);
	char				next_symbol();
	const char*			next_name();
	void				skip(char sym);
	void				skip_comment();
};
}