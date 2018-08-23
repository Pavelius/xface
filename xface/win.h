#define MAKELONG(a,b)	((long)(((short unsigned)(a))|(((unsigned)((short unsigned)(b)))<<16)))
#define LOWORD(l)		((WORD)((unsigned)(l)))
#define HIWORD(l)		((WORD)(((unsigned)(l)>>16)&0xFFFF))
#define MAKEINTRESOURCE(i) (const char*)((unsigned)((WORD)(i)))

#define WINUSERAPI extern "C"
#define WINGDIAPI extern "C"
#define WINBASEAPI extern "C"
#define WINAPI __stdcall
#define CALLBACK __stdcall
#define DLL extern "C"

#define FALSE 0
#define TRUE 1

typedef long LRESULT;
typedef int LPARAM;
typedef int WPARAM;
typedef long LONG;
typedef void* HWND;
typedef void* HFONT;
typedef void* HDC;
typedef unsigned UINT;
typedef unsigned DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef LRESULT(CALLBACK *WNDPROC)(void*, unsigned, int, int);
typedef void (CALLBACK *TIMERPROC)(HWND, UINT, UINT, unsigned);
typedef unsigned	COLORREF;
typedef int(CALLBACK *BFFCALLBACK)(void*, unsigned, unsigned, unsigned);
typedef unsigned(CALLBACK *OFNHOOKPROC)(void*, unsigned, unsigned, unsigned);
typedef void(CALLBACK *PIMAGE_TLS_CALLBACK)(void* DllHandle, unsigned long Reason, void* Reserved);

#define VK_BACK		8
#define VK_TAB		9
#define VK_CLEAR	12
#define VK_RETURN	13
#define VK_SHIFT	16
#define VK_CONTROL	17
#define VK_MENU		18
#define VK_PAUSE	19
#define VK_CAPITAL	20
#define VK_KANA		0x15
#define VK_JUNJA	0x17
#define VK_FINAL	0x18
#define VK_KANJI	0x19
#define VK_ESCAPE	0x1B
#define VK_CONVERT	0x1C
#define VK_NONCONVERT	0x1D
#define VK_ACCEPT	0x1E
#define VK_MODECHANGE	0x1F
#define VK_SPACE	32
#define VK_PRIOR	33
#define VK_NEXT	34
#define VK_END	35
#define VK_HOME	36
#define VK_LEFT	37
#define VK_UP	38
#define VK_RIGHT 39
#define VK_DOWN	40
#define VK_SELECT	41
#define VK_PRINT	42
#define VK_EXECUTE	43
#define VK_SNAPSHOT	44
#define VK_INSERT	45
#define VK_DELETE	46
#define VK_HELP	47
#define VK_LWIN	0x5B
#define VK_RWIN	0x5C
#define VK_APPS	0x5D
#define VK_SLEEP	0x5F
#define VK_NUMPAD0	0x60
#define VK_NUMPAD1	0x61
#define VK_NUMPAD2	0x62
#define VK_NUMPAD3	0x63
#define VK_NUMPAD4	0x64
#define VK_NUMPAD5	0x65
#define VK_NUMPAD6	0x66
#define VK_NUMPAD7	0x67
#define VK_NUMPAD8	0x68
#define VK_NUMPAD9	0x69
#define VK_MULTIPLY	0x6A
#define VK_ADD	0x6B
#define VK_SEPARATOR	0x6C
#define VK_SUBTRACT	0x6D
#define VK_DECIMAL	0x6E
#define VK_DIVIDE	0x6F
#define VK_F1	0x70
#define VK_F2	0x71
#define VK_F3	0x72
#define VK_F4	0x73
#define VK_F5	0x74
#define VK_F6	0x75
#define VK_F7	0x76
#define VK_F8	0x77
#define VK_F9	0x78
#define VK_F10	0x79
#define VK_F11	0x7A
#define VK_F12	0x7B
#define VK_OEM_COMMA	0xBC
#define VK_OEM_MINUS	0xBD
#define VK_OEM_PERIOD	0xBE

#define MIIM_STATE 1
#define MIIM_ID 2
#define MIIM_SUBMENU 4
#define MIIM_CHECKMARKS 8
#define MIIM_TYPE 16
#define MIIM_DATA 32
#define MIIM_STRING 64
#define MIIM_BITMAP 128
#define MIIM_FTYPE 256

#define MFT_BITMAP 4
#define MFT_MENUBARBREAK 32
#define MFT_MENUBREAK 64
#define MFT_OWNERDRAW 256
#define MFT_RADIOCHECK 512
#define MFT_RIGHTJUSTIFY 0x4000
#define MFT_SEPARATOR 0x800
#define MFT_RIGHTORDER 0x2000L
#define MFT_STRING 0
#define MFS_CHECKED 8
#define MFS_DEFAULT 4096
#define MFS_DISABLED 3
#define MFS_ENABLED 0
#define MFS_GRAYED 3
#define MFS_HILITE 128
#define MFS_UNCHECKED 0
#define MFS_UNHILITE 0

#define SW_HIDE 0
#define SW_NORMAL 1
#define SW_SHOWNORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_MAXIMIZE 3
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA 8
#define SW_RESTORE 9
#define SW_SHOWDEFAULT 10
#define SW_FORCEMINIMIZE 11
#define SW_MAX 11

#define SC_SCREENSAVE 0xF140
#define SC_MONITORPOWER 0xF170

#define GMEM_FIXED 0
#define GMEM_MOVEABLE 2
#define GMEM_MODIFY 128

