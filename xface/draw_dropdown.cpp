#include "draw_control.h"
#include "screenshoot.h"

bool draw::dropdown(const rect& rc, draw::controls::control& e) {
	pushfocus pf;
	screenshoot screen;
	setfocus(e, true);
	while(ismodal()) {
		screen.restore();
		e.view(rc);
		domodal();
		switch(hot.key) {
		case KeyEscape:
			breakmodal(0);
			hot.zero();
			break;
		case KeyTab:
		case KeyTab | Shift:
			breakmodal(0);
			break;
		case MouseLeft:
		case MouseLeft + Ctrl:
		case MouseLeft + Shift:
		case MouseLeftDBL:
		case MouseLeftDBL + Ctrl:
		case MouseLeftDBL + Shift:
			if(hot.pressed) {
				if(!areb(rc))
					breakmodal(0);
				else {
					// Уже обработали команду
					//hot.key = InputUpdate;
				}
			}
			break;
		}
	}
	return getresult() != 0;
}