//Copyright 2013 by Pavel Chistyakov
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

#include "crt.h"
#include "io_plugin.h"

struct xml_writer : public io::writer
{

	int	level;
	int	elc[64];

	void write_attr(const char* name)
	{
		e << " " << name << "=";
	}

	void write_value(const char* value)
	{
		e << "\"" << value << "\"";
	}

	void write_value(int value)
	{
		e << "\"" << value << "\"";
	}

	void write_cr()
	{
		if(level == -1)
			return;
		e << "\r\n";
		for(int i = 0; i < level; i++)
			e << "  ";
	}

	void open(const char* name, int type) override
	{
		if(level && elc[level] == 0)
			e << ">";
		write_cr();
		e << "<" << name;
		elc[++level] = 0;
	}

	void set(const char* name, int value, int type) override
	{
		write_attr(name);
		write_value(value);
	}

	void set(const char* name, const char* value, int type) override
	{
		write_attr(name);
		write_value(value);
	}

	void close(const char* name, io::node_s type) override
	{
		if(!elc[level])
		{
			e << "/>";
			level--;
			elc[level]++;
		}
		else
		{
			level--;
			elc[level]++;
			write_cr();
			e << "</" << name << ">";
		}
	}

	xml_writer(io::stream& e) : writer(e), level(0)
	{
		memset(elc, 0, sizeof(elc));
		const char* header = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		e.write(header, zlen(header));
	}

};

static const char* next(const char* p)
{
	while(true)
	{
		// Skip comments
		if(p[0] == '<' && p[1] == '!' && p[2] == '-' && p[3] == '-')
		{
			p += 4;
			while(!(p[0] == '-' && p[1] == '-' && p[2] == '>'))
			{
				if(p[0] == 0)
					return p;
				p++;
			}
			p = zskipspcr(p + 3);
			continue;
		}
		// Skip handler instructions
		if(p[0] == '<' && p[1] == '?')
		{
			p += 2;
			while(!(p[0] == '?' && p[1] == '>'))
			{
				if(p[0] == 0)
					return p;
				p++;
			}
			p = zskipspcr(p + 2);
			continue;
		}
		break;
	}
	return zskipspcr(p);
}

static const char* read_name(const char* p, io::reader::node& n)
{
	char temp[260];
	auto r = temp; r[0] = 0;
	auto re = temp + sizeof(temp) / sizeof(temp[0]) - 1;
	while(*p)
	{
		if((*p >= '0' && *p <= '9') || *p == '_' || ischa(*p) || *p == '.' || *p == ':')
		{
			if(r < re)
				*r++ = *p++;
			else
				p++;
		}
		else
			break;
	}
	*r = 0;
	n.name = szdup(temp);
	return next(p);
}

const char* read_symbol(const char* p, char& result)
{
	if(p[0] == '&')
	{
		p++;
		if(p[0] == '#')
		{
			unsigned char i = 0;
			while(true)
			{
				if(p[0] == 0 || p[0] == ';')
					break;
				char a = p[0];
				if(a >= '0' && a <= '9')
					i = i * 10 + (a - '0');
				p++;
			}
			p++;
			result = i;
			return p;
		}
		else if(szmatch(p, "amp;"))
		{
			p += 4;
			result = '&';
			return p;
		}
		else if(szmatch(p, "lt;"))
		{
			p += 3;
			result = '<';
			return p;
		}
		else if(szmatch(p, "gt;"))
		{
			p += 3;
			result = '>';
			return p;
		}
		else if(szmatch(p, "apos;"))
		{
			p += 5;
			result = '\'';
			return p;
		}
		else if(szmatch(p, "quot;"))
		{
			p += 5;
			result = '\"';
			return p;
		}
		result = '#';
		return p;
	}
	result = *p++;
	return p;
}

static const char* read_value(const char* p, char* result, unsigned size)
{
	auto pe = result + size;
	while(true)
	{
		if(p[0] == 0)
		{
			*result++ = 0;
			return p;
		}
		else if(p[0] == '\"')
		{
			*result++ = 0;
			return next(p + 1);
		}
		char a;
		p = read_symbol(p, a);
		if(result < pe)
			*result++ = a;
	}
}

static const char* read_object(const char* p, io::reader::node& n, io::reader& e)
{
	p = next(p);
	if(p[0] == '<')
	{
		p = read_name(next(p + 1), n);
		e.open(n);
		// Read parameters
		while(ischa(*p) || *p == '_' || *p == ':')
		{
			char temp[1024];
			io::reader::node at(n);
			p = read_name(p, at);
			if(p[0] == '=')
			{
				p = next(p + 1);
				if(p[0] == '\"')
				{
					p = read_value(p + 1, temp, sizeof(temp) / sizeof(temp[0]));
					e.set(at, temp);
				}
			}
		}
		// If object is closed
		if(p[0] == '/' && p[1] == '>')
		{
			e.close(n);
			return next(p + 2);
		}
		else if(p[0] == '>')
		{
			p = next(p + 1);
			while(p[0] == '<')
			{
				if(p[0] == '<' && p[1] == '/')
				{
					p = read_name(next(p + 2), n);
					if(p[0] == '>')
					{
						p = next(p + 1);
						e.close(n);
					}
					return p;
				}
				else
				{
					io::reader::node cn(n);
					p = read_object(p, cn, e);
				}
			}
		}
	}
	return p;
}

static struct xml_reader_parser : public io::plugin
{

	const char* read(const char* p, io::reader& e) override
	{
		io::reader::node root;
		root.type = io::Struct;
		return read_object(p, root, e);
	}

	io::writer* write(io::stream& e) override
	{
		return new xml_writer(e);
	}

	xml_reader_parser()
	{
		name = "xml";
		fullname = "XML data object";
		filter = "*.xml";
	}

} reader_parser_instance;