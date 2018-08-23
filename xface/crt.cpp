#include "crt.h"
#include "io.h"

int	locale; // Current localization

void setsignature(char d[4], const char* s)
{
	d[0] = s[0];
	d[1] = s[1];
	d[2] = s[2];
	d[3] = 0;
}

bool issignature(const char d[4], const char* s)
{
	return (d[0] == s[0])
		&& (d[1] == s[1])
		&& (d[2] == s[2])
		&& d[3]==0;
}

char* szsep(char* result, const char* separator, const char* start)
{
	if(result[0])
		zcat(result, separator);
	else if(start)
	{
		zcpy(result, start);
		return result;
	}
	return zend(result);
}

void szadd(char* result, const char* value, const char* title, const char* separator)
{
	if(result[0] == 0)
	{
		if(title)
			zcpy(result, title);
	}
	else
		zcat(result, separator);
	zcat(result, value);
}

const char* szskipcr(const char* p)
{
	if(*p == '\n')
	{
		p++;
		if(*p == '\r')
			p++;
	}
	else if(*p == '\r')
	{
		p++;
		if(*p == '\n')
			p++;
	}
	return p;
}

const char* szskipcrr(const char* p0, const char* p)
{
	if(!p)
		return 0;
	if(p0 >= p)
		return p;
	if(p[-1] == '\n')
	{
		p--;
		if(p0 >= p)
			return p;
		if(p[-1] == '\r')
			p--;
	}
	else if(p[-1] == '\r')
	{
		p--;
		if(p0 >= p)
			return p;
		if(p[-1] == '\n')
			p--;
	}
	return p;
}

int szcmp(const char* p1, const char* p2, int max_count)
{
	for(; *p2 && *p1 && max_count>0; p1++, p2++, max_count--)
	{
		if(*p1 == *p2)
			continue;
		return *p1 - *p2;
	}
	if(!max_count)
		return 0;
	return *p1 - *p2;
}

int szcmpi(const char* p1, const char* p2)
{
	while(*p2 && *p1)
	{
		unsigned s1 = szupper(szget(&p1));
		unsigned s2 = szupper(szget(&p2));
		if(s1 == s2)
			continue;
		return s1 - s2;
	}
	unsigned s1 = szupper(szget(&p1));
	unsigned s2 = szupper(szget(&p2));
	return s1 - s2;
}

int szcmpi(const char* p1, const char* p2, int max_count)
{
	while(*p2 && *p1 && max_count-- > 0)
	{
		unsigned s1 = szupper(szget(&p1));
		unsigned s2 = szupper(szget(&p2));
		if(s1 == s2)
			continue;
		return s1 - s2;
	}
	if(!max_count)
		return 0;
	unsigned s1 = szupper(szget(&p1));
	unsigned s2 = szupper(szget(&p2));
	return s1 - s2;
}

char* sznum(char* result, int num, int precision, const char* empthy, int radix)
{
	char* p1 = result;
	if(num == 0)
	{
		if(empthy)
			zcpy(p1, empthy);
		else
		{
			zcpy(p1, "0");
			while(--precision > 0)
				zcat(p1, "0");
		}
		p1 = zend(p1);
	}
	else
	{
		char temp[32];
		int p = 0;
		if(num < 0)
		{
			*p1++ = '-';
			num = -num;
		}
		switch(radix)
		{
		case 16:
			while(num)
			{
				int a = (num%radix);
				if(a > 9)
					temp[p++] = 'A' - 10 + a;
				else
					temp[p++] = '0' + a;
				num /= radix;
			}
			break;
		default:
			while(num)
			{
				temp[p++] = '0' + (num%radix);
				num /= radix;
			}
			break;
		}
		while(precision-- > p)
			*p1++ = '0';
		while(p)
			*p1++ = temp[--p];
		p1[0] = 0;
	}
	return result;
}

