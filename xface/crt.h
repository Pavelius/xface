#pragma once

#ifdef _MSC_VER
namespace std {
template<class T>
class initializer_list {	// list of pointers to elements
public:
	typedef T						value_type;
	typedef const T&				reference;
	typedef const T&				const_reference;
	typedef unsigned				size_type;
	typedef const T*				iterator;
	typedef const T*				const_iterator;
	constexpr initializer_list() noexcept : first(0), last(0) {}
	constexpr initializer_list(const T* first_arg, const T* last_arg) noexcept : first(first_arg), last(last_arg) {}
	constexpr const T*				begin() const noexcept { return first; }
	constexpr const T*				end() const noexcept { return last; }
	constexpr unsigned				size() const noexcept { return last - first; }
private:
	const T* first;
	const T* last;
};
}
#else
#include <initializer_list>
#endif

#ifdef _DEBUG
#define assert(e) if(!(e)) {exit(255);}
#else
#define assert(e)
#endif
#define maptbl(t, id) (t[imax((unsigned)0, imin((unsigned)id, (sizeof(t)/sizeof(t[0])-1)))])
#define maprnd(t) t[rand()%(sizeof(t)/sizeof(t[0]))]
#define lenof(t) (sizeof(t)/sizeof(t[0]))
#define zendof(t) (t + sizeof(t)/sizeof(t[0]) - 1)
#define	FO(c,m) ((unsigned)&((c*)0)->m)
#define BSDATA(e) template<> e bsdata<e>::elements[]
#define BSDATAC(e, c) template<> e bsdata<e>::elements[c] = {}; template<> array bsdata<e>::source(bsdata<e>::elements, sizeof(bsdata<e>::elements[0]), 0, sizeof(bsdata<e>::elements)/sizeof(bsdata<e>::elements[0]));
#define NOBSDATA(e) template<> struct bsdata<e> : bsdata<int> {};
#define BSMETA(e) template<> const bsreq bsmeta<e>::meta[]
#define BSHEAD(e) template<> array bsdata<e>::source(bsdata<e>::elements, sizeof(bsdata<e>::elements[0]), sizeof(bsdata<e>::elements)/sizeof(bsdata<e>::elements[0]));
#define BSLNK(T1, T2) template<> struct bsdata<T1> : bsdata<T2> {};
#define BSINF(e) {#e, bsmeta<e##i>::meta, bsdata<e##i>::source}

extern "C" int						atexit(void(*func)(void));
extern "C" void*					bsearch(const void* key, const void* base, unsigned num, unsigned size, int(*compar)(const void*, const void*));
extern "C" unsigned					clock(); // Returns the processor time consumed by the program.
extern "C" void						exit(int exit_code);
extern "C" int						memcmp(const void* p1, const void* p2, unsigned size);
extern "C" void*					memmove(void* destination, const void* source, unsigned size);
extern "C" void*					memcpy(void* destination, const void* source, unsigned size);
extern "C" void*					memset(void* destination, int value, unsigned size);
extern "C" void						qsort(void* base, unsigned num, unsigned size, int(*compar)(const void*, const void*));
extern "C" int						rand(void); // Get next random value
extern "C" void						srand(unsigned seed); // Set random seed
extern "C" int						strcmp(const char* s1, const char* s2); // Compare two strings
extern "C" long long				time(long long* seconds);

