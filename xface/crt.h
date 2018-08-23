#pragma once

#define xva_start(v) ((const char*)&v + sizeof(v))
#ifdef _DEBUG
#define assert(e) if(!(e)) {exit(255);}
#else
#define assert(e)
#endif
#define assert_enum(e, last) static_assert(sizeof(e##_data) / sizeof(e##_data[0]) == last + 1, "Invalid count of " #e " elements")
#define getstr_enum(e) template<> const char* getstr<e##_s>(e##_s value) { return e##_data[value].name; }
#define maptbl(t, id) (t[imax((unsigned)0, imin((unsigned)id, (sizeof(t)/sizeof(t[0])-1)))])
#define maprnd(t) t[rand()%(sizeof(t)/sizeof(t[0]))]
#define lenghtof(t) (sizeof(t)/sizeof(t[0]))
#define zendof(t) (t + sizeof(t)/sizeof(t[0]) - 1)

extern "C" void						abort();
extern "C" int						atexit(void(*func)(void));
extern "C" void*					bsearch(const void* key, const void *base, unsigned num, unsigned size, int(*compar)(const void *, const void *));
extern "C" unsigned					clock(); // Returns the processor time consumed by the program.
extern "C" void						exit(int exit_code);
extern "C" int						memcmp(const void* p1, const void* p2, unsigned size);
extern "C" void*					memmove(void* destination, const void* source, unsigned size);
extern "C" void*					memcpy(void* destination, const void* source, unsigned size);
extern "C" void*					memset(void* destination, int value, unsigned size);
extern "C" void						qsort(void* base, unsigned num, unsigned size, int(*compar)(const void*, const void*));
extern "C" int						rand(void); // Get next random value
extern "C" void						sleep(unsigned seconds); // Suspend execution for an interval of time
extern "C" void						srand(unsigned seed); // Set random seed
extern "C" int						strcmp(const char* s1, const char* s2); // Compare two strings
extern "C" int						system(const char* command); // Execute system command
extern "C" long long				time(long long* seconds);

