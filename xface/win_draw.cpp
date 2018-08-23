#include "crt.h"
#include "draw.h"
#include "win.h"

#pragma pack(push)
#pragma pack(1)
static struct video_8t {
	BITMAPINFO		bmp;
	unsigned char	bmp_pallette[256 * 4];
} video_descriptor;
#pragma pack(pop)

static HWND			hwnd;
static point		minimum;
extern bool			sys_optimize_mouse_move;
extern rect			sys_static_area;
static bool			use_mouse = true;

static int tokey(int vk) {
	switch(vk) {
	case VK_CONTROL: return Ctrl;
	case VK_MENU: return Alt;
	case VK_SHIFT: return Shift;
	case VK_LEFT: return KeyLeft;
	case VK_RIGHT: return KeyRight;
	case VK_UP: return KeyUp;
	case VK_DOWN: return KeyDown;
	case VK_PRIOR: return KeyPageUp;
	case VK_NEXT: return KeyPageDown;
	case VK_HOME: return KeyHome;
	case VK_END: return KeyEnd;
	case VK_BACK: return KeyBackspace;
	case VK_DELETE: return KeyDelete;
	case VK_RETURN: return KeyEnter;
	case VK_ESCAPE: return KeyEscape;
	case VK_SPACE: return KeySpace;
	case VK_TAB: return KeyTab;
	case VK_F1: return F1;
	case VK_F2: return F2;
	case VK_F3: return F3;
	case VK_F4: return F4;
	case VK_F5: return F5;
	case VK_F6: return F6;
	case VK_F7: return F7;
	case VK_F8: return F8;
	case VK_F9: return F9;
	case VK_F10: return F10;
	case VK_F11: return F11;
	case VK_F12: return F12;
	case VK_MULTIPLY: return Alpha + (unsigned)'*';
	case VK_DIVIDE: return Alpha + (unsigned)'/';
	case VK_ADD: return Alpha + (unsigned)'+';
	case VK_SUBTRACT: return Alpha + (unsigned)'-';
	case VK_OEM_COMMA: return Alpha + (unsigned)',';
	case VK_OEM_PERIOD: return Alpha + (unsigned)'.';
	default: return Alpha + vk;
	}
}

static void set_cursor(cursors e) {
	static void* data[] =
	{
		LoadCursorA(0, (char*)32512),//IDC_ARROW
		LoadCursorA(0, (char*)32649),//IDC_HAND
		LoadCursorA(0, (char*)32644),//IDC_SIZEWE
		LoadCursorA(0, (char*)32645),//IDC_SIZENS
		LoadCursorA(0, (char*)32646),//IDC_SIZEALL
		LoadCursorA(0, (char*)32648),//IDC_NO
		LoadCursorA(0, (char*)32513),//IDC_IBEAM
		LoadCursorA(0, (char*)32514),//IDC_WAIT
	};
	SetCursor(data[e]);
}