#define PM_NOREMOVE 0
#define PM_REMOVE 1
#define PM_NOYIELD 2

#define CF_TEXT	1
#define CF_BITMAP	2
#define CF_METAFILEPICT	3
#define CF_SYLK	4
#define CF_DIF	5
#define CF_TIFF	6
#define CF_OEMTEXT	7
#define CF_DIB	8
#define CF_PALETTE	9
#define CF_PENDATA	10
#define CF_RIFF	11
#define CF_WAVE	12
#define CF_UNICODETEXT	13
#define CF_ENHMETAFILE	14
#define CF_HDROP	15
#define CF_LOCALE	16

#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17

#define WS_BORDER	0x800000
#define WS_CAPTION	0xC00000
#define WS_CHILD	0x40000000
#define WS_CHILDWINDOW	0x40000000
#define WS_CLIPCHILDREN 0x2000000
#define WS_CLIPSIBLINGS 0x4000000
#define WS_DISABLED	0x8000000
#define WS_DLGFRAME	0x400000
#define WS_GROUP	0x20000
#define WS_HSCROLL	0x100000
#define WS_ICONIC	0x20000000
#define WS_MAXIMIZE	0x1000000
#define WS_MAXIMIZEBOX	0x10000
#define WS_MINIMIZE	0x20000000
#define WS_MINIMIZEBOX	0x20000
#define WS_OVERLAPPED	0
#define WS_OVERLAPPEDWINDOW	0xcf0000
#define WS_POPUP	0x80000000
#define WS_POPUPWINDOW	0x80880000
#define WS_SIZEBOX	0x40000
#define WS_SYSMENU	0x80000
#define WS_TABSTOP	0x10000
#define WS_THICKFRAME	0x40000
#define WS_TILED	0
#define WS_TILEDWINDOW	0xcf0000
#define WS_VISIBLE	0x10000000
#define WS_VSCROLL	0x200000

#define WS_EX_ACCEPTFILES 16
#define WS_EX_APPWINDOW	0x40000
#define WS_EX_CLIENTEDGE 512
#define WS_EX_COMPOSITED 0x2000000 /* XP */
#define WS_EX_CONTEXTHELP 0x400
#define WS_EX_CONTROLPARENT 0x10000
#define WS_EX_DLGMODALFRAME 1
#define WS_EX_LAYERED 0x80000   /* w2k */
#define WS_EX_LAYOUTRTL 0x400000 /* w98, w2k */
#define WS_EX_LEFT	0
#define WS_EX_LEFTSCROLLBAR	0x4000
#define WS_EX_LTRREADING	0
#define WS_EX_MDICHILD	64
#define WS_EX_NOACTIVATE 0x8000000 /* w2k */
#define WS_EX_NOINHERITLAYOUT 0x100000 /* w2k */
#define WS_EX_NOPARENTNOTIFY	4
#define WS_EX_OVERLAPPEDWINDOW	0x300
#define WS_EX_PALETTEWINDOW	0x188
#define WS_EX_RIGHT	0x1000
#define WS_EX_RIGHTSCROLLBAR	0
#define WS_EX_RTLREADING	0x2000
#define WS_EX_STATICEDGE	0x20000
#define WS_EX_TOOLWINDOW	128
#define WS_EX_TOPMOST	8
#define WS_EX_TRANSPARENT	32
#define WS_EX_WINDOWEDGE	256

#define MB_OK 0
#define MB_YESNO 0x00000004L
#define MB_ICONQUESTION 0x00000020L
#define MB_USERICON 128
#define MB_ICONASTERISK 64
#define MB_ICONEXCLAMATION 0x30
#define MB_ICONWARNING 0x30
#define MB_ICONERROR 16
#define MB_ICONHAND 16
#define MB_ABORTRETRYIGNORE 2
#define MB_APPLMODAL 0
#define MB_DEFAULT_DESKTOP_ONLY 0x20000
#define MB_HELP 0x4000
#define MB_RIGHT 0x80000
#define MB_RTLREADING 0x100000
#define MB_TOPMOST 0x40000
#define MB_DEFBUTTON1 0
#define MB_DEFBUTTON2 256
#define MB_DEFBUTTON3 512
#define MB_DEFBUTTON4 0x300
#define MB_ICONINFORMATION 64
#define MB_ICONSTOP 16
#define MB_OKCANCEL 1
#define MB_RETRYCANCEL 5

#define IDYES 6

#define WM_ACTIVATE 6
#define WM_CHAR 258
#define WM_CLOSE 16
#define WM_COMMAND 273
#define WM_COPY 769
#define WM_CREATE 1
#define WM_ERASEBKGND 20
#define WM_EXITSIZEMOVE 562
#define WM_SIZE 5
#define WM_MOUSEHOVER 0x2A1
#define WM_MOUSELEAVE 0x2A3
#define WM_GETMINMAXINFO 36
#define WM_SETCURSOR 32
#define WM_SETICON 0x0080
#define WM_TIMER 275
#define WM_KEYDOWN 256
#define WM_KEYUP 257
#define WM_KILLFOCUS 8
#define WM_MOUSEMOVE 512
#define WM_LBUTTONDOWN 513
#define WM_LBUTTONUP 514
#define WM_LBUTTONDBLCLK 515
#define WM_RBUTTONDOWN 516
#define WM_RBUTTONUP 517
#define WM_RBUTTONDBLCLK 518
#define WM_MBUTTONDOWN 519
#define WM_MBUTTONUP 520
#define WM_MBUTTONDBLCLK 521
#define WM_MOUSEWHEEL 522
#define WM_QUIT 18
#define WM_SYSCOMMAND 0x0112
#define WM_USER 1024
#define WM_MY_SIZE WM_USER+1
#define BFFM_SETSTATUSTEXTA (WM_USER + 100)
#define BFFM_SETSTATUSTEXTW (WM_USER + 104)
#define BFFM_ENABLEOK (WM_USER + 101)
#define BFFM_SETSELECTIONA (WM_USER + 102)
#define BFFM_SETSELECTIONW (WM_USER + 103)
#define BFFM_SETOKTEXT (WM_USER + 105)
#define BFFM_SETEXPANDED (WM_USER + 106)

