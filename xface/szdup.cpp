#include "crt.h"

// Support class making string copy from strings storage.
template <class T> struct strcol {

	strcol*	next;
	int		count;
	T		data[256 * 255 / sizeof(T)]; // Inner buffer

	strcol() : next(0), count(0) {
	}

	~strcol() {
		seqclear(this);
	}

	bool has(const T* value) {
		strcol* e = this;
		while(e) {
			if(value >= e->data && value <= e->data + e->count)
				return true;
			e = e->next;
		}
		return false;
	}

	const T* find(const T* text, int textc) {
		if(textc == -1)
			textc = zlen(text);
		const T c = text[0];
		for(strcol* t = this; t; t = t->next) {
			int m = t->count - textc;
			if(m < 0)
				continue;
			for(int i = 0; i < m; i++) {
				if(c == data[i]) {
					int	j = 1;
					T*	p = &data[i];
					for(; j < textc; j++)
						if(p[j] != text[j])
							break;
					if(j == textc && p[j] == 0)
						return p;
				}
			}
		}
		return 0;
	}

	const T* add(const T* text, int textc) {
		if(!text)
			return 0;
		if(has(text))
			return text;
		if(textc == -1)
			textc = zlen(text);
		const T* r = find(text, textc);
		if(r)
			return r;
		strcol* t = this;
		while(true) {
			if((unsigned)(t->count + textc + 1) > sizeof(data) / sizeof(data[0])) {
				if(!t->next)
					t->next = new strcol;
				if(!t->next)
					return 0;
				t = t->next;
				continue;
			}
			T* result = &t->data[t->count];
			memcpy(result, text, textc * sizeof(text[0]));
			result[textc] = 0;
			t->count += textc + 1;
			return result;
		}
	}

};

const char* szdup(const char* text) {
	static strcol<char> small;
	static strcol<char> big;
	if(!text)
		return 0;
	if(text[0] == 0)
		return "";
	//text = zskipspcr(text);
	int lenght = zlen(text);
	if(lenght < 32)
		return small.add(text, lenght);
	else
		return big.add(text, lenght);
}

static bool ischa(unsigned char u) {
	return (u >= 'A' && u <= 'Z')
		|| (u >= 'a' && u <= 'z')
		|| (u >= ((unsigned char)'À') && u <= ((unsigned char)'ß'))
		|| (u >= ((unsigned char)'à') && u <= ((unsigned char)'ÿ'));
}

// Work only with english symbols
const char* sztag(const char* p) {
	char temp[128];
	char* s = temp;
	bool upper = true;
	while(*p) {
		if(*p != '_' && !ischa((unsigned char)*p) && !isnum(*p)) {
			upper = true;
			p++;
			continue;
		}
		if(upper) {
			szput(&s, szupper(szget(&p)));
			upper = false;
		} else
			*s++ = *p++;
	}
	*s++ = 0;
	return szdup(temp);
}