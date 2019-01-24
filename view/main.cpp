#include "xface/crt.h"
#include "xface/draw.h"
#include "xface/draw_control.h"
#include "xface/io.h"

using namespace draw;

void set_light_theme();

namespace setting {
namespace show {
bool		offset;
bool		center = true;
bool		grid;
bool		mirror;
namespace text {
bool	bottom;
}
}
}

static void enter_number(int& value) {
	char temp[32];
	rect rc = {getwidth() - 100, 4, getwidth() - 4, 4 + texth() + 8};
	szprint(temp, zendof(temp), "%1i", value);
	controls::textedit te(temp, sizeof(temp), true);
	if(!te.editing(rc))
		return;
	value = sz2num(temp);
}

char* szpne(char* result, const char* url, const char* name, const char* ext) {
	const char* p1 = szfname(url);
	if(p1 == url)
		result[0] = 0;
	else
		zcpy(result, url, p1 - url);
	zcat(result, name);
	zcat(result, ".");
	zcat(result, ext);
	return result;
}

unsigned char* load_pallette(const char* url) {
	char temp[260];
	char temp2[260];
	void* result;
	result = loadb(szpne(temp, url, "main", "pal"));
	if(!result)
		result = loadb(szpne(temp, url, szfnamewe(temp2, url), "pal"));
	return (unsigned char*)result;
}

void mainview(const char* url) {
	unsigned char* pal = 0;
	sprite* pi = (sprite*)loadb(url);
	if(!pi) {
		char temp[512];
		szprint(temp, "File not found:\n\n%1", url);
		//dlgmsg("X-view", temp);
		return;
	}
	int wx = 640;
	int wy = 480;
	const sprite::frame& f1 = pi->get(0);
	bool background_image = (f1.encode == sprite::RAW || f1.encode == sprite::RAW8) && f1.ox == 0 && f1.oy == 0;
	if(background_image) {
		wx = f1.sx;
		wy = f1.sy;
		setting::show::center = false;
	}
	draw::create(-1, -1, wx, wy, WFResize | WFMinmax, 32);
	draw::initialize();
	draw::palt = (color*)load_pallette(url);
	settimer(100);
	setcaption(szfname(url));
	int current = 0;
	int tick = 0;
	char temp[128];
	bool animated_sprite = pi->cicles != 0;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::form);
		auto width = draw::getwidth();
		auto height = draw::getheight();
		int maximum = animated_sprite ? pi->cicles : pi->count;
		if(current >= maximum)
			current = maximum - 1;
		if(current < 0)
			current = 0;
		if(setting::show::grid) {
			const int grid_size = 10;
			const color grid_color = colors::form.darken();
			const color grid_bold = colors::form.darken().darken();
			for(int x = 0; x < width; x += grid_size)
				line(x, 0, x, height, (x % (grid_size * 10)) == 0 ? grid_bold : grid_color);
			for(int y = 0; y < height; y += grid_size)
				line(0, y, width, y, (y % (grid_size * 10)) == 0 ? grid_bold : grid_color);
		}
		int x = width / 2;
		int y = height / 2;
		int current_frame = animated_sprite ? pi->ganim(current, tick) : current;
		const sprite::frame& f = pi->get(current_frame);
		int sx = f.ox;
		int sy = f.oy;
		int cx = f.sx;
		int cy = f.sy;
		unsigned flags_image = 0;
		if(setting::show::mirror)
			flags_image = ImageMirrorH;
		if(setting::show::center)
			image(x, y, pi, current_frame, flags_image);
		else
			image(0, 0, pi, current_frame, ImageNoOffset | flags_image);
		if(setting::show::offset) {
			line(x, y, x - sx, y, colors::red);
			line(x, y, x, y - sy, colors::red);
		}
		szprint(temp, zendof(temp), "[%2i/%3i](%4i,%5i, size=%6ix%7i)", szfname(url), current, maximum, sx, sy, cx, cy);
		if(setting::show::center)
			zcat(temp, " center");
		if(setting::show::text::bottom)
			text(4, draw::getheight() - draw::texth() - 4, temp);
		else
			text(4, 4, temp);
		domodal();
		switch(hot.key) {
		case KeyRight:
			current++;
			break;
		case KeyLeft:
			current--;
			break;
		case Alpha + 'B':
			setting::show::offset = !setting::show::offset;
			break;
		case Alpha + 'C':
			setting::show::center = !setting::show::center;
			break;
		case Alpha + 'M':
			setting::show::mirror = !setting::show::mirror;
			break;
		case Alpha + 'G':
			setting::show::grid = !setting::show::grid;
			break;
		case Ctrl + Alpha + 'G':
			enter_number(current);
			break;
		case Alpha + 'N':
			setting::show::text::bottom = !setting::show::text::bottom;
			break;
		case InputTimer:
			tick++;
			break;
		default:
			break;
		}
	}
}

const char* sznext(const char* p) {
	if(*p == '\"') {
		p++;
		while(*p != '\"')
			p++;
		p++;
	} else {
		while(*p && (*p != ' ' && *p != '\t' && *p != 10 && *p != 13))
			p++;
	}
	return p;
}

static void correct_font() {
	char furl[260];
	char temp[260];
	if(metrics::font)
		return;
	if(!io::file::getmodule(temp, sizeof(temp) / sizeof(temp[0])))
		return;
	auto pc = (char*)szfname(temp);
	if(pc > temp && pc[-1] == '/' || pc[-1] == '\\')
		pc[-1] = 0;
	else
		pc[0] = 0;
	metrics::font = (sprite*)loadb(szurl(furl, temp, "font", "pma"));
}

static void test_directory() {
	char temp[260]; io::file::getdir(temp, sizeof(temp));
	if(temp[0])
		return;
}

int main(int argc, char *argv[]) {
	//test_directory();
	correct_font();
	set_light_theme();
	if(argc > 1 && argv[1][0])
		return mainview(argv[1]);
	else {
		//dlgmsg("PMA view", "Show sprites in PMA format. Pass to command line path to file wich you want to see.")
	}
	return 0;
}

char** szcmdargv(int& argc);

int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	int argc;
	char** argv = szcmdargv(argc);
	return main(argc, argv);
}