#define GWL_EXSTYLE (-20)
#define GWL_STYLE (-16)
#define GWL_WNDPROC (-4)
#define GWL_HINSTANCE (-6)
#define GWL_HWNDPARENT (-8)
#define GWL_ID (-12)
#define GWL_USERDATA (-21)

#define DIB_PAL_COLORS	1
#define DIB_RGB_COLORS	0

#define SRCCOPY		0x00CC0020
#define SRCPAINT	0x00EE0086

#define HTNOWHERE 0
#define HTCLIENT 1
#define HTCAPTION 2
#define HTSIZE 4

#define TME_HOVER	1
#define TME_LEAVE	2
#define TME_QUERY	0x40000000
#define TME_CANCEL	0x80000000
#define HOVER_DEFAULT	0xFFFFFFFF

#define STD_INPUT_HANDLE -10
#define STD_OUTPUT_HANDLE -11
#define STD_ERROR_HANDLE -12

#define OFN_ALLOWMULTISELECT 512
#define OFN_CREATEPROMPT 0x2000
#define OFN_ENABLEHOOK 32
#define OFN_ENABLESIZING 0x800000
#define OFN_ENABLETEMPLATE 64
#define OFN_ENABLETEMPLATEHANDLE 128
#define OFN_EXPLORER 0x80000
#define OFN_EXTENSIONDIFFERENT 0x400
#define OFN_FILEMUSTEXIST 0x1000
#define OFN_HIDEREADONLY 4
#define OFN_LONGNAMES 0x200000
#define OFN_NOCHANGEDIR 8
#define OFN_NODEREFERENCELINKS 0x100000
#define OFN_NOLONGNAMES 0x40000
#define OFN_NONETWORKBUTTON 0x20000
#define OFN_NOREADONLYRETURN 0x8000
#define OFN_NOTESTFILECREATE 0x10000
#define OFN_NOVALIDATE 256
#define OFN_OVERWRITEPROMPT 2
#define OFN_PATHMUSTEXIST 0x800
#define OFN_READONLY 1
#define OFN_SHAREAWARE 0x4000
#define OFN_SHOWHELP 16
#define OFN_SHAREFALLTHROUGH 2
#define OFN_SHARENOWARN 1
#define OFN_SHAREWARN 0
#define OFN_NODEREFERENCELINKS 0x100000

#define CC_RGBINIT	1
#define CC_FULLOPEN	2
#define CC_PREVENTFULLOPEN	4
#define CC_SHOWHELP	8
#define CC_ENABLEHOOK	16
#define CC_ENABLETEMPLATE	32
#define CC_ENABLETEMPLATEHANDLE	64
#define CC_SOLIDCOLOR	128
#define CC_ANYCOLOR	256

#define BFFM_INITIALIZED 1
#define BFFM_SELCHANGED 2
#define BFFM_VALIDATEFAILEDA 3
#define BFFM_VALIDATEFAILEDW 4

#define BIF_RETURNONLYFSDIRS 1
#define BIF_DONTGOBELOWDOMAIN 2
#define BIF_STATUSTEXT 4
#define BIF_RETURNFSANCESTORS 8
#define BIF_EDITBOX 16
#define BIF_VALIDATE 32
#define BIF_NEWDIALOGSTYLE 64
#define BIF_BROWSEINCLUDEURLS 128
#define BIF_USENEWUI ( BIF_EDITBOX | BIF_NEWDIALOGSTYLE )
#define BIF_BROWSEFORCOMPUTER 0x1000
#define BIF_BROWSEFORPRINTER 0x2000
#define BIF_BROWSEINCLUDEFILES 0x4000
#define BIF_SHAREABLE 0x8000

#define CS_BYTEALIGNCLIENT 4096
#define CS_BYTEALIGNWINDOW 8192
#define CS_KEYCVTWINDOW 4
#define CS_NOKEYCVT 256
#define CS_CLASSDC 64
#define CS_DBLCLKS 8
#define CS_GLOBALCLASS 16384
#define CS_HREDRAW 2
#define CS_NOCLOSE 512
#define CS_OWNDC 32
#define CS_PARENTDC 128
#define CS_SAVEBITS 2048
#define CS_VREDRAW 1

#define MF_ENABLED	0
#define MF_GRAYED	1
#define MF_DISABLED	2
#define MF_BITMAP	4
#define MF_CHECKED	8
#define MF_MENUBARBREAK 32
#define MF_MENUBREAK	64
#define MF_OWNERDRAW	256
#define MF_POPUP	16
#define MF_SEPARATOR	0x800
#define MF_STRING	0
#define MF_UNCHECKED	0
#define MF_DEFAULT	4096
#define MF_SYSMENU	0x2000
#define MF_HELP	0x4000
#define MF_END	128
#define MF_RIGHTJUSTIFY	0x4000
#define MF_MOUSESELECT	0x8000
#define MF_INSERT 0
#define MF_CHANGE 128
#define MF_APPEND 256
#define MF_DELETE 512
#define MF_REMOVE 4096
#define MF_USECHECKBITMAPS 512
#define MF_UNHILITE 0
#define MF_HILITE 128