char* sznum(char* outbuf, float f, int precision, const char* empthy)
{
	typedef union {
		long	L;
		float	F;
	} LF_t;
	long mantissa, int_part, frac_part;
	short exp2;
	LF_t x;
	outbuf[0] = 0;
	if(f == 0.0)
	{
		if(empthy)
			zcpy(outbuf, empthy);
		return outbuf;
	}
	x.F = f;

	exp2 = (unsigned char)(x.L >> 23) - 127;
	mantissa = (x.L & 0xFFFFFF) | 0x800000;
	frac_part = 0;
	int_part = 0;

	if(exp2 >= 31)
		return 0;
	else if(exp2 < -23)
		return 0;
	else if(exp2 >= 23)
		int_part = mantissa << (exp2 - 23);
	else if(exp2 >= 0)
	{
		int_part = mantissa >> (23 - exp2);
		frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
	}
	else
		frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);

	auto p = outbuf;

	if(x.L < 0)
		*p++ = '-';

	if(int_part == 0)
		*p++ = '0';
	else
	{
		sznum(p, int_part, 0, 0, 10);
		while(*p)
			p++;
	}
	if(frac_part == 0)
	{
		if(precision)
		{
			*p++ = '.';
			for(int i = 0; i < precision; i++)
				*p++ = 0;
		}
	}
	else
	{
		int max = 7;
		*p++ = '.';
		if(precision)
			max = precision;
		/* print BCD */
		for(int m = 0; m < max; m++)
		{
			/* frac_part *= 10;	*/
			frac_part = (frac_part << 3) + (frac_part << 1);
			*p++ = (frac_part >> 24) + '0';
			frac_part &= 0xFFFFFF;
		}
		if(precision == 0)
		{
			for(--p; p[0] == '0' && p[-1] != '.'; --p);
			++p;
		}
	}
	*p = 0;
	return outbuf;
}

int sz2num(const char* p1, const char** pp1)
{
	int result = 0;
	bool sign = false;
	const int radix = 10;
	while(*p1 && *p1 != '-' && (*p1 < '0' || *p1 > '9'))
		p1++;
	if(*p1 == '-')
	{
		sign = true;
		p1++;
	}
	while(*p1)
	{
		char a = *p1;
		if(a < '0' || a > '9')
			break;
		result = result*radix;
		result += a - '0';
		p1++;
	}
	if(sign)
		result = -result;
	if(pp1)
		*pp1 = p1;
	return result;
}

char* szurlc(char* result)
{
	char* p = result;
	while(*p)
	{
		if(*p == '\\')
			*p = '/';
		p++;
	}
	return zskipspcr(result);
}

char* szurl(char* p, const char* path, const char* name, const char* ext, const char* suffix)
{
	if(!p)
		return 0;
	*p = 0;
	if(path)
	{
		zcpy(p, path);
		zcat(p, "/");
	}
	if(name)
		zcat(p, name);
	if(suffix)
		zcat(p, suffix);
	if(ext && szext(p) == 0)
	{
		zcat(p, ".");
		zcat(p, ext);
	}
	return szurlc(p);
}

const char* szext(const char* path)
{
	for(const char* r = zend((char*)path); r > path; r--)
	{
		if(*r == '.')
			return r + 1;
		else if(*r == '\\' || *r == '/')
			return 0;
	}
	return 0;
}

const char* szfname(const char* path)
{
	for(const char* r = zend((char*)path); r > path; r--)
	{
		if(*r == '\\' || *r == '/')
			return r + 1;
	}
	return path;
}

char* szfnamewe(char* result, const char* name)
{
	zcpy(result, szfname(name));
	char* p = (char*)szext(result);
	if(p && p!=result)
		p[-1] = 0;
	return result;
}

const char* szline(const char* p, int number)
{
	if(number < 1)
		return p;
	while(true)
	{
		switch(*p)
		{
		case 0:
			return p;
		case '\n':
		case '\r':
			if((p[0] == '\n' && p[1] == '\r')
				|| (p[0] == '\r' && p[1] == '\n'))
				p += 2;
			else
				p++;
			if(--number == 0)
				return p;
			break;
		default:
			p++;
			break;
		}
	}
}