enum codepages { CPNONE, CP1251, CPUTF8, CPU16BE, CPU16LE };
enum crterrors {
	NoCrtError,
	ErrorFileNotFound, ErrorImagePluginNotFound, ErrorCompressPluginNotFound,
	ErrorInvalidInputParameters, ErrorIllegalByteSequence,
};
namespace metrics {
const codepages					code = CP1251;
}
//
int									getdigitscount(unsigned number); // Get digits count of number. For example if number=100, result be 3.
inline int							gethour(unsigned d) { return (d / 60) % 24; } // Get hour
inline int							getminute(unsigned d) { return (d % 60); } // Get minute
template<class T> const char*		getstr(T e); // Template to return string of small class
bool								ischa(unsigned u); // is alphabetical character?
bool								issignature(const char name[4], const char* s); // Is name equal to string s?
inline bool							isnum(unsigned u) { return u >= '0' && u <= '9'; } // is numeric character?
void*								loadb(const char* url, int* size = 0, int additional_bytes_alloated = 0); // Load binary file.
char*								loadt(const char* url, int* size = 0); // Load text file and decode it to system codepage.
bool								matchuc(const char* name, const char* filter);
void								printc(const char* format, ...); // Analog prinf() but use format of this library
void								printcnf(const char* text); // Write to console this text
const char*							psidn(const char* p, char* result, char* result_end); // safe parse identifier name from string
inline const char*					psidn(const char* p, char* result) { return psidn(p, result, result + 128); }
const char*							psnum(const char* p, int& value); // Parse number from string
const char*							psstr(const char* p, char* value, char end_symbol = '\"'); // Parse string from string (like c format "Some\nstring")
void								setsignature(char name[4], const char* s); // Set signature to name
float								sqrt(const float x); // Return aquare root of 'x'
void								szadd(char* result, const char* value, const char* title = "(", const char* separator = ", ");
int									sz2num(const char* p1, const char** pp1 = 0);
void								szencode(char* output, int output_count, codepages output_code, const char* input, int input_count, codepages input_code);
unsigned							szget(const char** input, codepages page = metrics::code);
int									szcmpi(const char* p1, const char* p2);
int									szcmpi(const char* p1, const char* p2, int count);
const char*							szdup(const char *text);
const char*							szext(const char* path);
const char*							szfname(const char* text); // Get file name from string (no fail, always return valid value)
char*								szfnamewe(char* result, const char* name); // get file name without extension (no fail)
const char*							szline(const char* start_text, int line_number);
const char*							szline(const char* start_text, int line_number, int column_number);
int									szline(const char* start_text, const char* position);
const char*							szlineb(const char* start_text, const char* position);
const char*							szlinee(const char* position);
unsigned							szlower(unsigned u); // to lower reg
void								szlower(char* p, int count = 1); // to lower reg
bool								szmatch(const char* text, const char* name); //
char*								sznum(char* result, int num, int precision = 0, const char* empthy = 0, int radix = 10);
char*								sznum(char* result, float f, int precision = 0, const char* empthy = "0.00");
bool								szpmatch(const char* text, const char* pattern);
char*					            szprints(char* result, const char* result_maximum, const char* format, ...);
char*								szprintvs(char* result, const char* result_maximum, const char* format, const char* vl);
void								szput(char** output, unsigned u, codepages page = metrics::code);
char*								szput(char* output, unsigned u, codepages page = metrics::code); // Fast symbol put function. Return 'output'.
char*								szsep(char* result, const char* sep = ", ", const char* start = 0);
const char*							szskipcr(const char* p);
const char*							szskipcrr(const char* p0, const char* p);
const char*							sztag(const char* p);
unsigned							szupper(unsigned u);
char*								szupper(char* p, int count = 1); // to upper reg
char*								szurl(char* p, const char* path, const char* name, const char* ext = 0, const char* suffix = 0);
char*								szurlc(char* p1);
inline int							xrand(int n1, int n2) { return n1 + rand() % (n2 - n1 + 1); }
inline int							xroll(int c, int d, int b = 0) { while(c--) b += 1 + (rand() % d); return b; }
// Untility structures
template<int v> struct static_value { constexpr static int value = v; };
// Common used templates
inline int							ifloor(double n) { return (int)n; }
template<class T> inline T			imax(T a, T b) { return a > b ? a : b; }
template<class T> inline T			imin(T a, T b) { return a < b ? a : b; }
template<class T> inline T			iabs(T a) { return a > 0 ? a : -a; }
template<class T> inline void		iswap(T& a, T& b) { T i = a; a = b; b = i; }
// Inline sequence functions
template<class T> inline void		seqclear(T* p) { T* z = p->next; while(z) { T* n = z->next; z->next = 0; delete z; z = n; } p->next = 0; } // Use to clean up sequenced resources if destructor. Use it like 'clear(this)'.
template<class T, class Z> inline const T* seqchr(const T* p, const Z id) { while(p->id) { if(p->id == id) return p; p++; } return 0; }
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
template<class T> inline int		znormalize(T* pb, T* pe) { T* p0 = pb; T* ps = pb; while(pb < pe) { if(*pb) *ps++ = *pb; pb++; } if(ps < pe) *ps = 0; return ps - p0; }
template<class T> inline int		zindex(const T* p, T e) { if(!p) return -1; const T* p1 = p; while(*p) { if(*p++ == e) return p - p1 - 1; } return -1; }
template<class T> inline const T*	zfindi(const T* p, int id) { while(p->id) { if(p->id == id) return p; p++; } return 0; }
template<class T> inline int		zfind(const T* p, T e) { if(!p) return -1; const T* p1 = p; while(*p) { if(*p++ == e) return p - p1 - 1; } return -1; }
template<class T> inline const T*	zright(const T* p, int count) { auto e = zend(p) - count; return (e < p) ? p : e; }
template<class T> inline void		zshuffle(T* p, int count) { for(int i = 0; i < count; i++) iswap(p[i], p[rand() % count]); }
template<class T> inline T*			zskipsp(T* p) { if(p) while(*p == 32 || *p == 9) p++; return p; }
template<class T> inline T*			zskipspcr(T* p) { if(p) while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++; return p; }
// Inline other function
inline int							d100() { return rand() % 100; }
inline int							d10() { return 1 + (rand() % 10); }
inline int							d20() { return 1 + (rand() % 20); }