#define TPM_CENTERALIGN 4
#define TPM_LEFTALIGN 0
#define TPM_RIGHTALIGN 8
#define TPM_LEFTBUTTON 0
#define TPM_RIGHTBUTTON 2
#define TPM_HORIZONTAL 0
#define TPM_VERTICAL 64
#define TPM_TOPALIGN 0
#define TPM_VCENTERALIGN 16
#define TPM_BOTTOMALIGN 32
#define TPM_NONOTIFY 128
#define TPM_RETURNCMD 256

#define PFD_TYPE_RGBA	0
#define PFD_TYPE_COLORINDEX	1
#define PFD_MAIN_PLANE	0
#define PFD_OVERLAY_PLANE	1
#define PFD_UNDERLAY_PLANE	(-1)
#define PFD_DOUBLEBUFFER	1
#define PFD_STEREO	2
#define PFD_DRAW_TO_WINDOW	4
#define PFD_DRAW_TO_BITMAP	8
#define PFD_SUPPORT_GDI	16
#define PFD_SUPPORT_OPENGL	32
#define PFD_GENERIC_FORMAT	64
#define PFD_NEED_PALETTE	128
#define PFD_NEED_SYSTEM_PALETTE	0x00000100
#define PFD_SWAP_EXCHANGE	0x00000200
#define PFD_SWAP_COPY	0x00000400
#define PFD_SWAP_LAYER_BUFFERS	0x00000800
#define PFD_GENERIC_ACCELERATED	0x00001000
#define PFD_DEPTH_DONTCARE	0x20000000
#define PFD_DOUBLEBUFFER_DONTCARE	0x40000000
#define PFD_STEREO_DONTCARE	0x80000000

#define IDI_APPLICATION MAKEINTRESOURCE(32512)
#define IDI_HAND MAKEINTRESOURCE(32513)
#define IDI_QUESTION MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION MAKEINTRESOURCE(32515)
#define IDI_ASTERISK MAKEINTRESOURCE(32516)
#define IDI_WINLOGO MAKEINTRESOURCE(32517)

#define IDC_ARROW MAKEINTRESOURCE(32512)
#define IDC_IBEAM MAKEINTRESOURCE(32513)
#define IDC_WAIT MAKEINTRESOURCE(32514)
#define IDC_CROSS MAKEINTRESOURCE(32515)
#define IDC_UPARROW MAKEINTRESOURCE(32516)
#define IDC_SIZENWSE MAKEINTRESOURCE(32642)
#define IDC_SIZENESW MAKEINTRESOURCE(32643)
#define IDC_SIZEWE MAKEINTRESOURCE(32644)
#define IDC_SIZENS MAKEINTRESOURCE(32645)
#define IDC_SIZEALL MAKEINTRESOURCE(32646)
#define IDC_NO MAKEINTRESOURCE(32648)
#define IDC_HAND MAKEINTRESOURCE(32649)
#define IDC_APPSTARTING MAKEINTRESOURCE(32650)
#define IDC_HELP MAKEINTRESOURCE(32651)
#define IDC_ICON MAKEINTRESOURCE(32641)
#define IDC_SIZE MAKEINTRESOURCE(32640)

#define	STILL_ACTIVE 259
#define INFINITE 0xFFFFFFFF

#define GENERIC_READ              0x80000000
#define GENERIC_WRITE             0x40000000
#define FILE_SHARE_READ           0x00000001
#define FILE_SHARE_WRITE          0x00000002

#define CREATE_NEW                1
#define CREATE_ALWAYS             2
#define OPEN_EXISTING             3
#define OPEN_ALWAYS               4
#define TRUNCATE_EXISTING         5

#define FILE_ATTRIBUTE_READONLY   0x00000001  
#define FILE_ATTRIBUTE_NORMAL     0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY  0x00000100

#define FILE_BEGIN			0
#define FILE_CURRENT		1
#define FILE_END			2

#define HEAP_NO_SERIALIZE	1

#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3

#define FLS_OUT_OF_INDEXES 0xFFFFFFFF

#define FW_DONTCARE	0
#define FW_THIN	100
#define FW_EXTRALIGHT	200
#define FW_ULTRALIGHT	FW_EXTRALIGHT
#define FW_LIGHT	300
#define FW_NORMAL	400
#define FW_REGULAR	400
#define FW_MEDIUM	500
#define FW_SEMIBOLD	600
#define FW_DEMIBOLD	FW_SEMIBOLD
#define FW_BOLD	700
#define FW_EXTRABOLD	800
#define FW_ULTRABOLD	FW_EXTRABOLD
#define FW_HEAVY	900
#define FW_BLACK	FW_HEAVY

#define DEFAULT_CHARSET	1
#define CLIP_DEFAULT_PRECIS	0
#define OUT_DEFAULT_PRECIS	0
#define ANTIALIASED_QUALITY 4

#define FF_DECORATIVE	80
#define FF_DONTCARE		0
#define FF_MODERN		48
#define FF_ROMAN		16
#define FF_SCRIPT		64
#define FF_SWISS		32

#define GGO_GRAY8_BITMAP	6

