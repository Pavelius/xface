#include "main.h"

static short unsigned	global_counter;
static unsigned	char	current_session;

void timestamp::write() {
	if(create_date)
		return;
	create_date = datetime::now();
	counter = ++global_counter;
	session = current_session;
}

void timestamp::setsession(unsigned char v) {
	current_session = v;
}