static int handle(MSG& msg) {
	POINT pt;
	TRACKMOUSEEVENT tm;
	switch(msg.message) {
	case WM_MOUSEMOVE:
		if(msg.hwnd != hwnd)
			break;
		if(!use_mouse)
			break;
		memset(&tm, 0, sizeof(tm));
		tm.cbSize = sizeof(tm);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hwnd;
		TrackMouseEvent(&tm);
		hot::mouse.x = LOWORD(msg.lParam);
		hot::mouse.y = HIWORD(msg.lParam);
		if(draw::mouseinput && sys_optimize_mouse_move && !draw::drag::active()) {
			if(hot::mouse.in(sys_static_area))
				return InputNoUpdate;
		}
		return MouseMove;
	case WM_MOUSELEAVE:
		if(msg.hwnd != hwnd)
			break;
		if(!use_mouse)
			break;
		GetCursorPos(&pt);
		ScreenToClient(msg.hwnd, &pt);
		hot::mouse.x = (short)pt.x;
		if(hot::mouse.x < 0)
			hot::mouse.x = -10000;
		hot::mouse.y = (short)pt.y;
		if(hot::mouse.y < 0)
			hot::mouse.y = -10000;
		return MouseMove;
	case WM_LBUTTONDOWN:
		if(msg.hwnd != hwnd)
			break;
		if(!use_mouse)
			break;
		hot::pressed = true;
		return MouseLeft;
	case WM_LBUTTONDBLCLK:
		if(msg.hwnd != hwnd)
			break;
		if(!use_mouse)
			break;
		hot::pressed = true;
		return MouseLeftDBL;
	case WM_LBUTTONUP:
		if(msg.hwnd != hwnd)
			break;
		if(!use_mouse)
			break;
		if(!hot::pressed)
			break;
		hot::pressed = false;
		return MouseLeft;
	case WM_RBUTTONDOWN:
		if(!use_mouse)
			break;
		hot::pressed = true;
		return MouseRight;
	case WM_RBUTTONUP:
		if(!use_mouse)
			break;
		hot::pressed = false;
		return MouseRight;
	case WM_MOUSEWHEEL:
		if(!use_mouse)
			break;
		if(msg.wParam & 0x80000000)
			return MouseWheelDown;
		else
			return MouseWheelUp;
		break;
	case WM_TIMER:
		if(msg.hwnd != hwnd)
			break;
		if(msg.wParam == InputTimer)
			return InputTimer;
		break;
	case WM_KEYDOWN:
		return tokey(msg.wParam);
	case WM_CHAR:
		hot::param = msg.wParam;
		return InputSymbol;
	case WM_MY_SIZE:
	case WM_SIZE:
		return InputUpdate;
	}
	return 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
	MSG msg;
	switch(uMsg) {
	case WM_ERASEBKGND:
		if(draw::canvas) {
			RECT rc; GetClientRect(hwnd, &rc);
			video_descriptor.bmp.bmiHeader.biSize = sizeof(video_descriptor.bmp.bmiHeader);
			video_descriptor.bmp.bmiHeader.biWidth = draw::canvas->width;
			video_descriptor.bmp.bmiHeader.biHeight = -draw::canvas->height;
			video_descriptor.bmp.bmiHeader.biBitCount = draw::canvas->bpp;
			video_descriptor.bmp.bmiHeader.biPlanes = 1;
			if(rc.right != draw::canvas->width || rc.bottom != draw::canvas->height)
				StretchDIBits((void*)wParam,
					0, 0, rc.right, rc.bottom,
					0, 0, draw::canvas->width, draw::canvas->height,
					draw::canvas->bits, &video_descriptor.bmp, DIB_RGB_COLORS, SRCCOPY);
			else
				SetDIBitsToDevice((void*)wParam,
					0, 0, rc.right, rc.bottom,
					0, 0, 0, draw::canvas->height,
					draw::canvas->bits, &video_descriptor.bmp, DIB_RGB_COLORS);
		}
		return 1;
	case WM_CLOSE:
		PostQuitMessage(-1);
		return 0;
	case WM_EXITSIZEMOVE:
	case WM_SIZE:
		if(!PeekMessageA(&msg, hwnd, WM_MY_SIZE, WM_MY_SIZE, 0))
			PostMessageA(hwnd, WM_MY_SIZE, 0, 0);
		return 0;
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = minimum.x;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = minimum.y;
		return 0;
	case WM_SETCURSOR:
		if(LOWORD(lParam) == HTCLIENT) {
			set_cursor(hot::cursor);
			return 1;
		}
		break;
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static const char* register_class(const char* class_name) {
	WNDCLASS wc;
	if(!GetClassInfoA(GetModuleHandleA(0), class_name, &wc)) {
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; // Own DC For Window.
		wc.lpfnWndProc = WndProc;	// WndProc Handles Messages
		wc.hInstance = GetModuleHandleA(0);	// Set The Instance
		wc.hIcon = (void*)LoadIconA(wc.hInstance, (const char*)1); // WndProc Handles Messages
		wc.lpszClassName = class_name; // Set The Class Name
		RegisterClassA(&wc); // Attempt To Register The Window Class
	}
	return class_name;
}

void draw::updatewindow() {
	if(!hwnd)
		return;
	if(!IsWindowVisible(hwnd))
		ShowWindow(hwnd, SW_SHOW);
	InvalidateRect(hwnd, 0, 1);
	UpdateWindow(hwnd);
}

void draw::syscursor(bool enable) {
	ShowCursor(enable ? 1 : 0);
}

void draw::create(int x, int y, int width, int height, unsigned flags, int bpp) {
	if(!bpp)
		bpp = draw::canvas->bpp;
	if(!width)
		width = (GetSystemMetrics(SM_CXFULLSCREEN) / 3) * 2;
	if(!height)
		height = (GetSystemMetrics(SM_CYFULLSCREEN) / 3) * 2;
	// custom
	unsigned dwStyle = WS_CAPTION | WS_VISIBLE | WS_SYSMENU; // Windows Style;
	if(flags&WFResize)
		dwStyle |= WS_THICKFRAME;
	else
		dwStyle |= WS_BORDER;
	if(flags&WFMinmax) {
		dwStyle |= WS_MINIMIZEBOX;
		if(flags&WFResize)
			dwStyle |= WS_MAXIMIZEBOX;
	}
	if(flags&WFMaximized)
		dwStyle |= WS_MAXIMIZE;
	RECT MinimumRect = {0, 0, width, height};
	AdjustWindowRectEx(&MinimumRect, dwStyle, 0, 0);
	minimum.x = (short)(MinimumRect.right - MinimumRect.left);
	minimum.y = (short)(MinimumRect.bottom - MinimumRect.top);
	if(x == -1)
		x = (GetSystemMetrics(SM_CXFULLSCREEN) - minimum.x) / 2;
	if(y == -1)
		y = (GetSystemMetrics(SM_CYFULLSCREEN) - minimum.y) / 2;
	rect position;
	position.x1 = x;
	position.y1 = y;
	position.x2 = position.x1 + minimum.x;
	position.y2 = position.y1 + minimum.y;
	// Update current surface
	if(draw::canvas)
		draw::canvas->resize(width, height, bpp, true);
	setclip();
	// Create The Window
	hwnd = CreateWindowExA(0, register_class("CFaceWindow"), 0, dwStyle,
		position.x1, position.y1,
		position.width(), position.height(),
		0, 0, GetModuleHandleA(0), 0);
	if(!hwnd)
		return;
	// Update mouse coordinates
	POINT pt; GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);
	hot::mouse.x = (short)pt.x;
	hot::mouse.y = (short)pt.y;
}

void draw::sysredraw() {
	MSG	msg;
	updatewindow();
	if(!hwnd)
		return;
	while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		handle(msg);
	}
}

int draw::rawinput() {
	MSG	msg;
	updatewindow();
	if(!hwnd)
		return 0;
	while(GetMessageA(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		unsigned m = handle(msg);
		if(m == InputNoUpdate)
			continue;
		if(m) {
			if(m != MouseMove && m >= (unsigned)MouseLeft) {
				if(GetKeyState(VK_SHIFT) < 0)
					m |= Shift;
				if(GetKeyState(VK_MENU) < 0)
					m |= Alt;
				if(GetKeyState(VK_CONTROL) < 0)
					m |= Ctrl;
			}
			if(m == InputUpdate) {
				if(canvas) {
					RECT rc; GetClientRect(hwnd, &rc);
					canvas->resize(rc.right - rc.left, rc.bottom - rc.top, 32, true);
					setclip();
				}
			}
			return m;
		}
	}
	return 0;
}

void draw::setcaption(const char* string) {
	SetWindowTextA(hwnd, string);
}

void draw::settimer(unsigned milleseconds) {
	if(milleseconds)
		SetTimer(hwnd, InputTimer, milleseconds, 0);
	else
		KillTimer(hwnd, InputTimer);
}