#define INT_MIN     (-2147483647 - 1) // minimum (signed) int value

#define ANSI_CHARSET 0

#define INTERNET_SERVICE_FTP 1
#define INTERNET_SERVICE_GOPHER 2
#define INTERNET_SERVICE_HTTP 3

#define INTERNET_DEFAULT_FTP_PORT 21
#define INTERNET_DEFAULT_GOPHER_PORT 70
#define INTERNET_DEFAULT_HTTP_PORT 80
#define INTERNET_DEFAULT_HTTPS_PORT 443
#define INTERNET_DEFAULT_SOCKS_PORT 1080

#define INTERNET_FLAG_RELOAD 0x80000000
#define INTERNET_FLAG_RAW_DATA 0x40000000
#define INTERNET_FLAG_EXISTING_CONNECT 0x20000000
#define INTERNET_FLAG_ASYNC 0x10000000
#define INTERNET_FLAG_PASSIVE 0x08000000
#define INTERNET_FLAG_NO_CACHE_WRITE 0x04000000
#define INTERNET_FLAG_DONT_CACHE INTERNET_FLAG_NO_CACHE_WRITE
#define INTERNET_FLAG_MAKE_PERSISTENT 0x02000000
#define INTERNET_FLAG_OFFLINE 0x1000000
#define INTERNET_FLAG_SECURE 0x800000
#define INTERNET_FLAG_KEEP_CONNECTION 0x400000
#define INTERNET_FLAG_NO_AUTO_REDIRECT 0x200000
#define INTERNET_FLAG_READ_PREFETCH 0x100000
#define INTERNET_FLAG_NO_COOKIES 0x80000
#define INTERNET_FLAG_NO_AUTH 0x40000
#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP 0x8000
#define INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS 0x4000
#define INTERNET_FLAG_IGNORE_CERT_DATE_INVALID 0x2000
#define INTERNET_FLAG_IGNORE_CERT_CN_INVALID 0x1000
#define INTERNET_FLAG_MUST_CACHE_REQUEST 16
#define INTERNET_FLAG_RESYNCHRONIZE 0x800
#define INTERNET_FLAG_HYPERLINK 0x400
#define INTERNET_FLAG_NO_UI 0x200
#define INTERNET_FLAG_PRAGMA_NOCACHE 0x100
#define INTERNET_FLAG_TRANSFER_ASCII FTP_TRANSFER_TYPE_ASCII
#define INTERNET_FLAG_TRANSFER_BINARY FTP_TRANSFER_TYPE_BINARY