enum codepages { CPNONE, CP1251, CPUTF8, CPU16BE, CPU16LE };
namespace metrics {
const codepages						code = CP1251;
}
//
int									getdigitscount(unsigned number); // Get digits count of number. For example if number=100, result be 3.
template<class T> const char*		getstr(T e); // Template to return string of small class
bool								ischa(unsigned u); // is alphabetical character?
inline bool							isnum(unsigned u) { return u >= '0' && u <= '9'; } // is numeric character?
int									isqrt(const int x); // Return aquare root of 'x'
void*								loadb(const char* url, int* size = 0, int additional_bytes_alloated = 0); // Load binary file.
char*								loadt(const char* url, int* size = 0); // Load text file and decode it to system codepage.
bool								matchuc(const char* name, const char* filter);
const char*							psidn(const char* p, char* result, char* result_end); // safe parse identifier name from string
inline const char*					psidn(const char* p, char* result) { return psidn(p, result, result + 128); }
const char*							psnum(const char* p, int& value); // Parse number from string
const char*							psstr(const char* p, char* value, char end_symbol = '\"'); // Parse string from string (like c format "Some\nstring")
unsigned							rmoptimal(unsigned need_count);
void								rmremove(void* data, unsigned size, unsigned index, unsigned& count, int elements_count);
void*								rmreserve(void* data, unsigned new_size);
void								rmreserve(void** data, unsigned count, unsigned& count_maximum, unsigned size);
float								sqrt(const float x); // Return aquare root of 'x'
int									sz2num(const char* p1, const char** pp1 = 0);
void								szencode(char* output, int output_count, codepages output_code, const char* input, int input_count, codepages input_code);
unsigned							szget(const char** input, codepages page = metrics::code);
int									szcmpi(const char* p1, const char* p2);
int									szcmpi(const char* p1, const char* p2, int count);
const char*							szdup(const char* text);
const char*							szext(const char* path);
const char*							szfname(const char* text); // Get file name from string (no fail, always return valid value)
char*								szfnamewe(char* result, const char* name); // get file name without extension (no fail)
unsigned							szlower(unsigned u); // to lower reg
void								szlower(char* p, int count = 1); // to lower reg
bool								szmatch(const char* text, const char* name); //
char*								sznum(char* result, int num, int precision = 0, const char* empthy = 0, int radix = 10);
char*								sznum(char* result, float f, int precision = 0, const char* empthy = "0.00");
bool								szpmatch(const char* text, const char* pattern);
void								szput(char** output, unsigned u, codepages page = metrics::code);
char*								szput(char* output, unsigned u, codepages page = metrics::code); // Fast symbol put function. Return 'output'.
const char*							szskipcr(const char* p);
const char*							szskipcrr(const char* p0, const char* p);
bool								szstart(const char* text, const char* name);
unsigned							szupper(unsigned u);
char*								szupper(char* p, int count = 1); // to upper reg
char*								szurl(char* p, const char* path, const char* name, const char* ext = 0, const char* suffix = 0);
char*								szurlc(char* p1);
inline int							xrand(int n1, int n2) { return n1 + rand() % (n2 - n1 + 1); }
// Common used templates
inline int							ifloor(double n) { return (int)n; }
template<class T> inline T			imax(T a, T b) { return a > b ? a : b; }
template<class T> inline T			imin(T a, T b) { return a < b ? a : b; }
template<class T> inline T			iabs(T a) { return a > 0 ? a : -a; }
template<class T> inline void		iswap(T& a, T& b) { T i = a; a = b; b = i; }
// Inline sequence functions
template<class T> inline void		seqclear(T* p) { T* z = p->next; while(z) { T* n = z->next; z->next = 0; delete z; z = n; } p->next = 0; } // Use to clean up sequenced resources if destructor. Use it like 'clear(this)'.
template<class T> inline T*			seqlast(T* p) { while(p->next) p = p->next; return p; } // Return last element in sequence.
template<class T> inline void		seqlink(T* p) { p->next = 0; if(!T::first) T::first = p; else seqlast(T::first)->next = p; }
// Inline strings functions
template<class T> inline const T*	zchr(const T* p, T e) { while(*p) { if(*p == e) return p; p++; } return 0; }
template<class T> inline void		zcpy(T* p1, const T* p2) { while(*p2) *p1++ = *p2++; *p1 = 0; }
template<class T> inline void		zcpy(T* p1, const T* p2, int max_count) { while(*p2 && max_count-- > 0) *p1++ = *p2++; *p1 = 0; }
template<class T> inline T*			zend(T* p) { while(*p) p++; return p; }
template<class T> inline void		zcat(T* p1, const T e) { p1 = zend(p1); p1[0] = e; p1[1] = 0; }
template<class T> inline void		zcat(T* p1, const T* p2) { zcpy(zend(p1), p2); }
template<class T> inline int		zlen(T* p) { return zend(p) - p; }
template<class T> inline void		zshuffle(T* p, int count) { for(int i = 0; i < count; i++) iswap(p[i], p[rand() % count]); }
template<class T> inline T*			zskipsp(T* p) { if(p) while(*p == 32 || *p == 9) p++; return p; }
template<class T> inline T*			zskipspcr(T* p) { if(p) while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++; return p; }
// Light-weaight proxy container
template<class T>
class aref {
	T*								data;
	unsigned						count;
public:
	constexpr aref() : data(0), count(0) {}
	constexpr aref(T* data, unsigned count) : data(data), count(count) {}
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	constexpr T*					begin() { return data; }
	constexpr const T*				begin() const { return data; }
	constexpr T*					end() { return data + count; }
	constexpr const T*				end() const { return data + count; }
	constexpr unsigned				getcount() const { return count; }
	constexpr int					indexof(const T* e) const { if(e >= data && e < data + count) return e - data; return -1; }
	constexpr int					indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	constexpr bool					is(const T t) const { return indexof(t) != -1; }
};
// Storge like vector
template<class T, int count_max = 128>
struct adat {
	T								data[count_max];
	unsigned						count;
	constexpr adat() : data{}, count(0) {}
	constexpr adat(const std::initializer_list<T> list) : count(0) { for(auto& e : list) add(e); }
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	T*								add() { if(count < count_max) return data + (count++); return data; }
	void							add(const T& e) { if(count < count_max) data[count++] = e; }
	T*								begin() { return data; }
	const T*						begin() const { return data; }
	void							clear() { count = 0; }
	T*								end() { return data + count; }
	const T*						end() const { return data + count; }
	const T*						endof() const { return data + count_max; }
	int								getcount() const { return count; }
	int								getmaximum() const { return count_max; }
	int								indexof(const T* e) const { if(e >= data && e < data + count) return e - data; return -1; }
	int								indexof(const T t) const { for(auto& e : *this) if(e == t) return &e - data; return -1; }
	bool							is(const T t) const { return indexof(t) != -1; }
	void							remove(int index, int remove_count = 1) { if(index < 0) return; if(index<int(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0]) * (count - index - 1)); count--; }
};
typedef adat<void*, 64>				reflist;
// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned>
class cflags {
	DT								data;
public:
	constexpr cflags() : data(0) {}
	constexpr cflags(const std::initializer_list<T> list) : data() { for(auto e : list) add(e); }
	constexpr void					add(const T id) { data |= 1 << id; }
	constexpr void					clear() { data = 0; }
	constexpr bool					is(const T id) const { return (data & (1 << id)) != 0; }
	constexpr void					remove(T id) { data &= ~(1 << id); }
};
// Abstract array vector
class array {
	unsigned						size;
	unsigned						count_maximum;
	void							grow(unsigned offset, unsigned delta);
	void							shrink(unsigned offset, unsigned delta);
	void							zero(unsigned offset, unsigned delta);
public:
	void*							data;
	unsigned						count;
	typedef int(*pcompare)(const void* p1, const void* p2, void* param);
	constexpr array(unsigned size = 0) : size(size), count_maximum(0), data(0), count(0) {}
	constexpr array(void* data, unsigned size, unsigned count) : size(size), count_maximum(count | 0x80000000), data(data), count(count) {}
	constexpr array(void* data, unsigned size, unsigned count, unsigned count_maximum) : size(size), count_maximum(count_maximum | 0x80000000), data(data), count(count) {}
	~array();
	void*							add();
	void*							addz() { auto p = add(); memset(p, 0, size); return p; }
	void*							add(const void* element);
	char*							begin() const { return (char*)data; }
	void							change(unsigned offset, int size);
	void							clear();
	char*							end() const { return (char*)data + size * count; }
	int								find(const char* value, unsigned offset) const;
	int								find(void* value, unsigned offset, unsigned size) const;
	unsigned						getmaximum() const { return count_maximum & 0x7FFFFFFF; }
	unsigned						getcount() const { return count; }
	unsigned						getsize() const { return size; }
	int								indexof(const void* element) const;
	void*							insert(int index, const void* element);
	bool							isgrowable() const { return (count_maximum & 0x80000000) == 0; }
	void*							ptr(int index) const { return (char*)data + size * index; }
	template<class T> aref<T> records() const { return aref<T>((T*)data, count); }
	void							remove(int index, int elements_count = 1);
	void							shift(int i1, int i2, unsigned c1, unsigned c2);
	void							setcount(unsigned value) { count = value; }
	void							setup(unsigned size);
	void							sort(int i1, int i2, pcompare compare, void* param);
	void							swap(int i1, int i2);
	void							reserve(unsigned count);
};
template<class T>
class vector : public array {
public:
	constexpr vector() : array(sizeof(T)) {}
	~vector() { for(auto& e : *this) e.~T(); }
	constexpr T& operator[](int index) { return ((T*)data)[index]; }
	constexpr const T& operator[](int index) const { return ((T*)data)[index]; }
	constexpr explicit operator bool() const { return count != 0; }
	constexpr operator aref<T>() const { return aref<T>((T*)data, count); }
	T*								add() { return (T*)array::add(); }
	void							add(const T& v) { *((T*)array::add()) = v; }
	constexpr const T*				begin() const { return (T*)data; }
	constexpr T*					begin() { return (T*)data; }
	constexpr const T*				end() const { return (T*)data + count; }
	constexpr T*					end() { return (T*)data + count; }
	constexpr int					indexof(const T* e) const { if(e >= data && e < data + count) return e - data; return -1; }
	constexpr int					indexof(const T t) const { for(auto& e : *this) if(e == t) return &e - (T*)data; return -1; }
	constexpr T*					ptr(int index) const { return (T*)data + index; }
};
// Abstract data access class
template<typename T> struct bsdata {
	static T						elements[];
	static array					source;
	static constexpr array*			source_ptr = &source;
	//
	static T*						add() { return (T*)source.add(); }
	static constexpr T*				begin() { return elements; }
	static constexpr T*				end() { return elements + source.getcount(); }
};
template<> struct bsdata<int> { static constexpr array* source_ptr = 0; };
NOBSDATA(unsigned)
NOBSDATA(short)
NOBSDATA(unsigned short)
NOBSDATA(char)
NOBSDATA(unsigned char)
NOBSDATA(const char*)
NOBSDATA(bool)
// Callback function of status probing. Return true if `object` support `index` status.
typedef bool(*fnallow)(const void* object, int index);
// Callback function of choosing one element from array of many elements and storing it into `pointer`
typedef bool(*fnchoose)(const void* object, array& source, void* pointer);
// Callback function of checking some functionality of `object`
typedef bool(*fnvisible)(const void* object);
// Callback funtion of object command executing
typedef void(*fncommand)(void* object);
// Callback function of source identification. Return property filled 'source'.
typedef void(*fnsource)(const void* object, array& source);
// Abstract serializer
struct serializer {
	enum type_s { Text, Number, Array, Struct };
	struct node {
		node* parent;
		const char*					name;
		type_s						type;
		int							index;
		void*						data; // application defined data
		bool						skip; // set this if you want skip block
		//
		constexpr node(type_s type = Text) : parent(0), name(""), type(type), index(0), data(0), skip(false) {}
		constexpr node(node& parent, const char* name = "", type_s type = Text) : parent(&parent), name(name), type(type), index(0), data(0), skip(false) {}
		bool				operator==(const char* name) const { return name && strcmp(this->name, name) == 0; }
		//
		bool						isparent(const char* id) const { return parent && *parent == id; }
	};
	struct reader {
		virtual void				open(node& e) = 0;
		virtual void				set(node& e, const char* value) = 0;
		virtual void				close(node& e) = 0;
	};
	virtual ~serializer() {}
	virtual void					open(const char* id, type_s type = Text) = 0;
	virtual void					set(const char* id, int v, type_s type = Number) = 0;
	virtual void					set(const char* id, const char* v, type_s type = Text) = 0;
	virtual void					close(const char* id, type_s type = Text) = 0;
};
// Get base type
template<class T> struct meta_decoy { typedef T value; };
template<> struct meta_decoy<const char*> { typedef const char* value; };
template<class T> struct meta_decoy<T*> : meta_decoy<T> {};
template<class T> struct meta_decoy<const T*> : meta_decoy<T> {};
template<class T, unsigned N> struct meta_decoy<T[N]> : meta_decoy<T> {};
template<class T> struct meta_decoy<T[]> : meta_decoy<T> {};
template<class T> struct meta_decoy<const T> : meta_decoy<T> {};
template<class T> struct meta_decoy<std::initializer_list<T>> : meta_decoy<T> {};
template<class T> struct meta_decoy<vector<T>> : meta_decoy<T> {};
template<class T, unsigned N> struct meta_decoy<adat<T, N>> : meta_decoy<T> {};
template<class T, class DT> struct meta_decoy<cflags<T, DT>> : meta_decoy<T> {};