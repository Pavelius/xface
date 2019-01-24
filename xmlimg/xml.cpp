//Copyright 2014 by Pavel Chistyakov
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "xface/crt.h"
#include "xface/io.h"
#include "xml.h"

bool xml::var::operator==(const char* string) const {
	return szcmpi(name, string) == 0;
}

bool xml::var::operator!=(const char* string) const {
	return szcmpi(name, string) != 0;
}

xml::reader::reader(const char* url, bool url_is_text_buffer) : type(NameEnd), url_is_text_buffer(url_is_text_buffer) {
	if(url_is_text_buffer)
		this->pb = url;
	else
		this->pb = loadt(url);
	this->text = pb;
	if(text)
		this->end = zend(text);
	else
		this->end = 0;
	next();
}

xml::reader::~reader() {
	if(!url_is_text_buffer)
		delete pb;
	pb = 0;
	clear();
}

const char* xml::reader::next_name() {
	static char temp[256];
	text = zskipspcr(text);
	if((text[0] >= 'A' && text[0] <= 'Z')
		|| (text[0] >= 'a' && text[0] <= 'z')) {
		unsigned i = 0;
		while(true) {
			if((text[0] >= 'A' && text[0] <= 'Z')
				|| (text[0] >= 'a' && text[0] <= 'z')
				|| (text[0] >= '0' && text[0] <= '9')
				|| text[0] == '_') {
				if(i < sizeof(temp) / sizeof(temp[0]) - 1)
					temp[i++] = text[0];
			} else
				break;
			text++;
		}
		text = zskipspcr(text);
		temp[i] = 0;
		return temp;
	}
	return 0;
}

bool xml::reader::token(const char* name) {
	int size = zlen(name);
	if(!size)
		return false;
	if(memcmp(text, name, size * sizeof(text[0])) == 0) {
		text += size;
		return true;
	}
	return false;
}

char xml::reader::next_symbol() {
	if(text[0] == '&') {
		text++;
		if(text[0] == '#') {
			unsigned char i = 0;
			while(true) {
				if(text[0] == 0 || text[0] == ';')
					break;
				char a = text[0];
				if(a >= '0' && a <= '9')
					i = i * 10 + (a - '0');
				text++;
			}
			text++;
			return i;
		} else if(token("amp;"))
			return '&';
		else if(token("lt;"))
			return '<';
		else if(token("gt;"))
			return '>';
		else if(token("apos;"))
			return '\'';
		else if(token("quot;"))
			return '\"';
		else
			return '#';
	}
	return *text++;
}

void xml::reader::next_value(var& v, bool param) {
	static char temp[2048];
	char* ps = temp;
	char* pm = ps + sizeof(temp) / sizeof(temp[0]) - 1;
	if(param) {
		while(text[0]) {
			if(text[0] == '\"') {
				text++;
				break;
			}
			if(ps < pm)
				*ps++ = next_symbol();
		}
	} else if(token("<![CDATA[")) {
		while(text[0]) {
			if(text[0] == ']' && text[1] == ']') {
				text += 2;
				break;
			}
			if(ps < pm)
				*ps++ = *text++;
		}
	} else {
		while(text[0]) {
			if(text[0] == '<')
				break;
			if(ps < pm)
				*ps++ = next_symbol();
		}
	}
	*ps = 0;
	text = zskipspcr(text);
	v.value = temp;
}

void xml::reader::skip(char sym) {
	if(text[0] == sym)
		text++;
	else {
		// ошибка: ожидется символ sym
	}
}

void xml::reader::skip_comment() {
	while(true) {
		if(text[0] == '<' && text[1] == '!' && text[2] == '-' && text[3] == '-') {
			text += 4;
			while(text[0] && !(text[0] == '-' && text[1] == '-' && text[2] == '>'))
				text++;
			text = zskipspcr(text + 3);
			continue;
		}
		if(text[0] == '<' && text[1] == '?') {
			text += 2;
			while(text[0] && !(text[0] == '?' && text[1] == '>'))
				text++;
			text = zskipspcr(text + 2);
			continue;
		}
		break;
	}
}

void xml::reader::skip_attributes() {
	while(type == Attribute)
		next();
}

void xml::reader::skip_block() {
	skip_attributes();
	if(type == NameBegin) {
		next();
		skip_block();
		next();
	}
}

bool xml::reader::skip() {
	while(true) {
		switch(type) {
		case xml::NameBegin:
			skip_block();
			break;
		case xml::Attribute:
			break;
		case xml::NameEnd:
			return true;
		case xml::FileEnd:
			return false;
		}
		next();
	}
}

const char* xml::reader::content() {
	next();
	skip_block();
	const char* p = szdup(element.value);
	next();
	return p;
}

void xml::reader::next() {
	if(!text || !text[0]) {
		type = FileEnd;
		return;
	}
	skip_comment();
	text = zskipspcr(text);
	if(!text || !text[0]) {
		type = FileEnd;
		return;
	}
	switch(type) {
	case FileEnd:
		return;
	case NameEnd:
		skip('<');
		if(text[0] == '/') {
			text++;
			type = NameEnd;
			element.name = next_name();
			skip('>');
		} else {
			type = NameBegin;
			const char* p = next_name();
			if(p)
				element.name = szdup(p);
			else
				element.name = "";
			attribute.name = 0;
			element.value = 0;
		}
		break;
	case Attribute:
		if(text[0] == '/' && text[1] == '>') {
			text = zskipspcr(text + 2);
			type = NameEnd;
			break;
		}
		// продолжаем парсить
	case NameBegin:
		if(text[0] == '>') {
			text = zskipspcr(text + 1);
			type = NameEnd;
			next_value(element, false);
			next(); // рекурсивный вызов
		} else {
			type = Attribute;
			attribute.name = next_name();
			skip('=');
			text = zskipspcr(text + 1);
			skip('\"');
			next_value(attribute, true);
		}
		break;
	}
}