struct POINT {
	LONG		x;
	LONG		y;
};
struct RECT {
	LONG		left;
	LONG		top;
	LONG		right;
	LONG		bottom;
};
struct RGBQUAD {
	unsigned char	rgbBlue;
	unsigned char	rgbGreen;
	unsigned char	rgbRed;
	unsigned char	rgbReserved;
};
struct MINMAXINFO {
	POINT		ptReserved;
	POINT		ptMaxSize;
	POINT		ptMaxPosition;
	POINT		ptMinTrackSize;
	POINT		ptMaxTrackSize;
};
struct BITMAPINFOHEADER {
	unsigned	biSize;
	LONG		biWidth;
	LONG		biHeight;
	WORD		biPlanes;
	WORD		biBitCount;
	unsigned	biCompression;
	unsigned	biSizeImage;
	LONG		biXPelsPerMeter;
	LONG		biYPelsPerMeter;
	unsigned	biClrUsed;
	unsigned	biClrImportant;
};
struct BITMAPINFO {
	BITMAPINFOHEADER	bmiHeader;
	RGBQUAD		bmiColors[1];
};
struct MSG {
	HWND		hwnd;
	UINT		message;
	int			wParam;
	int			lParam;
	unsigned	time;
	POINT		pt;
};
struct CREATESTRUCT {
	void*		lpCreateParams;
	void*		hInstance;
	void*		hMenu;
	HWND		hwndParent;
	int			cy;
	int			cx;
	int			y;
	int			x;
	LONG		style;
	const char*	lpszName;
	const char*	lpszClass;
	unsigned	dwExStyle;
};
struct TRACKMOUSEEVENT {
	unsigned	cbSize;
	unsigned	dwFlags;
	HWND		hwndTrack;
	unsigned	dwHoverTime;
};
struct WNDCLASS {
	UINT		style;
	WNDPROC		lpfnWndProc;
	int			cbClsExtra;
	int			cbWndExtra;
	void*		hInstance;
	void*		hIcon;
	void*		hCursor;
	void*		hbrBackground;
	const char*	lpszMenuName;
	const char*	lpszClassName;
};
struct MENUITEMINFO {
	UINT					cbSize;
	UINT					fMask;
	UINT					fType;
	UINT					fState;
	UINT					wID;
	void*					hSubMenu;
	void*					hbmpChecked;
	void*					hbmpUnchecked;
	unsigned				dwItemData;
	char*					dwTypeData;
	UINT					cch;
	void*					hbmpItem;
};
struct TPMPARAMS {
	UINT					cbSize;
	RECT					rcExclude;
};
struct WINDOWPLACEMENT {
	UINT					length;
	UINT					flags;
	UINT					showCmd;
	POINT					ptMinPosition;
	POINT					ptMaxPosition;
	RECT					rcNormalPosition;
};
struct FILETIME {
	unsigned				dwLowDateTime;
	unsigned				dwHighDateTime;
};
struct WIN32_FIND_DATA {
	unsigned				dwFileAttributes;
	FILETIME				ftCreationTime;
	FILETIME				ftLastAccessTime;
	FILETIME				ftLastWriteTime;
	unsigned				nFileSizeHigh;
	unsigned				nFileSizeLow;
	unsigned				dwReserved0;
	unsigned				dwReserved1;
	char					cFileName[260];
	char					cAlternateFileName[14];
};
struct BROWSEINFOA {
	void*					hwndOwner;
	void*					pidlRoot;
	char*					pszDisplayName;
	const char*				lpszTitle;
	unsigned				ulFlags;
	BFFCALLBACK				lpfn;
	unsigned				lParam;
	int						iImage;
};
struct OPENFILENAME {
	unsigned				lStructSize;
	void*					hwndOwner;
	void*					hInstance;
	const char*				lpstrFilter;
	char*					lpstrCustomFilter;
	unsigned				nMaxCustFilter;
	unsigned				nFilterIndex;
	char*					lpstrFile;
	unsigned				nMaxFile;
	char*					lpstrFileTitle;
	unsigned				nMaxFileTitle;
	const char*				lpstrInitialDir;
	const char*				lpstrTitle;
	unsigned				Flags;
	short unsigned			nFileOffset;
	short unsigned			nFileExtension;
	const char*				lpstrDefExt;
	unsigned				lCustData;
	OFNHOOKPROC				lpfnHook;
	const char*				lpTemplateName;
	void*					pvReserved;
	unsigned				dwReserved;
	unsigned				FlagsEx;
};
struct CHOOSECOLOR {
	unsigned				lStructSize;
	void*					hwndOwner;
	void*					hInstance;
	COLORREF				rgbResult;
	COLORREF*				lpCustColors;
	unsigned				Flags;
	unsigned				lCustData;
	OFNHOOKPROC				lpfnHook;
	const char*				lpTemplateName;
};
struct PIXELFORMATDESCRIPTOR {
	WORD nSize;
	WORD nVersion;
	unsigned dwFlags;
	unsigned char iPixelType;
	unsigned char cColorBits;
	unsigned char cRedBits;
	unsigned char cRedShift;
	unsigned char cGreenBits;
	unsigned char cGreenShift;
	unsigned char cBlueBits;
	unsigned char cBlueShift;
	unsigned char cAlphaBits;
	unsigned char cAlphaShift;
	unsigned char cAccumBits;
	unsigned char cAccumRedBits;
	unsigned char cAccumGreenBits;
	unsigned char cAccumBlueBits;
	unsigned char cAccumAlphaBits;
	unsigned char cDepthBits;
	unsigned char cStencilBits;
	unsigned char cAuxBuffers;
	unsigned char iLayerType;
	unsigned char bReserved;
	unsigned dwLayerMask;
	unsigned dwVisibleMask;
	unsigned dwDamageMask;
};
struct SYSTEMTIME {
	unsigned short wYear;
	unsigned short wMonth;
	unsigned short wDayOfWeek;
	unsigned short wDay;
	unsigned short wHour;
	unsigned short wMinute;
	unsigned short wSecond;
	unsigned short wMilliseconds;
};
struct IMAGE_TLS_DIRECTORY {
	unsigned	StartAddressOfRawData;
	unsigned	EndAddressOfRawData;
	unsigned	AddressOfIndex;
	unsigned	AddressOfCallBacks;
	unsigned	SizeOfZeroFill;
	unsigned	Characteristics;
};
struct FIXED
{
	WORD	fract;
	short	value;
};
struct MAT2
{
	FIXED eM11;
	FIXED eM12;
	FIXED eM21;
	FIXED eM22;
};
struct GLYPHMETRICS
{
	UINT gmBlackBoxX;
	UINT gmBlackBoxY;
	POINT gmptGlyphOrigin;
	short gmCellIncX;
	short gmCellIncY;
};
struct TEXTMETRICA
{
	LONG tmHeight;
	LONG tmAscent;
	LONG tmDescent;
	LONG tmInternalLeading;
	LONG tmExternalLeading;
	LONG tmAveCharWidth;
	LONG tmMaxCharWidth;
	LONG tmWeight;
	LONG tmOverhang;
	LONG tmDigitizedAspectX;
	LONG tmDigitizedAspectY;
	BYTE tmFirstChar;
	BYTE tmLastChar;
	BYTE tmDefaultChar;
	BYTE tmBreakChar;
	BYTE tmItalic;
	BYTE tmUnderlined;
	BYTE tmStruckOut;
	BYTE tmPitchAndFamily;
	BYTE tmCharSet;
};

