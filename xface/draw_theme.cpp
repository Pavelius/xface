#include "draw.h"

void set_dark_theme() {
	colors::active = color::create(172, 128, 0);
	colors::border = color::create(73, 73, 80);
	colors::button = color::create(0, 122, 204);
	colors::form = color::create(64, 64, 64);
	colors::window = color::create(32, 32, 32);
	colors::text = color::create(255, 255, 255);
	colors::edit = color::create(38, 79, 120);
	colors::h1 = colors::text.mix(colors::edit, 64);
	colors::h2 = colors::text.mix(colors::edit, 96);
	colors::h3 = colors::text.mix(colors::edit, 128);
	colors::special = color::create(255, 244, 32);
	colors::border = colors::window.mix(colors::text, 128);
	colors::tips::text = color::create(255, 255, 255);
	colors::tips::back = color::create(100, 100, 120);
	colors::tabs::back = color::create(255, 204, 0);
	colors::tabs::text = colors::black;
}

void set_light_theme() {
	colors::active = color::create(0, 128, 172);
	colors::button = color::create(223, 223, 223);
	colors::form = color::create(240, 240, 240);
	colors::window = color::create(255, 255, 255);
	colors::text = color::create(0, 0, 0);
	colors::edit = color::create(173, 214, 255);
	colors::h1 = colors::text.mix(colors::edit, 64);
	colors::h2 = colors::text.mix(colors::edit, 96);
	colors::h3 = colors::text.mix(colors::edit, 128);
	colors::special = color::create(0, 0, 255);
	colors::border = color::create(172, 172, 172);
	colors::tips::text = color::create(255, 255, 255);
	colors::tips::back = color::create(80, 80, 120);
	colors::tabs::back = color::create(0, 122, 204);
	colors::tabs::text = color::create(255, 255, 255);
}