const char* szline(const char* p, int line_number, int column_number)
{
	const char* p1 = szline(p, line_number);
	if(column_number < 0)
		return p1;
	while(column_number--)
	{
		unsigned sym = szget(&p1);
		if(sym == '\n' || sym == '\r')
			return p1;
	}
	return p1;
}

const char* szlineb(const char* start_text, const char* position)
{
	if(!start_text || !position)
		return position;
	while(position > start_text)
	{
		// Для формата unicode это также будет работать
		if(position[-1] == '\n' || position[-1] == '\r')
			return position;
		position--;
	}
	return start_text;
}

const char* szlinee(const char* string)
{
	register const char* p = string;
	while(*p && *p != '\n' && *p != '\r')
		p++;
	return p;
}

int szline(const char* p, const char* pos)
{
	int r = 0;
	while(true)
	{
		switch(*p)
		{
		case 0:
			return r;
		case '\n':
		case '\r':
			if((p[0] == '\n' && p[1] == '\r')
				|| (p[0] == '\r' && p[1] == '\n'))
				p += 2;
			else
				p++;
			if(p > pos)
				return r;
			r++;
			break;
		default:
			p++;
			break;
		}
	}
}

bool szmatch(const char* text, const char* name)
{
	while(*name)
	{
		if(*name++ != *text++)
			return false;
	}
	if(ischa(*text))
		return false;
	return true;
}

bool matchuc(const char* name, const char* filter)
{
	if(!name || name[0] == 0)
		return false;
	auto sym = szupper(szget(&filter));
	auto pn = name;
	while(pn[0])
	{
		auto sym1 = szupper(szget(&pn));
		if(sym1 == sym)
		{
			auto pf = filter;
			auto ps = pn;
			while(true)
			{
				if(pf[0] == 0)
					return true;
				auto sym2 = szupper(szget(&pf));
				auto sym1 = szupper(szget(&pn));
				if(sym1 != sym2)
					break;
			}
			pn = ps;
		}
	}
	return false;
}

bool ischa(unsigned u)
{
	return (u >= 'A' && u <= 'Z')
		|| (u >= 'a' && u <= 'z')
		|| (u >= 0x410 && u <= 0x44F);
}

unsigned szupper(unsigned u)
{
	if(u >= 0x61 && u <= 0x7A)
		return u - 0x61 + 0x41;
	else if(u >= 0x430 && u <= 0x44F)
		return u - 0x430 + 0x410;
	return u;
}

char* szupper(char* p, int count)
{
	char* s1 = p;
	const char* p1 = p;
	while(count-- > 0)
		szput(&s1, szupper(szget(&p1)));
	return p;
}

unsigned szlower(unsigned u)
{
	if(u >= 0x41 && u <= 0x5A)
		return u - 0x41 + 0x61;
	else if(u >= 0x410 && u <= 0x42F)
		return u - 0x410 + 0x430;
	return u;
}

int getdigitscount(unsigned number)
{
	if(number < 10)
		return 1;
	if(number < 100)
		return 2;
	if(number < 1000)
		return 3;
	if(number < 10000)
		return 4;
	if(number < 100000)
		return 5;
	if(number < 1000000)
		return 6;
	if(number < 10000000)
		return 7;
	if(number < 100000000)
		return 8;
	return 9;
}

void szlower(char* p, int count)
{
	char* s1 = p;
	const char* p1 = p;
	if(count == -1)
	{
		while(true)
		{
			unsigned sym = szget(&p1);
			if(!sym)
				break;
			szput(&s1, szlower(sym));
		}
		szput(&s1, 0);
	}
	else
	{
		while(count-- > 0)
			szput(&s1, szlower(szget(&p1)));
	}
}

unsigned szget(const char** input, codepages code)
{
	const unsigned char* p;
	unsigned result;
	switch(code)
	{
	case CPUTF8:
		p = (unsigned char*)*input;
		result = *p++;
		if(result >= 192 && result <= 223)
			result = (result - 192) * 64 + (*p++ - 128);
		else if(result >= 224 && result <= 239)
		{
			result = (result - 224) * 4096 + (p[0] - 128) * 64 + (p[1] - 128);
			p += 2;
		}
		*input = (const char*)p;
		return result;
	case CPU16LE:
		p = (unsigned char*)*input;
		result = p[0]|(p[1]<<8);
		*input = (const char*)(p + 2);
		return result;
	case CP1251:
		result = (unsigned char)*(*input)++;
		if(((unsigned char)result >= 0xC0))
			return result - 0xC0 + 0x410;
		else switch(result)
		{
		case 0xB2: return 0x406;
		case 0xAF: return 0x407;
		case 0xB3: return 0x456;
		case 0xBF: return 0x457;
		}
		return result;
	default:
		return *(*input)++;
	}
}

void szput(char** output, unsigned value, codepages code)
{
	char* p;
	switch(code)
	{
	case CPUTF8:
		p = *output;
		if(((unsigned short)value) < 128)
			*p++ = (unsigned char)value;
		else if(((unsigned short)value) < 2047)
		{
			*p++ = (unsigned char)(192 + (((unsigned short)value) / 64));
			*p++ = (unsigned char)(128 + (((unsigned short)value) % 64));
		}
		else
		{
			*p++ = (unsigned char)(224 + (((unsigned short)value) / 4096));
			*p++ = (unsigned char)(128 + ((((unsigned short)value) / 64) % 64));
			*p++ = (unsigned char)(224 + (((unsigned short)value) % 64));
		}
		*output = p;
		break;
	case CP1251:
		if(value >= 0x410 && value <= 0x44F)
			value = value - 0x410 + 0xC0;
		else switch(value)
		{
		case 0x406: value = 0xB2; break; // I
		case 0x407: value = 0xAF; break; // Ї
		case 0x456: value = 0xB3; break;
		case 0x457: value = 0xBF; break;
		}
		*(*output)++ = (unsigned char)value;
		break;
	case CPU16LE:
		*(*output)++ = (unsigned char)(value & 0xFF);
		*(*output)++ = (unsigned char)(((unsigned)value >> 8));
		break;
	case CPU16BE:
		*(*output)++ = (unsigned char)(((unsigned)value >> 8));
		*(*output)++ = (unsigned char)(value & 0xFF);
		break;
	default:
		*(*output)++ = (unsigned char)value;
		break;
	}
}

char* szput(char* result, unsigned sym, codepages page)
{
	char* p = result;
	szput(&p, sym, page);
	*p = 0;
	return result;
}

void szencode(char* output, int output_count, codepages output_code, const char* input, int input_count, codepages input_code)
{
	char* s1 = output;
	char* s2 = s1 + output_count;
	const char* p1 = input;
	const char* p2 = p1 + input_count;
	while(p1 < p2 && s1 < s2)
		szput(&s1, szget(&p1, input_code), output_code);
	if(s1 < s2)
	{
		s1[0] = 0;
		if((output_code == CPU16BE || output_code == CPU16LE) && (s1+1)<s2)
			s1[1] = 0;
	}
}

static bool szpmatch(const char* text, const char* s, const char* s2)
{
	while(true)
	{
		register const char* d = text;
		while(s<s2)
		{
			if(*d == 0)
				return false;
			unsigned char c = *s;
			if(c == '?')
			{
				s++;
				d++;
			}
			else if(c == '*')
			{
				s++;
				if(s == s2)
					return true;
				while(*d)
				{
					if(*d == *s)
						break;
					d++;
				}
			}
			else
			{
				if(*d++ != *s++)
					return false;
			}
		}
		return true;
	}
}

bool szpmatch(const char* text, const char* pattern)
{
	const char* p = pattern;
	while(true)
	{
		const char* p2 = zchr(p, ',');
		if(!p2)
			p2 = zend(p);
		if(szpmatch(text, p, p2))
			return true;
		if(*p2 == 0)
			return false;
		p = zskipsp(p2 + 1);
	}
}