DLL int WINAPI				AdjustWindowRectEx(RECT*, unsigned, int, unsigned);
DLL int WINAPI				AppendMenuA(void*, UINT, unsigned, const char*);
DLL int WINAPI				BringWindowToTop(void*);
DLL int WINAPI				ChooseColorA(CHOOSECOLOR*);
DLL int WINAPI				ClientToScreen(void*, POINT*);
DLL int WINAPI				CloseClipboard(void);
DLL int WINAPI				CloseHandle(void* hObject);
WINGDIAPI void* WINAPI		CreateCompatibleDC(void*);
DLL int WINAPI				CreateDirectoryA(const char* lpPathName, void* lpSecurityAttributes);
DLL void* WINAPI			CreateFileA(const char* lpFileName, unsigned dwDesiredAccess, unsigned dwShareMode, void* lpSecurityAttributes, unsigned dwCreationDisposition, unsigned dwFlagsAndAttributes, void* hTemplateFile);
WINGDIAPI void*	WINAPI		CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, const char*);
WINGDIAPI void*	WINAPI		CreateFontW(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, const wchar_t*);
DLL void* WINAPI			CreateMenu(void);
DLL void* WINAPI			CreatePopupMenu(void);
DLL void* WINAPI			CreateThread(void* lpThreadAttributes, unsigned dwStackSize, unsigned(*lpStartAddress)(void* param), void* lpParameter, unsigned dwCreationFlags, unsigned* lpThreadId);
DLL void* WINAPI			CreateWindowExA(unsigned, const char*, const char*, unsigned, int, int, int, int, void*, void*, void*, void*);
DLL int WINAPI				ChoosePixelFormat(void*, const PIXELFORMATDESCRIPTOR*);
DLL long WINAPI				DefWindowProcA(void*, UINT, int, int);
DLL void WINAPI				DeleteCriticalSection(void* p);
WINGDIAPI int WINAPI		DeleteDC(void*);
DLL int	WINAPI				DeleteFileA(const char* lpFileName);
WINGDIAPI int WINAPI		DeleteObject(void*);
DLL int WINAPI				DestroyMenu(void*);
DLL int WINAPI				DestroyWindow(void*);
DLL long WINAPI				DispatchMessageA(const MSG*);
DLL int WINAPI				EmptyClipboard(void);
DLL int WINAPI				EnableWindow(void*, int);
DLL void WINAPI				EnterCriticalSection(void* p);
DLL void WINAPI				ExitProcess(unsigned uExitCode);
DLL int WINAPI				FindClose(void* hFindFile);
DLL void* WINAPI			FindFirstFileA(const char* lpFileName, WIN32_FIND_DATA* lpFindFileData);
DLL int WINAPI				FindNextFileA(void* hFindFile, WIN32_FIND_DATA* lpFindFileData);
DLL unsigned WINAPI			FlsAlloc(void (_stdcall *lpCallback)(void* pfd));
DLL int WINAPI				FlsFree(unsigned dwFlsIndex);
DLL void* WINAPI			FlsGetValue(unsigned dwFlsIndex);
DLL int WINAPI				FlsSetValue(unsigned dwFlsIndex, void* lpFlsData);
DLL int WINAPI				FtpCreateDirectoryA(void* hConnect, const char* lpszDirectory);
DLL int WINAPI				FtpGetCurrentDirectoryA(void* hConnect, const char* lpszCurrentDirectory, unsigned* lpdwCurrentDirectory);
DLL int WINAPI				FtpSetCurrentDirectoryA(void* hConnect, const char* lpszDirectory);
DLL void* WINAPI			GetActiveWindow(void);
DLL unsigned WINAPI			GetCurrentDirectoryA(unsigned nBufferLength, char* lpBuffer);
DLL void* WINAPI			GetDC(void*);
DLL int WINAPI				GetExitCodeThread(void* hThread, unsigned* lpExitCode);
WINGDIAPI DWORD WINAPI		GetGlyphOutlineW(void*, UINT, UINT, GLYPHMETRICS*, DWORD, void*, const MAT2*);
DLL void WINAPI				GetLocalTime(SYSTEMTIME* lpSystemTime);
DLL unsigned WINAPI			GetModuleFileNameA(void* hModule, char* lpFilename, unsigned nSize);
DLL void* WINAPI			GetProcessHeap(void);
WINGDIAPI int WINAPI		GetTextMetricsA(void*, TEXTMETRICA*);
DLL unsigned WINAPI			GetTickCount(void);
DLL void* WINAPI			GlobalAlloc(unsigned flags, unsigned);
DLL void* WINAPI			GlobalLock(void*);
DLL unsigned WINAPI			GlobalSize(void*);
DLL int WINAPI				GlobalUnlock(void*);
DLL void* WINAPI			GetCapture(void);
DLL int WINAPI				GetClassInfoA(void*, const char*, WNDCLASS*);
DLL int WINAPI				GetClientRect(HWND, RECT*);
DLL void* WINAPI			GetClipboardData(UINT);
DLL const char*	WINAPI		GetCommandLineA(void);
DLL int WINAPI				GetCursorPos(POINT* lpPoint);
DLL void* WINAPI			GetDesktopWindow();
DLL int WINAPI				GetFileAttributesA(const char* lpFileName);
DLL HWND WINAPI				GetFocus();
DLL int WINAPI				GetKeyNameTextA(long, char*, int);
DLL short WINAPI			GetKeyState(int);
DLL int WINAPI				GetMenuItemInfoA(void* hMenu, unsigned uItem, int fByPosition, MENUITEMINFO* lpmii);
DLL int WINAPI				GetMessageA(MSG*, HWND, UINT, UINT);
DLL void* WINAPI			GetModuleHandleA(const char*);
DLL int WINAPI				GetOpenFileNameA(OPENFILENAME*);
DLL void* WINAPI			GetParent(void*);
DLL int WINAPI				GetSaveFileNameA(OPENFILENAME*);
DLL void* WINAPI			GetStdHandle(unsigned nStdHandle);
DLL int WINAPI				GetSystemMetrics(int);
DLL LONG WINAPI				GetWindowLongA(void*, int);
DLL int WINAPI				GetWindowPlacement(void* hWnd, WINDOWPLACEMENT* lpwndpl);
DLL int WINAPI				GetWindowRect(void* hwnd, RECT* rc);
DLL void* WINAPI			HeapAlloc(void* hHeap, unsigned dwFlags, unsigned dwBytes);
DLL int WINAPI				HeapFree(void* hHeap, unsigned dwFlags, void* lpMem);
DLL void* WINAPI			HeapReAlloc(void* hHeap, unsigned dwFlags, void* lpMem, unsigned dwBytes);
DLL void WINAPI				InitializeCriticalSection(void* p);
DLL void WINAPI				InitializeCriticalSectionAndSpinCount(void* p, unsigned count);
DLL int WINAPI				InsertMenuItemA(void*, UINT, int, const MENUITEMINFO*);
DLL int WINAPI				InternetCloseHandle(void* hInternet);
DLL void* WINAPI			InternetOpenA(const char* lpszAgent, unsigned dwAccessType, const char* lpszProxyName, const char* lpszProxyBypass, unsigned dwFlags);
DLL void* WINAPI			InternetConnectA(void* hInternet, const char* lpszServerName, int nServerPort, const char* lpszUsername, const char* lpszPassword, unsigned dwService, unsigned dwFlags, unsigned* dwContext);
DLL int WINAPI				InvalidateRect(HWND, RECT*, int);
DLL int WINAPI				IsClipboardFormatAvailable(UINT);
DLL int WINAPI				IsWindowVisible(HWND);
DLL int WINAPI				KillTimer(HWND, UINT);
DLL void WINAPI				LeaveCriticalSection(void* p);
DLL void* WINAPI			LoadCursorA(void*, const char*);
DLL void* WINAPI			LoadIconA(void*, const char*);
DLL int WINAPI				MapWindowPoints(void*, void*, POINT*, UINT);
DLL unsigned WINAPI			MapVirtualKeyA(unsigned, unsigned);
DLL int WINAPI				MessageBoxA(void* hWnd, const char* lpText, const char* lpCaption, unsigned uType);
DLL int WINAPI				OpenClipboard(HWND);
DLL int WINAPI				PathFileExistsA(const char* pszPath);
DLL int WINAPI				PeekMessageA(MSG*, void*, UINT, UINT, UINT);
DLL LRESULT WINAPI			PostMessageA(void*, UINT, int, int);
DLL void WINAPI				PostQuitMessage(int);
DLL int WINAPI				ReadFile(void* hFile, void* lpBuffer, unsigned nNumberOfBytesToRead, unsigned* lpNumberOfBytesRead, void* lpOverlapped);
DLL unsigned WINAPI			RegisterClassA(const WNDCLASS*);
DLL int WINAPI				ReleaseCapture(void);
DLL int WINAPI				ReleaseDC(void*, void*);
DLL int WINAPI				ScreenToClient(void* hWnd, POINT* lpPoint);
WINGDIAPI void* WINAPI		SelectObject(void*, void*);
DLL void* WINAPI			SetCapture(void*);
DLL void* WINAPI			SetClipboardData(UINT, void*);
DLL int WINAPI				SetCurrentDirectoryA(const char* path);
DLL void* WINAPI			SetCursor(void*);
DLL unsigned WINAPI			SetFilePointer(int hFile, unsigned lDistanceToMove, unsigned* lpDistanceToMoveHigh, int dwMoveMethod);
DLL void* WINAPI			SetFocus(void*);
DLL int WINAPI				SetForegroundWindow(void*);
DLL int WINAPI				SetPixelFormat(void*, int, const PIXELFORMATDESCRIPTOR*);
DLL UINT WINAPI				SetTimer(HWND, UINT, UINT, TIMERPROC);
DLL int WINAPI				SetWindowTextA(HWND, const char*);
DLL LRESULT WINAPI			SendMessageA(void*, UINT, int, int);
DLL void WINAPI				Sleep(unsigned dwMilliseconds);
DLL void* WINAPI			SHBrowseForFolderA(BROWSEINFOA*);
DLL int WINAPI				SHGetPathFromIDListA(void*, char*);
DLL int WINAPI				ShowWindow(HWND, int);
DLL int WINAPI				StretchDIBits(void*, int, int, int, int, int, int, int, int, const void*, const BITMAPINFO*, UINT, unsigned);
DLL int WINAPI				SetDIBitsToDevice(void*, int, int, unsigned, unsigned, int, int, UINT, UINT, const void*, const BITMAPINFO*, UINT);
DLL LONG WINAPI				SetWindowLongA(void*, int, LONG);
DLL int WINAPI				ShowCursor(int bShow);
DLL int WINAPI				SwapBuffers(void*);
DLL int WINAPI				TrackMouseEvent(TRACKMOUSEEVENT*);
DLL int WINAPI				TrackPopupMenuEx(void*, UINT, int, int, void*, TPMPARAMS*);
DLL int WINAPI				TranslateMessage(const MSG*);
DLL int WINAPI				TryEnterCriticalSection(void* data);
DLL int WINAPI				UpdateWindow(HWND);
DLL int WINAPI				UnregisterClassA(const char*, void*);
DLL unsigned WINAPI			WaitForSingleObject(void* handle, unsigned dwMilliseconds);
DLL int WINAPI				WriteFile(void* hFile, const void* lpBuffer, unsigned nNumberOfBytesToWrite, unsigned* lpNumberOfBytesWritten, void* lpOverlapped);
DLL void* WINAPI			wglCreateContext(void*);
DLL int WINAPI				wglMakeCurrent(void*, void*);
DLL int WINAPI				wglDeleteContext(void*);
DLL int WINAPI				wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase);