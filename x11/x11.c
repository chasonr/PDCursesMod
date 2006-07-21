/************************************************************************ 
 * This file is part of PDCurses. PDCurses is public domain software;	*
 * you may use it for any purpose. This software is provided AS IS with	*
 * NO WARRANTY whatsoever.						*
 *									*
 * If you use PDCurses in an application, an acknowledgement would be	*
 * appreciated, but is not mandatory. If you make corrections or	*
 * enhancements to PDCurses, please forward them to the current		*
 * maintainer for the benefit of other users.				*
 *									*
 * No distribution of modified PDCurses code may be made under the name	*
 * "PDCurses", except by the current maintainer. (Although PDCurses is	*
 * public domain, the name is a trademark.)				*
 *									*
 * See the file maintain.er for details of the current maintainer.	*
 ************************************************************************/

#include "pdcx11.h"

#ifdef HAVE_DECKEYSYM_H
# include <DECkeysym.h>
#endif

#ifdef HAVE_SUNKEYSYM_H
# include <Sunkeysym.h>
#endif

#ifdef HAVE_XPM_H
# include <xpm.h>
#endif

#include <stdlib.h>
#include <string.h>

RCSID("$Id: x11.c,v 1.2 2006/07/21 17:43:13 wmcbrine Exp $");

#ifndef XPOINTER_TYPEDEFED
typedef char * XPointer;
#endif

#ifndef MAX_PATH
# define MAX_PATH 256
#endif

AppData app_data;

#if NeedWidePrototypes
# define PDC_SCROLLBAR_TYPE double
#else
# define PDC_SCROLLBAR_TYPE float
#endif

#define MAX_COLORS   8  /* maximum of "normal" colours */
#define COLOR_CURSOR 16 /* colour of cursor - 1 more than 2 * MAX_COLORS */
#define COLOR_BORDER 17 /* colour of border - 2 more than 2 * MAX_COLORS */

#ifdef HAVE_XPM_H
# define PDC_NUMBER_OPTIONS 33
#define PDC_NUMBER_APP_RESOURCES 35
#else
# define PDC_NUMBER_OPTIONS 32
# define PDC_NUMBER_APP_RESOURCES 34
#endif
#define PDC_NUMBER_XCURSES_ACTIONS 5

#define XCURSESNORMALFONTINFO	app_data.normalfont
#define XCURSESITALICFONTINFO	app_data.italicfont
#define XCURSESLINES		app_data.lines
#define XCURSESCOLS		app_data.cols
#define XCURSESBITMAPFILE	app_data.bitmapFile
#ifdef HAVE_XPM_H
# define XCURSESPIXMAPFILE	app_data.pixmapFile
#endif
#define XCURSESCOMPOSEKEY	app_data.composeKey
#define XCURSESPOINTER		app_data.pointer
#define XCURSESPOINTERFORECOLOR app_data.pointerForeColor
#define XCURSESPOINTERBACKCOLOR app_data.pointerBackColor
#define XCURSESCURSORCOLOR	app_data.cursorColor
#define XCURSESBORDERWIDTH	app_data.borderWidth
#define XCURSESBORDERCOLOR	app_data.borderColor
#define XCURSESDOUBLECLICKPERIOD app_data.doubleClickPeriod
#define XCURSESCLICKPERIOD	app_data.clickPeriod
#define XCURSESSCROLLBARWIDTH	app_data.scrollbarWidth
#define XCURSESCURSORBLINKRATE	app_data.cursorBlinkRate
#define XCURSESTEXTCURSOR	app_data.textCursor

#define XCURSESDISPLAY		(XtDisplay(drawing))
#define XCURSESWIN		(XtWindow(drawing))

/* Default icons for XCurses applications.  */

#include "big_icon.xbm"
#include "little_icon.xbm"

void SelectionOff(void);
void XCursesButton(Widget, XEvent *, String *, Cardinal *);
void XCursesDisplayCursor(int, int, int, int);
void XCursesExitXCursesProcess(int, int, char *);
void XCursesHandleString(Widget, XEvent *, String *, Cardinal *);
void XCursesKeyPress(Widget, XEvent *, String *, Cardinal *);
void XCursesModifierPress(Widget, XEvent *, String *, Cardinal *);
void XCursesPasteSelection(Widget, XButtonEvent *);
void XCursesRequestorCallbackForGetSelection(Widget, XtPointer, Atom *, Atom *,
                                             XtPointer, unsigned long *, int *);
void XCursesRequestorCallbackForPaste(Widget, XtPointer, Atom *, Atom *,
				      XtPointer, unsigned long *, int *);
RETSIGTYPE XCursesSignalHandler(int);
void XCursesStructureNotify(Widget, XtPointer, XEvent *, Boolean *);

struct XCursesKey
{
	int keycode;
	int numkeypad;
	int normal;
	int shifted;
	int control;
	int alt;
};

typedef struct XCursesKey XCURSESKEY;

XCURSESKEY XCursesKeys[] =
{
/* keycode	keypad	normal	     shifted	   control	alt*/
 {XK_Left,	FALSE,	KEY_LEFT,    KEY_SLEFT,    CTL_LEFT,	ALT_LEFT},
 {XK_Right,	FALSE,	KEY_RIGHT,   KEY_SRIGHT,   CTL_RIGHT,	ALT_RIGHT},
 {XK_Up,	FALSE,	KEY_UP,      KEY_SUP,	   CTL_UP,	ALT_UP},
 {XK_Down,	FALSE,	KEY_DOWN,    KEY_SDOWN,    CTL_DOWN,	ALT_DOWN},
 {XK_Home,	FALSE,	KEY_HOME,    KEY_SHOME,    CTL_HOME,	ALT_HOME},
/* Sun Type 4 keyboard */
 {XK_R7,	FALSE,	KEY_HOME,    KEY_SHOME,    CTL_HOME,	ALT_HOME},
 {XK_End,	FALSE,	KEY_END,     KEY_SEND,	   CTL_END,	ALT_END},
/* Sun Type 4 keyboard */
 {XK_R13,	FALSE,	KEY_END,     KEY_SEND,	   CTL_END,	ALT_END},
 {XK_Prior,	FALSE,	KEY_PPAGE,   KEY_SPREVIOUS,CTL_PGUP,	ALT_PGUP},
/* Sun Type 4 keyboard */
 {XK_R9,	FALSE,	KEY_PPAGE,   KEY_SPREVIOUS,CTL_PGUP,	ALT_PGUP},
 {XK_Next,	FALSE,	KEY_NPAGE,   KEY_SNEXT,    CTL_PGDN,	ALT_PGDN},
/* Sun Type 4 keyboard */
 {XK_R15,	FALSE,	KEY_NPAGE,   KEY_SNEXT,    CTL_PGDN,	ALT_PGDN},
 {XK_Insert,	FALSE,	KEY_IC,      KEY_SIC,	   CTL_INS,	ALT_INS},
 {XK_Delete,	FALSE,	KEY_DC,      KEY_SDC,	   CTL_DEL,	ALT_DEL},
 {XK_F1,	FALSE,	KEY_F(1),    KEY_F(13),    KEY_F(25),	KEY_F(37)},
 {XK_F2,	FALSE,	KEY_F(2),    KEY_F(14),    KEY_F(26),	KEY_F(38)},
 {XK_F3,	FALSE,	KEY_F(3),    KEY_F(15),    KEY_F(27),	KEY_F(39)},
 {XK_F4,	FALSE,	KEY_F(4),    KEY_F(16),    KEY_F(28),	KEY_F(40)},
 {XK_F5,	FALSE,	KEY_F(5),    KEY_F(17),    KEY_F(29),	KEY_F(41)},
 {XK_F6,	FALSE,	KEY_F(6),    KEY_F(18),    KEY_F(30),	KEY_F(42)},
 {XK_F7,	FALSE,	KEY_F(7),    KEY_F(19),    KEY_F(31),	KEY_F(43)},
 {XK_F8,	FALSE,	KEY_F(8),    KEY_F(20),    KEY_F(32),	KEY_F(44)},
 {XK_F9,	FALSE,	KEY_F(9),    KEY_F(21),    KEY_F(33),	KEY_F(45)},
 {XK_F10,	FALSE,	KEY_F(10),   KEY_F(22),    KEY_F(34),	KEY_F(46)},
 {XK_F11,	FALSE,	KEY_F(11),   KEY_F(23),    KEY_F(35),	KEY_F(47)},
 {XK_F12,	FALSE,	KEY_F(12),   KEY_F(24),    KEY_F(36),	KEY_F(48)},
 {XK_F13,	FALSE,	KEY_F(13),   KEY_F(25),    KEY_F(37),	KEY_F(49)},
 {XK_F14,	FALSE,	KEY_F(14),   KEY_F(26),    KEY_F(38),	KEY_F(50)},
 {XK_F15,	FALSE,	KEY_F(15),   KEY_F(27),    KEY_F(39),	KEY_F(51)},
 {XK_F16,	FALSE,	KEY_F(16),   KEY_F(28),    KEY_F(40),	KEY_F(52)},
 {XK_F17,	FALSE,	KEY_F(17),   KEY_F(29),    KEY_F(41),	KEY_F(53)},
 {XK_F18,	FALSE,	KEY_F(18),   KEY_F(30),    KEY_F(42),	KEY_F(54)},
 {XK_F19,	FALSE,	KEY_F(19),   KEY_F(31),    KEY_F(43),	KEY_F(55)},
 {XK_F20,	FALSE,	KEY_F(20),   KEY_F(32),    KEY_F(44),	KEY_F(56)},
 {XK_BackSpace, FALSE,	0x08,        0x08,	   CTL_BKSP,	ALT_BKSP},
 {XK_Tab,	FALSE,	0x09,        KEY_BTAB,	   CTL_TAB,	ALT_TAB},
 {XK_Select,	FALSE,	KEY_SELECT,  KEY_SELECT,   KEY_SELECT,	KEY_SELECT},
 {XK_Print,	FALSE,	KEY_PRINT,   KEY_SPRINT,   KEY_PRINT,	KEY_PRINT},
 {XK_Find,	FALSE,	KEY_FIND,    KEY_SFIND,    KEY_FIND,	KEY_FIND},
 {XK_Pause,	FALSE,	KEY_SUSPEND, KEY_SSUSPEND, KEY_SUSPEND, KEY_SUSPEND},
 {XK_Clear,	FALSE,	KEY_CLEAR,   KEY_CLEAR,    KEY_CLEAR,	KEY_CLEAR},
 {XK_Cancel,	FALSE,	KEY_CANCEL,  KEY_SCANCEL,  KEY_CANCEL,	KEY_CANCEL},
 {XK_Break,	FALSE,	KEY_BREAK,   KEY_BREAK,    KEY_BREAK,	KEY_BREAK},
 {XK_Help,	FALSE,	KEY_HELP,    KEY_SHELP,    KEY_LHELP,	KEY_HELP},
 {XK_L4,	FALSE,	KEY_UNDO,    KEY_SUNDO,    KEY_UNDO,	KEY_UNDO},
 {XK_L6,	FALSE,	KEY_COPY,    KEY_SCOPY,    KEY_COPY,	KEY_COPY},
 {XK_L9,	FALSE,	KEY_FIND,    KEY_SFIND,    KEY_FIND,	KEY_FIND},
 {XK_Menu,	FALSE,	KEY_OPTIONS, KEY_SOPTIONS, KEY_OPTIONS, KEY_OPTIONS},
#ifdef HAVE_SUNKEYSYM_H
 {SunXK_F36,	FALSE,	KEY_F(41),   KEY_F(43),    KEY_F(45),	KEY_F(47)},
 {SunXK_F37,	FALSE,	KEY_F(42),   KEY_F(44),    KEY_F(46),	KEY_F(48)},
#endif
#ifdef HAVE_DECKEYSYM_H
 {DXK_Remove,	FALSE,	KEY_DC,      KEY_SDC,	   CTL_DEL,	ALT_DEL},
#endif
 {XK_Escape,	FALSE,	0x1B,        0x1B,	   0x1B,	ALT_ESC},
 {XK_KP_Enter,	TRUE,	PADENTER,    PADENTER,	   CTL_PADENTER,ALT_PADENTER},
 {XK_KP_Add,	TRUE,	PADPLUS,     (int)'+',	   CTL_PADPLUS, ALT_PADPLUS},
 {XK_KP_Subtract,TRUE,	PADMINUS,    (int)'-',	   CTL_PADMINUS,ALT_PADMINUS},
 {XK_KP_Multiply,TRUE,	PADSTAR,     (int)'*',	   CTL_PADSTAR, ALT_PADSTAR},
/* Sun Type 4 keyboard */
 {XK_R6,	TRUE,	PADSTAR,     (int)'*',	   CTL_PADSTAR, ALT_PADSTAR},
 {XK_KP_Divide, TRUE,	PADSLASH,    (int)'/',	   CTL_PADSLASH,ALT_PADSLASH},
/* Sun Type 4 keyboard */
 {XK_R5,	TRUE,	PADSLASH,    (int)'/',	   CTL_PADSLASH,ALT_PADSLASH},
 {XK_KP_Decimal,TRUE,	PADSTOP,     (int)'.',	   CTL_PADSTOP, ALT_PADSTOP},
 {XK_KP_0,	TRUE,	PAD0,	     (int)'0',	   CTL_PAD0,	ALT_PAD0},
 {XK_KP_1,	TRUE,	KEY_C1,      (int)'1',	   CTL_PAD1,	ALT_PAD1},
 {XK_KP_2,	TRUE,	KEY_C2,      (int)'2',	   CTL_PAD2,	ALT_PAD2},
 {XK_KP_3,	TRUE,	KEY_C3,      (int)'3',	   CTL_PAD3,	ALT_PAD3},
 {XK_KP_4,	TRUE,	KEY_B1,      (int)'4',	   CTL_PAD4,	ALT_PAD4},
 {XK_KP_5,	TRUE,	KEY_B2,      (int)'5',	   CTL_PAD5,	ALT_PAD5},
/* Sun Type 4 keyboard */
 {XK_R11,	TRUE,	KEY_B2,      (int)'5',	   CTL_PAD5,	ALT_PAD5},
 {XK_KP_6,	TRUE,	KEY_B3,      (int)'6',	   CTL_PAD6,	ALT_PAD6},
 {XK_KP_7,	TRUE,	KEY_A1,      (int)'7',	   CTL_PAD7,	ALT_PAD7},
 {XK_KP_8,	TRUE,	KEY_A2,      (int)'8',	   CTL_PAD8,	ALT_PAD8},
 {XK_KP_9,	TRUE,	KEY_A3,      (int)'9',	   CTL_PAD9,	ALT_PAD9},
/* the following added to support Sun Type 5 keyboards */
 {XK_F21,	FALSE,	KEY_SUSPEND, KEY_SSUSPEND, KEY_SUSPEND, KEY_SUSPEND},
 {XK_F22,	FALSE,	KEY_PRINT,   KEY_SPRINT,   KEY_PRINT,	KEY_PRINT},
 {XK_F24,	TRUE,	PADMINUS,    (int)'-',	   CTL_PADMINUS,ALT_PADMINUS},
/* Sun Type 4 keyboard */
 {XK_F25,	TRUE,	PADSLASH,    (int)'/',	   CTL_PADSLASH,ALT_PADSLASH},
/* Sun Type 4 keyboard */
 {XK_F26,	TRUE,	PADSTAR,     (int)'*',	   CTL_PADSTAR, ALT_PADSTAR},
 {XK_F27,	TRUE,	KEY_A1,      (int)'7',	   CTL_PAD7,	ALT_PAD7},
 {XK_F29,	TRUE,	KEY_A3,      (int)'9',	   CTL_PAD9,	ALT_PAD9},
 {XK_F31,	TRUE,	KEY_B2,      (int)'5',	   CTL_PAD5,	ALT_PAD5},
 {XK_F35,	TRUE,	KEY_C3,      (int)'3',	   CTL_PAD3,	ALT_PAD3},
#ifdef HAVE_XK_KP_DELETE
 {XK_KP_Delete, TRUE,	PADSTOP,     (int)'.',	   CTL_PADSTOP, ALT_PADSTOP},
#endif
#ifdef HAVE_XK_KP_INSERT
 {XK_KP_Insert, TRUE,	PAD0,	     (int)'0',	   CTL_PAD0,	ALT_PAD0},
#endif
#ifdef HAVE_XK_KP_END
 {XK_KP_End,	TRUE,	KEY_C1,      (int)'1',	   CTL_PAD1,	ALT_PAD1},
#endif
#ifdef HAVE_XK_KP_DOWN
 {XK_KP_Down,	TRUE,	KEY_C2,      (int)'2',	   CTL_PAD2,	ALT_PAD2},
#endif
#ifdef HAVE_XK_KP_NEXT
 {XK_KP_Next,	TRUE,	KEY_C3,      (int)'3',	   CTL_PAD3,	ALT_PAD3},
#endif
#ifdef HAVE_XK_KP_LEFT
 {XK_KP_Left,	TRUE,	KEY_B1,      (int)'4',	   CTL_PAD4,	ALT_PAD4},
#endif
#ifdef HAVE_XK_KP_BEGIN
 {XK_KP_Begin,	TRUE,	KEY_B2,      (int)'5',	   CTL_PAD5,	ALT_PAD5},
#endif
#ifdef HAVE_XK_KP_RIGHT
 {XK_KP_Right,	TRUE,	KEY_B3,      (int)'6',	   CTL_PAD6,	ALT_PAD6},
#endif
#ifdef HAVE_XK_KP_HOME
 {XK_KP_Home,	TRUE,	KEY_A1,      (int)'7',	   CTL_PAD7,	ALT_PAD7},
#endif
#ifdef HAVE_XK_KP_UP
 {XK_KP_Up,	TRUE,	KEY_A2,      (int)'8',	   CTL_PAD8,	ALT_PAD8},
#endif
#ifdef HAVE_XK_KP_PRIOR
 {XK_KP_Prior,	TRUE,	KEY_A3,      (int)'9',	   CTL_PAD9,	ALT_PAD9},
#endif

#ifdef INCLUDE_LATIN1
# include "latin1kbd.h"
#endif

 {0,		0,	0,	     0,		   0,		0},
};

#define MAX_COMPOSE_CHARS 14
#define MAX_COMPOSE_PRE 60

char *compose_chars =
	"`'~^,/\"AaPpSs!?0123CcRr-_<>Xx.=Yy |EeIiOoUu+NnLlgDd:*TtMmVv";

/*
   ` :    �   �   �   �   �   �   �   �   �   �
   ' :    �   �   �   �   �   �   �   �   �   �   �   �   �   �
   ~ :    �   �   �   �   �   �
   ^ :    �   �   �   �   �   �   �   �   �   �
   , :    �   �   �   �
   / :    �   �   �   �
   " :    �   �   �   �   �   �   �   �   �   �   �   �   �
   A :    �   �   �   �   �   �   �   �   �   �
   a :    �   �   �   �   �   �   �   �   �   �
   P :    �   �   �
   p :    �   �   �
   S :    �   �   �   �   �
   s :    �   �   �   �   �   �   �   �
   ! :    �   �
   ? :    �   �
   1 :    �   �   �   �
   0 :    �   �
   2 :    �   �
   3 :    �   �   �
   C :    �   �   �   �
   c :    �   �   �   �   �
   R :    �
   r :    �
   - :    �   �   �   �   �   �
   _ :    �   �
   < :    �
   > :    �
   X :    �   �
   x :    �   �   �
   . :    �   �   �
   = :    �   �
   Y :    �   �   �
   y :    �   �   �   �   �
     :
   | :    �   �   �   �   �   �
   E :    �   �   �   �   �
   e :    �   �   �   �   �
   I :    �   �   �   �   �
   i :    �   �   �   �   �
   O :    �   �   �   �   �   �   �   �   �   �   �   �
   o :    �   �   �   �   �   �   �   �   �   �   �   �   �
   U :    �   �   �   �   �
   u :    �   �   �   �   �   �
   + :    �
   N :    �   �   �
   n :    �   �   �
   L :    �   �   �
   l :    �   �   �
   g :    �
   D :    �   �
   d :    �   �
   : :    �
   * :    �   �
   T :    �
   t :    �
   M :    �
   m :    �
   V :    �
   v :    �
*/

char compose_lookups[MAX_COMPOSE_PRE][MAX_COMPOSE_CHARS] =
{
/* ` */ {'A','E','I','O','U','a','e','i','o','u',000,000,000,000},
/* ' */ {' ','A','E','I','O','U','Y','a','e','i','o','u','y', 39},
/* ~ */ {'A','N','O','a','n','o',000,000,000,000,000,000,000,000},
/* ^ */ {'A','E','I','O','U','a','e','i','o','u',000,000,000,000},
/* , */ {' ','C','c',',',000,000,000,000,000,000,000,000,000,000},
/* / */ {'O','o','u','U',000,000,000,000,000,000,000,000,000,000},
/* " */ {' ','A','E','I','O','U','a','e','i','o','u','y','\"',000},
/* A */ {'E','`', 39,'~','^','"','o','*','-','>','_',000,000,000},
/* a */ {'e','`', 39,'~','^','"','o','-','_','>',000,000,000,000},
/* P */ {' ','G','!',000,000,000,000,000,000,000,000,000,000,000},
/* p */ {' ','g','!',000,000,000,000,000,000,000,000,000,000,000},
/* S */ {'S','!','O','0','A',000,000,000,000,000,000,000,000,000},
/* s */ {'s','!','a','1','2','3','0','o',000,000,000,000,000,000},
/* ! */ {' ','!',000,000,000,000,000,000,000,000,000,000,000,000},
/* ? */ {' ','?',000,000,000,000,000,000,000,000,000,000,000,000},
/* 0 */ {'^','*',000,000,000,000,000,000,000,000,000,000,000,000},
/* 1 */ {' ','2','4','^',000,000,000,000,000,000,000,000,000,000},
/* 2 */ {' ','^',000,000,000,000,000,000,000,000,000,000,000,000},
/* 3 */ {' ','4','^',000,000,000,000,000,000,000,000,000,000,000},
/* C */ {'O',',','$','|',000,000,000,000,000,000,000,000,000,000},
/* c */ {'|','o',',','/','$',000,000,000,000,000,000,000,000,000},
/* R */ {'O',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* r */ {'o',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* - */ {' ','+',',','-','^',':',000,000,000,000,000,000,000,000},
/* _ */ {'_','^',000,000,000,000,000,000,000,000,000,000,000,000},
/* < */ {'<',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* > */ {'>',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* X */ {'O','X',000,000,000,000,000,000,000,000,000,000,000,000},
/* x */ {' ','o','x',000,000,000,000,000,000,000,000,000,000,000},
/* . */ {' ','^','.',000,000,000,000,000,000,000,000,000,000,000},
/* = */ {'Y','y',000,000,000,000,000,000,000,000,000,000,000,000},
/* Y */ {'=', 39,'-',000,000,000,000,000,000,000,000,000,000,000},
/* y */ {'=', 39,'"','$','-',000,000,000,000,000,000,000,000,000},
/*   */ {' ',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* | */ {' ','c','|','P','p','^',000,000,000,000,000,000,000,000},
/* E */ {'`', 39,'^','"','>',000,000,000,000,000,000,000,000,000},
/* e */ {'`', 39,'^','"','>',000,000,000,000,000,000,000,000,000},
/* I */ {'`', 39,'^','"','>',000,000,000,000,000,000,000,000,000},
/* i */ {'`', 39,'^','"','>',000,000,000,000,000,000,000,000,000},
/* O */ {'`', 39,'^','~','"','/','C','R','-','>','A','a',000,000},
/* o */ {' ','A','a','x','-','`', 39,'^','~','"','/','>','_',000},
/* U */ {'`', 39,'^','"','>',000,000,000,000,000,000,000,000,000},
/* u */ {'`', 39,'^','"',' ','>',000,000,000,000,000,000,000,000},
/* + */ {'-',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* N */ {'~','-','O',000,000,000,000,000,000,000,000,000,000,000},
/* n */ {'~','-','o',000,000,000,000,000,000,000,000,000,000,000},
/* L */ {'$','=','-',000,000,000,000,000,000,000,000,000,000,000},
/* l */ {'$','=','-',000,000,000,000,000,000,000,000,000,000,000},
/* g */ {'$',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* D */ {'E','-',000,000,000,000,000,000,000,000,000,000,000,000},
/* d */ {'e','-',000,000,000,000,000,000,000,000,000,000,000,000},
/* : */ {'-',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* * */ {'m','M',000,000,000,000,000,000,000,000,000,000,000,000},
/* T */ {'H',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* t */ {'h',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* M */ {'U',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* m */ {'u',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* V */ {'B',000,000,000,000,000,000,000,000,000,000,000,000,000},
/* v */ {'b',000,000,000,000,000,000,000,000,000,000,000,000,000},
};

int compose_keys[MAX_COMPOSE_PRE][MAX_COMPOSE_CHARS] =
{
/* ` */ {192,200,204,210,217,224,232,236,242,249,000,000,000,000},
/* ' */ {180,193,201,205,211,218,221,225,233,237,243,250,253,180},
/* ~ */ {195,209,213,227,241,245,000,000,000,000,000,000,000,000},
/* ^ */ {194,202,206,212,219,226,234,238,244,251,000,000,000,000},
/* , */ {184,199,231,184,000,000,000,000,000,000,000,000,000,000},
/* / */ {216,248,181,181,000,000,000,000,000,000,000,000,000,000},
/* " */ {168,196,203,207,214,220,228,235,239,246,252,255,168,000},
/* A */ {198,192,193,195,194,196,197,197,195,194,170,000,000,000},
/* a */ {230,224,225,227,226,228,229,227,170,226,000,000,000,000},
/* P */ {222,182,182,000,000,000,000,000,000,000,000,000,000,000},
/* p */ {254,182,182,000,000,000,000,000,000,000,000,000,000,000},
/* S */ {167,167,167,186,170,000,000,000,000,000,000,000,000,000},
/* s */ {223,167,170,185,178,179,186,167,000,000,000,000,000,000},
/* ! */ {161,161,000,000,000,000,000,000,000,000,000,000,000,000},
/* ? */ {191,191,000,000,000,000,000,000,000,000,000,000,000,000},
/* 0 */ {176,176,000,000,000,000,000,000,000,000,000,000,000,000},
/* 1 */ {185,189,188,185,000,000,000,000,000,000,000,000,000,000},
/* 2 */ {178,178,000,000,000,000,000,000,000,000,000,000,000,000},
/* 3 */ {179,190,179,000,000,000,000,000,000,000,000,000,000,000},
/* C */ {169,199,162,162,000,000,000,000,000,000,000,000,000,000},
/* c */ {162,169,231,162,162,000,000,000,000,000,000,000,000,000},
/* R */ {174,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* r */ {174,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* - */ {173,177,172,173,175,247,000,000,000,000,000,000,000,000},
/* _ */ {175,175,000,000,000,000,000,000,000,000,000,000,000,000},
/* < */ {171,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* > */ {187,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* X */ {164,215,000,000,000,000,000,000,000,000,000,000,000,000},
/* x */ {215,164,215,000,000,000,000,000,000,000,000,000,000,000},
/* . */ {183,183,183,000,000,000,000,000,000,000,000,000,000,000},
/* = */ {165,165,000,000,000,000,000,000,000,000,000,000,000,000},
/* Y */ {165,221,165,000,000,000,000,000,000,000,000,000,000,000},
/* y */ {165,253,255,165,165,000,000,000,000,000,000,000,000,000},
/*   */ {160,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* | */ {166,162,166,222,254,166,000,000,000,000,000,000,000,000},
/* E */ {200,201,202,203,202,000,000,000,000,000,000,000,000,000},
/* e */ {232,233,234,235,234,000,000,000,000,000,000,000,000,000},
/* I */ {204,205,206,207,206,000,000,000,000,000,000,000,000,000},
/* i */ {236,237,238,239,238,000,000,000,000,000,000,000,000,000},
/* O */ {210,211,212,213,214,216,169,174,213,212,197,229,000,000},
/* o */ {176,197,229,164,245,242,243,244,245,246,248,244,186,000},
/* U */ {217,218,219,220,219,000,000,000,000,000,000,000,000,000},
/* u */ {249,250,251,252,181,251,000,000,000,000,000,000,000,000},
/* + */ {177,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* N */ {209,209,172,000,000,000,000,000,000,000,000,000,000,000},
/* n */ {241,241,172,000,000,000,000,000,000,000,000,000,000,000},
/* L */ {163,163,163,000,000,000,000,000,000,000,000,000,000,000},
/* l */ {163,163,163,000,000,000,000,000,000,000,000,000,000,000},
/* g */ {164,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* D */ {176,208,000,000,000,000,000,000,000,000,000,000,000,000},
/* d */ {176,240,000,000,000,000,000,000,000,000,000,000,000,000},
/* : */ {247,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* * */ {181,181,000,000,000,000,000,000,000,000,000,000,000,000},
/* T */ {222,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* t */ {254,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* M */ {215,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* u */ {215,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* V */ {166,000,000,000,000,000,000,000,000,000,000,000,000,000},
/* v */ {166,000,000,000,000,000,000,000,000,000,000,000,000,000},
};

#define BITMAPDEPTH 1

MOUSE_STATUS Trapped_Mouse_status;
unsigned long pdc_key_modifiers = 0L;
GC normal_gc, block_cursor_gc, rect_cursor_gc, italic_gc, border_gc;
int XCursesFontHeight, XCursesFontWidth;
int XCursesFontAscent, XCursesFontDescent;
int XCursesWindowWidth, XCursesWindowHeight;
int resizeXCursesWindowWidth = 0,resizeXCursesWindowHeight = 0;
char *bitmap_file = NULL;
#ifdef HAVE_XPM_H
char *pixmap_file = NULL;
#endif
KeySym compose_key = 0;
int compose_mask = 0;

int state_mask[8] =
{
	ShiftMask,
	LockMask,
	ControlMask,
	Mod1Mask,
	Mod2Mask,
	Mod3Mask,
	Mod4Mask,
	Mod5Mask
};

Atom wm_atom[2];
char *XCursesClassName = "XCurses";
XtAppContext app_context;
Widget topLevel, drawing, scrollBox, scrollVert, scrollHoriz;
int ReceivedMapNotify = 0;
Boolean mouse_selection = False;
chtype *tmpsel = NULL;
unsigned long tmpsel_length = 0;
int selection_start_x = 0;
int selection_start_y = 0;
int selection_end_x = 0;
int selection_end_y = 0;
Pixmap icon_bitmap;
#ifdef HAVE_XPM_H
Pixmap icon_pixmap;
Pixmap icon_pixmap_mask;
#endif
int visible_cursor = 0;
int windowEntered = 1;
static char *XCursesProgramName;

XtResource app_resources[PDC_NUMBER_APP_RESOURCES] =
{
	{
		"lines",
		"Lines",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, lines),
		XtRImmediate,
		(XtPointer)24,
	},

	{
		"cols",
		"Cols",
		XtRInt,
		sizeof(int), 
		XtOffsetOf(AppData, cols),
		XtRImmediate,
		(XtPointer)80,
	},

	{
		"cursorColor",
		"CursorColor",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, cursorColor),
		XtRString,
		(XtPointer)"Red",
	},

	{
		"colorBlack",
		"ColorBlack",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBlack),
		XtRString,
		(XtPointer)"Black",
	},

	{
		"colorRed",
		"ColorRed",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorRed),
		XtRString,
		(XtPointer)"red3",
	},

	{
		"colorGreen",
		"ColorGreen",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorGreen),
		XtRString,
		(XtPointer)"green3",
	},

	{
		"colorYellow",
		"ColorYellow",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorYellow),
		XtRString,
		(XtPointer)"yellow3",
	},

	{
		"colorBlue",
		"ColorBlue",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBlue),
		XtRString,
		(XtPointer)"blue3",
	},

	{
		"colorMagenta",
		"ColorMagenta",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorMagenta),
		XtRString,
		(XtPointer)"magenta3",
	},

	{
		"colorCyan",
		"ColorCyan",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorCyan),
		XtRString,
		(XtPointer)"cyan3",
	},

	{
		"colorWhite",
		"ColorWhite",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorWhite),
		XtRString,
		(XtPointer)"Grey",
	},

	{
		"colorBoldBlack",
		"ColorBoldBlack",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldBlack),
		XtRString,
		(XtPointer)"grey40",
	},

	{
		"colorBoldRed",
		"ColorBoldRed",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldRed),
		XtRString,
		(XtPointer)"red1",
	},

	{
		"colorBoldGreen",
		"ColorBoldGreen",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldGreen),
		XtRString,
		(XtPointer)"green1",
	},

	{
		"colorBoldYellow",
		"ColorBoldYellow",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldYellow),
		XtRString,
		(XtPointer)"yellow1",
	},

	{
		"colorBoldBlue",
		"ColorBoldBlue",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldBlue),
		XtRString,
		(XtPointer)"blue1",
	},

	{
		"colorBoldMagenta",
		"ColorBoldMagenta",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldMagenta),
		XtRString,
		(XtPointer)"magenta1",
	},

	{
		"colorBoldCyan",
		"ColorBoldCyan",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldCyan),
		XtRString,
		(XtPointer)"cyan1",
	},

	{
		"colorBoldWhite",
		"ColorBoldWhite",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, colorBoldWhite),
		XtRString,
		(XtPointer)"White",
	},

	{
		"normalFont",
		"NormalFont",
		XtRFontStruct,
		sizeof(XFontStruct),
		XtOffsetOf(AppData, normalfont),
		XtRString,
		(XtPointer)
#ifdef PDC_WIDE
		"-misc-fixed-medium-r-normal--20-200-75-75-c-100-iso10646-1",
#else
		"7x13",
#endif
	},

	{
		"italicFont",
		"ItalicFont",
		XtRFontStruct,
		sizeof(XFontStruct),
		XtOffsetOf(AppData, italicfont),
		XtRString,
		(XtPointer)
#ifdef PDC_WIDE
		"-misc-fixed-medium-r-normal--20-200-75-75-c-100-iso10646-1",
#else
		"7x13",
#endif
	},

	{
		"bitmap",
		"Bitmap",
		XtRString,
		MAX_PATH,
		XtOffsetOf(AppData, bitmapFile),
		XtRString,
		(XtPointer)"",
	},

#ifdef HAVE_XPM_H
	{
		"pixmap",
		"Pixmap",
		XtRString,
		MAX_PATH,
		XtOffsetOf(AppData, pixmapFile),
		XtRString,
		(XtPointer)"",
	},
#endif

	{
		"composeKey",
		"ComposeKey",
		XtRString,
		MAX_PATH,
		XtOffsetOf(AppData, composeKey),
		XtRString,
		(XtPointer)"",
	},

	{
		"pointer",
		"Pointer",
		XtRCursor,
		sizeof(Cursor),
		XtOffsetOf(AppData, pointer),
		XtRString,
		(XtPointer)"xterm",
	},

	{
		"pointerForeColor",
		"PointerForeColor",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, pointerForeColor),
		XtRString,
		(XtPointer)"Black",
	},

	{
		"pointerBackColor",
		"PointerBackColor",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, pointerBackColor),
		XtRString,
		(XtPointer)"White",
	},

	{
		"shmmin",
		"Shmmin",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, shmmin),
		XtRImmediate,
		(XtPointer)0,
	},

	{
		"borderWidth",
		"BorderWidth",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, borderWidth),
		XtRImmediate,
		(XtPointer)0,
	},

	{
		"borderColor",
		"BorderColor",
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(AppData, borderColor),
		XtRString,
		(XtPointer)"Black",
	},

	{
		"doubleClickPeriod",
		"DoubleClickPeriod",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, doubleClickPeriod),
		XtRImmediate,
		(XtPointer)200,
	},

	{
		"clickPeriod",
		"ClickPeriod",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, clickPeriod),
		XtRImmediate,
		(XtPointer)100,
	},

	{
		"scrollbarWidth",
		"ScrollbarWidth",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, scrollbarWidth),
		XtRImmediate,
		(XtPointer)15,
	},

	{
		"cursorBlinkRate",
		"CursorBlinkRate",
		XtRInt,
		sizeof(int),
		XtOffsetOf(AppData, cursorBlinkRate),
		XtRImmediate,
		(XtPointer)0,
	},

	{
		"textCursor",
		"TextCursor",
		XtRString,
		MAX_PATH,
		XtOffsetOf(AppData, textCursor),
		XtRString,
		(XtPointer)"",
	},

};

XrmOptionDescRec options[PDC_NUMBER_OPTIONS] =
{
   {"-lines",		  "*lines",		XrmoptionSepArg, NULL},
   {"-cols",		  "*cols",		XrmoptionSepArg, NULL},
   {"-normalFont",	  "*normalFont",	XrmoptionSepArg, NULL},
   {"-italicFont",	  "*italicFont",	XrmoptionSepArg, NULL},
   {"-bitmap",		  "*bitmap",		XrmoptionSepArg, NULL},
#ifdef HAVE_XPM_H
   {"-pixmap",		  "*pixmap",		XrmoptionSepArg, NULL},
#endif
   {"-pointer",		  "*pointer",		XrmoptionSepArg, NULL},
   {"-shmmin",		  "*shmmin",		XrmoptionSepArg, NULL},
   {"-composeKey",	  "*composeKey",	XrmoptionSepArg, NULL},
   {"-clickPeriod",	  "*clickPeriod",	XrmoptionSepArg, NULL},
   {"-doubleClickPeriod", "*doubleClickPeriod", XrmoptionSepArg, NULL},
   {"-scrollbarWidth",	  "*scrollbarWidth",	XrmoptionSepArg, NULL},
   {"-pointerForeColor",  "*pointerForeColor",	XrmoptionSepArg, NULL},
   {"-pointerBackColor",  "*pointerBackColor",	XrmoptionSepArg, NULL},
   {"-cursorBlinkRate",   "*cursorBlinkRate",	XrmoptionSepArg, NULL},
   {"-cursorColor",	  "*cursorColor",	XrmoptionSepArg, NULL},
   {"-textCursor",	  "*textCursor",	XrmoptionSepArg, NULL},
   {"-colorBlack",	  "*colorBlack",	XrmoptionSepArg, NULL},
   {"-colorRed",	  "*colorRed",		XrmoptionSepArg, NULL},
   {"-colorGreen",	  "*colorGreen",	XrmoptionSepArg, NULL},
   {"-colorYellow",	  "*colorYellow",	XrmoptionSepArg, NULL},
   {"-colorBlue",	  "*colorBlue",		XrmoptionSepArg, NULL},
   {"-colorMagenta",	  "*colorMagenta",	XrmoptionSepArg, NULL},
   {"-colorCyan",	  "*colorCyan",		XrmoptionSepArg, NULL},
   {"-colorWhite",	  "*colorWhite",	XrmoptionSepArg, NULL},
   {"-colorBoldBlack",	  "*colorBoldBlack",	XrmoptionSepArg, NULL},
   {"-colorBoldRed",	  "*colorBoldRed",	XrmoptionSepArg, NULL},
   {"-colorBoldGreen",	  "*colorBoldGreen",	XrmoptionSepArg, NULL},
   {"-colorBoldYellow",   "*colorBoldYellow",	XrmoptionSepArg, NULL},
   {"-colorBoldBlue",	  "*colorBoldBlue",	XrmoptionSepArg, NULL},
   {"-colorBoldMagenta",  "*colorBoldMagenta",	XrmoptionSepArg, NULL},
   {"-colorBoldCyan",	  "*colorBoldCyan",	XrmoptionSepArg, NULL},
   {"-colorBoldWhite",	  "*colorBoldWhite",	XrmoptionSepArg, NULL},
};

XtActionsRec XCursesActions[PDC_NUMBER_XCURSES_ACTIONS] =
{
   {"XCursesButton",		(XtActionProc)XCursesButton},
   {"XCursesKeyPress",		(XtActionProc)XCursesKeyPress},
   {"XCursesModifierPress",	(XtActionProc)XCursesModifierPress},
   {"XCursesPasteSelection",	(XtActionProc)XCursesPasteSelection},
   {"string",			(XtActionProc)XCursesHandleString},
};

char global_display_name[100];		/* large enough for DISPLAY=machine */
Bool after_first_curses_request = False;
int colors[(2 * MAX_COLORS) + 2];
Bool vertical_cursor = False;

#ifdef FOREIGN
	XIM Xim;
	XIC Xic;
	long im_event_mask;
	XIMStyles *im_supported_styles = NULL;
	XIMStyle my_style = 0;
#endif

XtTranslations XCursesTranslations;

char *defaultTranslations =
{
	"<Key>: XCursesKeyPress() \n" \
	"<Btn1Down>: XCursesButton() \n" \
	"!Ctrl <Btn2Down>: XCursesButton() \n" \
	"!Shift <Btn2Down>: XCursesButton() \n" \
	"!Ctrl <Btn2Up>: XCursesButton() \n" \
	"!Shift <Btn2Up>: XCursesButton() \n" \
	"<Btn1Up>: XCursesButton() \n" \
	"<Btn2Down>,<Btn2Up>: XCursesPasteSelection() \n" \
	"<Btn3Up>: XCursesButton() \n" \
	"<Btn3Down>: XCursesButton() \n" \
	"<Btn4Up>: XCursesButton() \n" \
	"<Btn4Down>: XCursesButton() \n" \
	"<Btn5Up>: XCursesButton() \n" \
	"<Btn5Down>: XCursesButton() \n" \
	"<BtnMotion>: XCursesButton()"
};

#ifdef PDCDEBUG
void XCsay(const char *msg)
{
	PDC_LOG(("%s:%s", XCLOGMSG, msg));
}
#endif

#ifdef PDC_WIDE
static int to_utf8(char *outcode, int code)
{
	if (code < 0x80)
	{
		outcode[0] = code;
		return 1;
	}
	else
		if (code < 0x800)
		{
			outcode[0] = ((code & 0x07c0) >> 6) | 0xc0;
			outcode[1] = (code & 0x003f) | 0x80;
			return 2;
		}
		else
		{
			outcode[0] = ((code & 0xf000) >> 12) | 0xe0;
			outcode[1] = ((code & 0x0fc0) >> 6) | 0x80;
			outcode[2] = (code & 0x003f) | 0x80;
			return 3;
		}
}

# ifndef X_HAVE_UTF8_STRING
static Atom XA_UTF8_STRING(Display *dpy)
{
	static AtomPtr p = NULL;

	if (p == NULL)
		p = XmuMakeAtom("UTF8_STRING");

	return XmuInternAtom(dpy, p);
}
# endif
#endif

signal_handler XCursesSetSignal(int signo, signal_handler action)
{
#if defined(SA_INTERRUPT) || defined(SA_RESTART)
	struct sigaction sigact, osigact;

	sigact.sa_handler = action;

# ifdef SA_INTERRUPT
#  ifdef SA_RESTART
	sigact.sa_flags = SA_INTERRUPT | SA_RESTART;
#  else
	sigact.sa_flags = SA_INTERRUPT;
#  endif
# else	/* must be SA_RESTART */
	sigact.sa_flags = SA_RESTART;
# endif
	sigemptyset(&sigact.sa_mask);

	if (sigaction(signo, &sigact, &osigact) != 0)
		return SIG_ERR;

	return osigact.sa_handler;

#else	/* not SA_INTERRUPT or SA_RESTART, use plain signal */
	return signal(signo, action);
#endif
}

RETSIGTYPE XCursesSigwinchHandler(int signo)
{
	PDC_LOG(("%s:XCursesSigwinchHandler() - called: SIGNO: %d\n",
		XCLOGMSG, signo));

	/* Patch by: Georg Fuchs, georg.fuchs@rz.uni-regensburg.de 
	   02-Feb-1999 */

	SP->resized += 1;

	/* Always trap SIGWINCH if the C library supports SIGWINCH */

#ifdef SIGWINCH
	XCursesSetSignal(SIGWINCH, XCursesSigwinchHandler);
#endif
}

static void makeXY(int x, int y, int fontwidth, int fontheight,
		   int *xpos, int *ypos)
{
	*xpos = (x * fontwidth) + XCURSESBORDERWIDTH;
	*ypos = XCURSESNORMALFONTINFO->ascent + (y * fontheight) + 
		XCURSESBORDERWIDTH;
}

static int XCursesNewPacket(chtype attr, bool rev, int len, int col, int row,
#ifdef PDC_WIDE
			    XChar2b *text)
#else
			    char *text)
#endif
{
	GC gc;
	int pair_num, xpos, ypos;
	short fore, back;

	if ((pair_num = PAIR_NUMBER(attr)) != 0)
	{
		if (pair_content(pair_num, &fore, &back) == ERR)
			return ERR;
	}
	else
	{
		fore = COLOR_WHITE;
		back = COLOR_BLACK;
	}

#ifdef PDC_WIDE
	text[len].byte1 = 0;
	text[len].byte2 = 0;
#else
	text[len] = '\0';
#endif

	/* Specify the colour table offsets */

	fore += (attr & A_BOLD) ? 8 : 0;
	back += (attr & A_BLINK) ? 8 : 0;

	/* Reverse flag = highlighted selection XOR A_REVERSE set */

	rev ^= !!(attr & A_REVERSE);

	/* Determine which GC to use - normal or italic */

	gc = (attr & A_ITALIC) ? italic_gc : normal_gc;

	/* Draw it */

	XSetForeground(XCURSESDISPLAY, gc, colors[rev ? back : fore]);
	XSetBackground(XCURSESDISPLAY, gc, colors[rev ? fore : back]);

	makeXY(col, row, XCursesFontWidth, XCursesFontHeight, &xpos, &ypos);

#ifdef PDC_WIDE
	XDrawImageString16(
#else
	XDrawImageString(
#endif
		XCURSESDISPLAY, XCURSESWIN, gc, xpos, ypos, text, len);

	/* Underline, etc. */

	if (attr & (A_LEFTLINE|A_RIGHTLINE|A_UNDERLINE))
	{
		int k;

		XSetForeground(XCURSESDISPLAY, gc, colors[SP->line_color]);

		if (attr & A_UNDERLINE)		/* UNDER */
			XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
				xpos, ypos + 1,
				xpos + XCursesFontWidth * len, ypos + 1);

		if (attr & A_LEFTLINE)		/* LEFT */
			for (k = 0; k < len; k++)
				XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
					xpos + XCursesFontWidth * k - 1,
					ypos - XCursesFontAscent,
					xpos + XCursesFontWidth * k - 1,
					ypos + XCursesFontDescent);

		if (attr & A_RIGHTLINE)		/* RIGHT */
			for (k = 0; k < len; k++)
				XDrawLine(XCURSESDISPLAY, XCURSESWIN, gc,
					xpos + XCursesFontWidth * (k + 1) - 1,
					ypos - XCursesFontAscent,
					xpos + XCursesFontWidth * (k + 1) - 1,
					ypos + XCursesFontDescent);
	}

	PDC_LOG(("%s:XCursesNewPacket() - row: %d col: %d "
		"num_cols: %d fore: %d back: %d text:<%s>\n",
		XCLOGMSG, row, col, len, fore, back, text));

	return OK;
}

int XCursesDisplayText(const chtype *ch, int row, int col, int num_cols,
			bool highlight)
{
#ifdef PDC_WIDE
	XChar2b text[513];
#else
	char text[513];
#endif
	chtype old_attr, attr;
	int i, j;

	PDC_LOG(("%s:XCursesDisplayText() - called: row: %d col: %d "
		"num_cols: %d\n", XCLOGMSG, row, col, num_cols));

	if (num_cols == 0)
		return OK;

	old_attr = *ch & A_ATTRIBUTES;

	for (i = 0, j = 0; j < num_cols; j++)
	{
		chtype curr = ch[j];

#ifndef PDC_WIDE
		/* Special handling for ACS_BLOCK */

		if (!(curr & A_CHARTEXT))
		{
			curr |= ' ';
			curr ^= A_REVERSE;
		}
#endif
		attr = curr & A_ATTRIBUTES;

		if (attr != old_attr)
		{
			if (XCursesNewPacket(old_attr, highlight, i, 
			    col, row, text) == ERR)
				return ERR;

			old_attr = attr;
			col += i;
			i = 0;
		}

#ifdef PDC_WIDE
		text[i].byte1 = (curr & 0xff00) >> 8;
		text[i++].byte2 = curr & 0x00ff;
#else
		text[i++] = curr & A_CHARTEXT;
#endif
	}

	return XCursesNewPacket(old_attr, highlight, i, col, row, text);
}

void get_GC(Display *display, Window win, GC *gc, XFontStruct *font_info,
	    int fore, int back, bool highlight)
{
	XGCValues values;

        /* Create default Graphics Context */

	*gc = XCreateGC(display, win, 0L, &values);

        /* specify font */

	XSetFont(display, *gc, font_info->fid);

        /* specify black foreground since default may be white on white */

	XSetForeground(display, *gc, colors[fore]);
	XSetBackground(display, *gc, colors[back]);

	if (highlight)
		XSetFunction(display, *gc, GXxor);
}

int get_colors(void)
{
	XC_LOG(("in get_colors\n"));

	colors[0] = app_data.colorBlack;
	colors[1] = app_data.colorRed;
	colors[2] = app_data.colorGreen;
	colors[3] = app_data.colorYellow;
	colors[4] = app_data.colorBlue;
	colors[5] = app_data.colorMagenta;
	colors[6] = app_data.colorCyan;
	colors[7] = app_data.colorWhite;
	colors[8] = app_data.colorBoldBlack;
	colors[9] = app_data.colorBoldRed;
	colors[10] = app_data.colorBoldGreen;
	colors[11] = app_data.colorBoldYellow;
	colors[12] = app_data.colorBoldBlue;
	colors[13] = app_data.colorBoldMagenta;
	colors[14] = app_data.colorBoldCyan;
	colors[15] = app_data.colorBoldWhite;
	colors[COLOR_CURSOR] = XCURSESCURSORCOLOR;
	colors[COLOR_BORDER] = XCURSESBORDERCOLOR;

	XC_LOG(("out of get_colors\n"));

	return OK;
}

void XCursesEndwin(void)
{
	PDC_LOG(("%s:XCursesEndwin() - called\n", XCLOGMSG));

	if (bitmap_file != NULL)
	{
		XFreePixmap(XCURSESDISPLAY, icon_bitmap);
		free(bitmap_file);
	}

#ifdef HAVE_XPM_H
	if (pixmap_file != NULL)
	{
		XFreePixmap(XCURSESDISPLAY, icon_pixmap);
		XFreePixmap(XCURSESDISPLAY, icon_pixmap_mask);
		free(pixmap_file);
	}
#endif
	XFreeGC(XCURSESDISPLAY, normal_gc);
	XFreeGC(XCURSESDISPLAY, italic_gc);
	XFreeGC(XCURSESDISPLAY, block_cursor_gc);
	XFreeGC(XCURSESDISPLAY, rect_cursor_gc);
	XFreeGC(XCURSESDISPLAY, border_gc);
#ifdef FOREIGN
	XDestroyIC(Xic);
#endif
}

int XCursesRefreshScrollbar(void)
{
	PDC_SCROLLBAR_TYPE total_y = SP->sb_total_y;
	PDC_SCROLLBAR_TYPE total_x = SP->sb_total_x;

	PDC_LOG(("%s:XCursesRefreshScrollbar() - called: \n", XCLOGMSG));

	if (!SP->sb_on)
		return ERR;

	if (total_y != 0)
		XawScrollbarSetThumb(scrollVert,
			(PDC_SCROLLBAR_TYPE)(SP->sb_cur_y) / total_y,
			(PDC_SCROLLBAR_TYPE)(SP->sb_viewport_y) / total_y);

	if (total_x != 0)
		XawScrollbarSetThumb(scrollHoriz,
			(PDC_SCROLLBAR_TYPE)(SP->sb_cur_x) / total_x,
			(PDC_SCROLLBAR_TYPE)(SP->sb_viewport_x) / total_x);

	return OK;
}

void SetCursorColor(chtype *ch, short *fore, short *back)
{
	int attr;
	short f, b;

	attr = PAIR_NUMBER(*ch);

	if (attr)
	{
		pair_content(attr, &f, &b);
		*fore = 7 - f;
		*back = 7 - b;
	}
	else
	{
		if (*ch & A_REVERSE)
		{
			*back = COLOR_BLACK;
			*fore = COLOR_WHITE;
		}
		else
		{
			*back = COLOR_WHITE;
			*fore = COLOR_BLACK;
		}
	}
}

void XCursesGetIcon(void)
{
	XIconSize *icon_size;
	int size_count = 0;
	Status rc;
	unsigned char *bitmap_bits = NULL;
	unsigned int icon_bitmap_width = 0, icon_bitmap_height = 0;
	unsigned int file_bitmap_width = 0, file_bitmap_height = 0;

	PDC_LOG(("%s:XCursesGetIcon\n", XCLOGMSG));

	icon_size = XAllocIconSize();

	rc = XGetIconSizes(XtDisplay(topLevel), 
		RootWindowOfScreen(XtScreen(topLevel)),
		&icon_size, &size_count);

	/* if the WM can advise on icon sizes... */

	if (rc && size_count)
	{
		int i;
		int max_height = 0, max_width = 0;

		PDC_LOG(("%s:size_count: %d rc: %d\n",
			XCLOGMSG, size_count, rc));

		for (i = 0; i < size_count; i++)
		{
			if (icon_size[i].max_width > max_width)
				max_width = icon_size[i].max_width;
			if (icon_size[i].max_height > max_height)
				max_height = icon_size[i].max_height;

			PDC_LOG(("%s:min: %d %d\n", XCLOGMSG,
				icon_size[i].min_width,
				icon_size[i].min_height));

			PDC_LOG(("%s:max: %d %d\n", XCLOGMSG,
				icon_size[i].max_width,
				icon_size[i].max_height));

			PDC_LOG(("%s:inc: %d %d\n", XCLOGMSG,
				icon_size[i].width_inc,
				icon_size[i].height_inc));
		}

		if (max_width >= big_icon_width &&
		    max_height >= big_icon_height)
		{
			icon_bitmap_width = big_icon_width;
			icon_bitmap_height = big_icon_height;
			bitmap_bits = (unsigned char *)big_icon_bits;
		}
		else
		{
			icon_bitmap_width = little_icon_width;
			icon_bitmap_height = little_icon_height;
			bitmap_bits = (unsigned char *)little_icon_bits;
		}

	}
	else  /* use small icon */
	{
		icon_bitmap_width = little_icon_width;
		icon_bitmap_height = little_icon_height;
		bitmap_bits = (unsigned char *)little_icon_bits;
	}

	XFree((char *)icon_size);

#ifdef HAVE_XPM_H
	if (strcmp(XCURSESPIXMAPFILE, "") != 0) /* supplied pixmap */
	{
		XpmReadFileToPixmap(XtDisplay(topLevel),
			RootWindowOfScreen(XtScreen(topLevel)),
			(char *)XCURSESPIXMAPFILE,
			&icon_pixmap, &icon_pixmap_mask, NULL);
		return;
	}
#endif

	if (strcmp(XCURSESBITMAPFILE, "") != 0) /* supplied bitmap */
	{
		int x_hot = 0, y_hot = 0;

		rc = XReadBitmapFile(XtDisplay(topLevel),
			RootWindowOfScreen(XtScreen(topLevel)),
			(char *)XCURSESBITMAPFILE,
			&file_bitmap_width, &file_bitmap_height,
			&icon_bitmap, &x_hot, &y_hot);

		switch(rc)
		{
		case BitmapOpenFailed:
			fprintf(stderr, "bitmap file %s: not found\n",
				XCURSESBITMAPFILE);
			break;
		case BitmapFileInvalid:
			fprintf(stderr, "bitmap file %s: contents invalid\n",
				XCURSESBITMAPFILE);
			break;
		default:
			return;
		}
	}

	icon_bitmap = XCreateBitmapFromData(XtDisplay(topLevel),
		RootWindowOfScreen(XtScreen(topLevel)),
		(char *)bitmap_bits, icon_bitmap_width, icon_bitmap_height);
}

/* This function redraws the entire screen. */

static void XCursesDisplayScreen(void)
{
	int row;

	PDC_LOG(("%s:XCursesDisplayScreen() - called:\n", XCLOGMSG));

	for (row = 0; row < XCursesLINES; row++)
	{
		get_line_lock(row);

		XCursesDisplayText((const chtype *)(Xcurscr + 
			XCURSCR_Y_OFF(row)), row, 0, COLS, FALSE);

		release_line_lock(row);
	}

	XCursesDisplayCursor(SP->cursrow, SP->curscol, SP->cursrow, 
		SP->curscol);

	/* Draw the border if required */

	if (XCURSESBORDERWIDTH)
		XDrawRectangle(XCURSESDISPLAY, XCURSESWIN, border_gc,
			XCURSESBORDERWIDTH / 2, XCURSESBORDERWIDTH / 2,
			XCursesWindowWidth - XCURSESBORDERWIDTH,
			XCursesWindowHeight - XCURSESBORDERWIDTH);
}

/* This function draws those portions of the screen that have changed. */

static void XCursesRefreshScreen(void)
{
	int row, start_col, num_cols;

	PDC_LOG(("%s:XCursesRefreshScreen() - called:\n", XCLOGMSG));

	for (row = 0; row < XCursesLINES; row++)
	{
		num_cols = (int)*(Xcurscr + XCURSCR_LENGTH_OFF + row);

		if (num_cols != 0)
		{
			get_line_lock(row);

			start_col = (int)*(Xcurscr + XCURSCR_START_OFF + row);

			XCursesDisplayText((const chtype *)(Xcurscr + 
				XCURSCR_Y_OFF(row) + (start_col * 
				sizeof(chtype))), row, start_col, 
				num_cols, FALSE);

			*(Xcurscr + XCURSCR_LENGTH_OFF + row) = 0;

			release_line_lock(row);
		}
	}

	if (mouse_selection)
		SelectionOff();
}

void XCursesExpose(Widget w, XtPointer client_data, XEvent *event,
		   Boolean *continue_to_dispatch)
{
	PDC_LOG(("%s:XCursesExpose called\n", XCLOGMSG));

	/* ignore all Exposes except last */

	if (event->xexpose.count != 0)
		return;

	if (after_first_curses_request && ReceivedMapNotify)
		XCursesDisplayScreen();
}

void XCursesNonmaskable(Widget w, XtPointer client_data, XEvent *event,
			Boolean *continue_to_dispatch)
{
	XClientMessageEvent *client_event = (XClientMessageEvent *)event;

	PDC_LOG(("%s:XCursesNonmaskable called: otherpid %d event %d\n",
		XCLOGMSG, otherpid, event->type));

	if (event->type == ClientMessage)
	{
		XC_LOG(("ClientMessage received\n"));

		/* This code used to include handling of 
		   WM_SAVE_YOURSELF, but it resulted in continual 
		   failure of THE on my Toshiba laptop. Removed on 
		   3-3-2001. Now only exits on WM_DELETE_WINDOW. */

		if ((Atom)client_event->data.s[0] == wm_atom[0])
			XCursesExitXCursesProcess(0, SIGKILL, "");
	}
}

void XCursesModifierPress(Widget w, XEvent *event, String *params, 
			  Cardinal *nparams)
{
#ifdef FOREIGN
	wchar_t buffer[120];
#else
	char buffer[120];
#endif
	int buflen = 40;
	int count, key;
	KeySym keysym;
	XComposeStatus compose;

	PDC_LOG(("%s:XCursesModifierPress called\n", XCLOGMSG));

	buffer[0] = '\0';

	count = XLookupString(&(event->xkey), buffer, buflen, &keysym, 
		&compose);

	/* Handle modifier keys first */

	SP->return_key_modifiers = True;

	switch (keysym) {
	case XK_Shift_L:
		key = KEY_SHIFT_L;
		break;
	case XK_Shift_R:
		key = KEY_SHIFT_R;
		break;
	case XK_Control_L:
		key = KEY_CONTROL_L;
		break;
	case XK_Control_R:
		key = KEY_CONTROL_R;
		break;
	case XK_Alt_L:
		key = KEY_ALT_L;
		break;
	case XK_Alt_R:
		key = KEY_ALT_R;
		break;
	default:
		key = 0;
	}

	if (key)
		XCursesSendKeyToCurses((unsigned long)key, NULL);
}

void XCursesKeyPress(Widget w, XEvent *event, String *params,
		     Cardinal *nparams)
{
	enum { STATE_NORMAL, STATE_COMPOSE, STATE_CHAR };

#ifdef FOREIGN
	Status status;
	wchar_t buffer[120];
#else
	char buffer[120];
#endif
	int buflen = 40;
	int count, key, i;
	KeySym keysym;
	XComposeStatus compose;
	static int compose_state = STATE_NORMAL;
	static int compose_index = 0;
	int save_visibility = 0;
	int char_idx = 0;
	int xpos,ypos;
	chtype *ch;
	short fore = 0, back = 0;
	unsigned long modifier = 0;

	PDC_LOG(("%s:XCursesKeyPress called\n", XCLOGMSG));

	/* ignore KeyReleases */

	if (event->type == KeyRelease)
		return;

	buffer[0] = '\0';

#ifdef FOREIGN
	count = XwcLookupString(Xic, &(event->xkey), buffer, buflen, 
		&keysym, &status);
#else
	count = XLookupString(&(event->xkey), buffer, buflen, &keysym, 
		&compose);
#endif
	/* Handle modifier keys first */

	if (event->type == KeyPress && keysym != compose_key
	    && IsModifierKey(keysym) && SP->return_key_modifiers)
	{
		switch (keysym) {
		case XK_Shift_L:
			key = KEY_SHIFT_L;
			break;
		case XK_Shift_R:
			key = KEY_SHIFT_R;
			break;
		case XK_Control_L:
			key = KEY_CONTROL_L;
			break;
		case XK_Control_R:
			key = KEY_CONTROL_R;
			break;
		case XK_Alt_L:
			key = KEY_ALT_L;
			break;
		case XK_Alt_R:
			key = KEY_ALT_R;
			break;
		default:
			key = 0;
		}

		if (key)
			XCursesSendKeyToCurses((unsigned long)key, NULL);
		return;
	}

	/* translate keysym into curses key code */

	PDC_LOG(("%s:Key mask: %x\n", XCLOGMSG, event->xkey.state));

#ifdef PDCDEBUG
	for (key = 0; key < 4; key++)
		PDC_debug("%s:Keysym %x %d\n", XCLOGMSG,
			XKeycodeToKeysym(XCURSESDISPLAY, 
			event->xkey.keycode, key), key);
#endif
	key = 0;

	/* Check if the key just pressed is the user-specified compose 
	   key; if it is, set the compose state and exit. */

	if (keysym == compose_key)
	{
		/* Change the shape of the cursor to an outline 
		   rectangle to indicate we are in "compose" status */

		save_visibility = SP->visibility;
		SP->visibility = 0;

		XCursesDisplayCursor(SP->cursrow, SP->curscol, 
			SP->cursrow, SP->curscol);

		SP->visibility = save_visibility;
		makeXY(SP->curscol, SP->cursrow, XCursesFontWidth, 
			XCursesFontHeight, &xpos, &ypos);

		ch = (chtype *)(Xcurscr + XCURSCR_Y_OFF(SP->cursrow) + 
			(SP->curscol * sizeof(chtype)));

		SetCursorColor(ch, &fore, &back);

		XSetForeground(XCURSESDISPLAY, rect_cursor_gc, colors[back]);

		XDrawRectangle(XCURSESDISPLAY, XCURSESWIN, rect_cursor_gc, 
			xpos + 1, ypos - XCursesFontHeight + 
			XCURSESNORMALFONTINFO->descent + 1, 
			XCursesFontWidth - 2, XCursesFontHeight - 2);

		compose_state = STATE_COMPOSE;
		return;
	}

	switch(compose_state)
	{
	case STATE_COMPOSE:
		if (event->xkey.state & compose_mask)
		{
			compose_state = STATE_NORMAL;
			XCursesDisplayCursor(SP->cursrow, SP->curscol,
				SP->cursrow, SP->curscol);
			break;
		}

		if (buffer[0] != 0 && count == 1)
			key = (int)buffer[0];

		compose_index = -1;

         
		for (i = 0; i < (int)strlen(compose_chars); i++)
			if ((int)compose_chars[i] == key)
			{
				compose_index = i;
				break;
			}

		if (compose_index == -1)
		{
			compose_state = STATE_NORMAL;
			compose_index = 0;
			XCursesDisplayCursor(SP->cursrow, SP->curscol,
				SP->cursrow, SP->curscol);
			break;
		}

		compose_state = STATE_CHAR;
		return;

	case STATE_CHAR:
		if (event->xkey.state & compose_mask)
		{
			compose_state = STATE_NORMAL;
			XCursesDisplayCursor(SP->cursrow, SP->curscol,
				SP->cursrow, SP->curscol);
			break;
		}

		if (buffer[0] != 0 && count == 1)
			key = (int)buffer[0];

		char_idx = -1;

		for (i = 0; i < MAX_COMPOSE_CHARS; i++)
			if (compose_lookups[compose_index][i] == key)
			{
				char_idx = i;
				break;
			}

		if (char_idx == -1)
		{
			compose_state = STATE_NORMAL;
			compose_index = 0;
			XCursesDisplayCursor(SP->cursrow, SP->curscol,
				SP->cursrow, SP->curscol);
			break;
		}

		XCursesSendKeyToCurses(
			(unsigned long)compose_keys[compose_index][char_idx],
			NULL);

		compose_state = STATE_NORMAL;
		compose_index = 0;
         
		XCursesDisplayCursor(SP->cursrow, SP->curscol,
			SP->cursrow, SP->curscol);

		return;
	}

	/* To get here we are procesing "normal" keys */

	PDC_LOG(("%s:Keysym %x %d\n", XCLOGMSG,
		XKeycodeToKeysym(XCURSESDISPLAY, event->xkey.keycode, key),
		key));

	if (SP->save_key_modifiers)
	{
		/* 0x10: usually, numlock modifier */

		if (event->xkey.state & Mod2Mask)
			modifier |= PDC_KEY_MODIFIER_NUMLOCK;

		/* 0x01: shift modifier */

		if (event->xkey.state & ShiftMask)
			modifier |= PDC_KEY_MODIFIER_SHIFT;

		/* 0x04: control modifier */

		if (event->xkey.state & ControlMask)
			modifier |= PDC_KEY_MODIFIER_CONTROL;

		/* 0x08: usually, alt modifier */

		if (event->xkey.state & Mod1Mask)
			modifier |= PDC_KEY_MODIFIER_ALT;
	}

	for (i = 0; XCursesKeys[i].keycode != 0; i++)
	{
		if (XCursesKeys[i].keycode == keysym)
		{
			PDC_LOG(("%s:State %x\n", XCLOGMSG, event->xkey.state));

			/* ControlMask: 0x04: control modifier
			   Mod1Mask: 0x08: usually, alt modifier
			   Mod2Mask: 0x10: usually, numlock modifier
			   ShiftMask: 0x01: shift modifier */

			if (XCursesKeys[i].numkeypad && (event->xkey.state &
			    Mod2Mask || event->xkey.state & ShiftMask))
			{
				key = XCursesKeys[i].shifted;
				break;
			}

			if (event->xkey.state & ShiftMask)
			{
				key = XCursesKeys[i].shifted;
				break;
			}

			if (event->xkey.state & ControlMask)
			{
				key = XCursesKeys[i].control;
				break;
			}

			if (event->xkey.state & Mod1Mask)
			{
				key = XCursesKeys[i].alt;
				break;
			}

			/* To get here, we ignore all other modifiers */

			key = XCursesKeys[i].normal;
			break;
		}
	}

	if (key == 0 && buffer[0] != 0 && count == 1)
		key = (int)buffer[0];

	PDC_LOG(("%s:Key: %s pressed - %x Mod: %x\n", XCLOGMSG,
		XKeysymToString(keysym), key, event->xkey.state));

	/* Handle ALT letters and numbers */

	if (event->xkey.state == Mod1Mask)
	{
		if (key >= (int)'A' && key <= (int)'Z')
			key = ALT_A + (key - (int)('A'));

		if (key >= (int)'a' && key <= (int)'z')
			key = ALT_A + (key - (int)('a'));

		if (key >= (int)'0' && key <= (int)'9')
			key = ALT_0 + (key - (int)('0'));
	}

	/* After all that, send the key back to the application if is 
	   NOT zero. */

	if (key)
	{
		key = key | modifier << 24;

		XCursesSendKeyToCurses((unsigned long)key, NULL);
	}
}

void XCursesHandleString(Widget w, XEvent *event, String *params,
			 Cardinal *nparams)
{
	int i = 0;
	unsigned char *ptr = NULL;

	if (*nparams != 1)
		return;

	if ((*params)[0] == '0' && (*params)[1] == 'x' && (*params)[2] != '\0')
	{
		unsigned char c;
		int total = 0;
		char *p;

		for (p = *params + 2; (c = *p); p++)
		{
			total *= 16;

			if (isupper(c))
				c = tolower(c);

			if (c >= '0' && c <= '9')
				total += c - '0';
			else
				if (c >= 'a' && c <= 'f')
					total += c - 'a' + 10;
				else
					break;
		}

		if (c == '\0')
			XCursesSendKeyToCurses((unsigned long)total, NULL);
	}
	else
	{
		ptr = (unsigned char *)*params;

		for (i = 0; i < (int)strlen((char *)ptr); i++)
			XCursesSendKeyToCurses((unsigned long)(ptr[i]), NULL);
	}
}

void XCursesPasteSelection(Widget w, XButtonEvent *button_event)
{
	PDC_LOG(("%s:XCursesPasteSelection() - called\n", XCLOGMSG));

	XtGetSelectionValue(w, XA_PRIMARY, XA_STRING, 
		XCursesRequestorCallbackForPaste, 
		(XtPointer)button_event, button_event->time);
}

void XCursesRequestorCallbackForPaste(Widget w, XtPointer data,
				      Atom *selection, Atom *type, 
				      XtPointer value,
				      unsigned long *length, int *format)
{
	unsigned long i, key;
	char *string = (char *)value;

	PDC_LOG(("%s:XCursesRequestorCallbackForPaste() - called\n",
		XCLOGMSG));

	if ((value == NULL) && (*length == 0))
		return;

	for (i = 0; i < (*length); i++)
	{
		key = (unsigned long)(string[i]);

		if (key == 10)		/* new line - convert to ^M */
			key = 13;

		XCursesSendKeyToCurses(key, NULL);
	}
}

Boolean XCursesConvertProc(Widget w, Atom *selection, Atom *target,
			   Atom *type_return, XtPointer *value_return,
			   unsigned long *length_return, int *format_return)
{
	PDC_LOG(("%s:XCursesConvertProc() - called\n", XCLOGMSG));

	if (*target == XA_TARGETS(XtDisplay(topLevel)))
	{
		XSelectionRequestEvent *req = XtGetSelectionRequest(w, 
			*selection, (XtRequestId)NULL);

		Atom *targetP;
		XPointer std_targets;
		unsigned long std_length;

		XmuConvertStandardSelection(topLevel, req->time, 
			selection, target, type_return, &std_targets, 
			&std_length, format_return);

		*length_return = std_length +
#ifdef PDC_WIDE
			2;
#else
			1;
#endif
		*value_return = XtMalloc(sizeof(Atom) * (*length_return));

		targetP = *(Atom**)value_return;
		*targetP++ = XA_STRING;
#ifdef PDC_WIDE
		*targetP++ = XA_UTF8_STRING(XtDisplay(topLevel));
#endif

		memmove((void *)targetP, (const void *)std_targets,
			sizeof(Atom) * std_length);

		XtFree((char *)std_targets);
		*type_return = XA_ATOM;
		*format_return = sizeof(Atom) * 8;

		return True;
	}
	else if (
#ifdef PDC_WIDE
		 *target == XA_UTF8_STRING(XtDisplay(topLevel)) ||
#endif
		 *target == XA_STRING)
	{
#ifdef PDC_WIDE
		bool utf8 = (*target == XA_UTF8_STRING(XtDisplay(topLevel)));
#endif
		char *data = XtMalloc(tmpsel_length
#ifdef PDC_WIDE
			* 3
#endif
			+ 1);

		chtype *tmp = tmpsel;
		int ret_length = 0;

#ifdef PDC_WIDE
		if (utf8)
		{
			while (*tmp)
				ret_length += to_utf8(data + ret_length,
					(int)(*tmp++ & A_CHARTEXT));
		}
		else
#endif
			while (*tmp)
				data[ret_length++] = *tmp++ & 0xff;

		*value_return = data;
		*length_return = ret_length;
		*format_return = 8;
		*type_return = *target;

		return True;
	}
	else
		return XmuConvertStandardSelection(topLevel, CurrentTime, 
		    selection, target, type_return, (XPointer*)value_return,
		    length_return, format_return);
}

void XCursesLoseOwnership(Widget w, Atom *type)
{
	PDC_LOG(("%s:XCursesLoseOwnership() - called\n", XCLOGMSG));

	if (tmpsel)
		free(tmpsel);

	tmpsel = NULL;
	tmpsel_length = 0;
	SelectionOff();
}

void ShowSelection(int start_x, int start_y, int end_x, int end_y,
		   bool highlight)
{
	int i, num_cols, start_col, row;

	PDC_LOG(("%s:ShowSelection() - called StartX: %d StartY: %d "
		"EndX: %d EndY: %d Highlight: %d\n", XCLOGMSG,
		start_x, start_y, end_x, end_y, highlight));

	for (i = 0; i < end_y - start_y + 1; i++)
	{

		if (start_y == end_y)		/* only one line */
		{
			start_col = start_x;
			num_cols = end_x - start_x + 1;
			row = start_y;
		}
		else if (i == 0)		/* first line */
		{
			start_col = start_x;
			num_cols = COLS - start_x;
			row = start_y;
		}
		else if (start_y + i == end_y)	/* last line */
		{
			start_col = 0;
			num_cols = end_x + 1;
			row = end_y;
		}
		else				/* full line */
		{
			start_col = 0;
			num_cols = COLS;
			row = start_y + i;
		}

		get_line_lock(row);

		XCursesDisplayText((const chtype *)(Xcurscr + 
			XCURSCR_Y_OFF(row) + (start_col * sizeof(chtype))),
			row, start_col, num_cols, highlight);

		release_line_lock(row);
	}
}

void SelectionOff(void)
{
	PDC_LOG(("%s:SelectionOff() - called\n", XCLOGMSG));

	XCursesDisplayScreen();

	selection_start_x = selection_start_y = selection_end_x = 
		selection_end_y = 0;

	mouse_selection = False;
}

void SelectionOn(int x, int y)
{
	PDC_LOG(("%s:SelectionOn() - called\n", XCLOGMSG));

	selection_start_x = selection_end_x = x;
	selection_start_y = selection_end_y = y;
}

void SelectionExtend(int x, int y)
{
	int temp, current_start, current_end, current_start_x, 
		current_end_x, current_start_y, current_end_y,
		new_start, new_end, new_start_x, new_end_x, new_start_y, 
		new_end_y;

	PDC_LOG(("%s:SelectionExtend() - called\n", XCLOGMSG));

	mouse_selection = True;

	/* convert x/y coordinates into start/stop */

	current_start = (selection_start_y * COLS) + selection_start_x;
	current_end = (selection_end_y * COLS) + selection_end_x;

	if (current_start > current_end)
	{
		current_start_x = selection_end_x;
		current_start_y = selection_end_y;
		current_end_x = selection_start_x;
		current_end_y = selection_start_y;
		temp = current_start;
		current_start = current_end;
		current_end = temp;
	}
	else
	{
		current_end_x = selection_end_x;
		current_end_y = selection_end_y;
		current_start_x = selection_start_x;
		current_start_y = selection_start_y;
	}

	/* Now we have the current selection as a linear expression. 
	   Convert the new position to a linear expression. */

	selection_end_x = x;
	selection_end_y = y;

	/* convert x/y coordinates into start/stop */

	new_start = (selection_start_y * COLS) + selection_start_x;
	new_end = (selection_end_y * COLS) + selection_end_x;

	if (new_start > new_end)
	{
		new_start_x = selection_end_x;
		new_start_y = selection_end_y;
		new_end_x = selection_start_x;
		new_end_y = selection_start_y;
		temp = new_start;
		new_start = new_end;
		new_end = temp;
	}
	else
	{
		new_end_x = selection_end_x;
		new_end_y = selection_end_y;
		new_start_x = selection_start_x;
		new_start_y = selection_start_y;
	}

	if (new_end > current_end)
		ShowSelection(current_end_x, current_end_y, new_end_x, 
			new_end_y, TRUE);
	else if (new_end < current_end)
		ShowSelection(new_end_x, new_end_y, current_end_x, 
			current_end_y, FALSE);
	else if (new_start < current_start)
		ShowSelection(new_start_x, new_start_y, current_start_x, 
			current_start_y, TRUE);
	else if (new_start > current_start)
		ShowSelection(current_start_x, current_start_y, 
			new_start_x, new_start_y, FALSE);
	else
		ShowSelection(current_start_x, current_start_y, 
			new_start_x, new_start_y, TRUE);
}

void SelectionSet(void)
{
	int i, j, start, end, start_x, end_x, start_y, end_y, num_cols, 
		start_col, row, num_chars, ch, last_nonblank, length, newlen;
	chtype *ptr = NULL;

	PDC_LOG(("%s:SelectionSet() - called\n", XCLOGMSG));

	/* convert x/y coordinates into start/stop */

	start = (selection_start_y * COLS) + selection_start_x;
	end = (selection_end_y * COLS) + selection_end_x;

	if (start == end)
	{
		if (tmpsel)
			free(tmpsel);

		tmpsel = NULL;
		tmpsel_length = 0;

		return;
	}

	if (start > end)
	{
		start_x = selection_end_x;
		start_y = selection_end_y;
		end_x = selection_start_x;
		end_y = selection_start_y;
		length = start - end + 1;
	}
	else
	{
		end_x = selection_end_x;
		end_y = selection_end_y;
		start_x = selection_start_x;
		start_y = selection_start_y;
		length = end - start + 1;
	}

	newlen = length + end_y - start_y + 2;

	if (length > (int)tmpsel_length)
	{
		if (tmpsel_length == 0)
			tmpsel = (chtype *)malloc(newlen * sizeof(chtype));
		else
			tmpsel = (chtype *)realloc(tmpsel,
				newlen * sizeof(chtype));
	}

	if (!tmpsel)
	{
		tmpsel_length = 0;
		return;
	}

	tmpsel_length = length;
	num_chars = 0;

	for (i = 0; i < end_y - start_y + 1; i++)
	{

		if (start_y == end_y)		/* only one line */
		{
			start_col = start_x;
			num_cols = end_x - start_x + 1;
			row = start_y;
		}
		else if (i == 0)		/* first line */
		{
			start_col = start_x;
			num_cols = COLS - start_x;
			row = start_y;
		}
		else if (start_y + i == end_y)	/* last line */
		{
			start_col = 0;
			num_cols = end_x + 1;
			row = end_y;
		}
		else				/* full line */
		{
			start_col = 0;
			num_cols = COLS;
			row = start_y + i;
		}

		get_line_lock(row);

		ptr = (chtype *)(Xcurscr + XCURSCR_Y_OFF(row) +
			start_col * sizeof(chtype));

		if (i < end_y - start_y)
		{
			last_nonblank = 0;

			for (j = 0; j < num_cols; j++)
			{
				ch = (int)(ptr[j] & A_CHARTEXT);
				if (ch != (int)' ')
					last_nonblank = j;
			}
		}
		else
			last_nonblank = num_cols - 1;

		for (j = 0; j <= last_nonblank; j++)
			tmpsel[num_chars++] = ptr[j];

		release_line_lock(row);

		if (i < end_y - start_y)
			tmpsel[num_chars++] = '\n';
	}

	tmpsel[num_chars] = '\0';
	tmpsel_length = num_chars;
}

void XCursesDisplayCursor(int old_row, int old_x, int new_row, int new_x)
{
	int xpos, ypos, i;
	char buf[2];
	chtype *ch;
	short fore = 0, back = 0;

	PDC_LOG(("%s:XCursesDisplayCursor() - draw char at row: %d col %d\n",
		XCLOGMSG, old_row, old_x));

	/* if the cursor position is outside the boundary of the screen, 
	   ignore the request */

	if (old_row >= XCursesLINES || old_x >= COLS ||
	    new_row >= XCursesLINES || new_x >= COLS)
		return;

	/* display the character at the current cursor position */

	PDC_LOG(("%s:XCursesDisplayCursor() - draw char at row: %d col %d\n",
		XCLOGMSG, old_row, old_x));

	XCursesDisplayText((const chtype *)(Xcurscr + (XCURSCR_Y_OFF(old_row) 
		+ (old_x * sizeof(chtype)))), old_row, old_x, 1, FALSE);

	/* display the cursor at the new cursor position */

	switch(SP->visibility)
	{
	case 0:		/* cursor not displayed, no more to do */
		break;

	case 1:		/* cursor visibility normal */

		makeXY(new_x, new_row, XCursesFontWidth, 
			XCursesFontHeight, &xpos, &ypos);

		ch = (chtype *)(Xcurscr + XCURSCR_Y_OFF(new_row) + 
			new_x * sizeof(chtype));

		SetCursorColor(ch, &fore, &back);
		XSetForeground(XCURSESDISPLAY, rect_cursor_gc, colors[back]);

		if (vertical_cursor)
			XDrawLine(XCURSESDISPLAY, XCURSESWIN, rect_cursor_gc,
				xpos + 1,
				ypos - XCURSESNORMALFONTINFO->ascent,
				xpos + 1, 
				ypos - XCURSESNORMALFONTINFO->ascent + 
					XCursesFontHeight - 1);
		else
		    for (i = 0; i < XCURSESNORMALFONTINFO->descent + 2; i++)
			XDrawLine(XCURSESDISPLAY, XCURSESWIN, rect_cursor_gc,
				xpos,
				ypos - 2 + i,
				xpos + XCursesFontWidth, 
				ypos - 2 + i);

		PDC_LOG(("%s:XCursesDisplayCursor() - draw line at "
			"row %d col %d\n", XCLOGMSG, new_row, new_x));

		break;

	default:	/* cursor visibility high */

		makeXY(new_x, new_row, XCursesFontWidth, 
			XCursesFontHeight, &xpos, &ypos);

		ch = (chtype *)(Xcurscr + XCURSCR_Y_OFF(new_row) +
			new_x * sizeof(chtype));

		SetCursorColor(ch, &fore, &back);

		if (vertical_cursor)
		{
			XSetForeground(XCURSESDISPLAY, rect_cursor_gc, 
				colors[back]);

			XDrawLine(XCURSESDISPLAY, XCURSESWIN, rect_cursor_gc,
				xpos + 1,
				ypos - XCURSESNORMALFONTINFO->ascent,
				xpos + 1,
				ypos - XCURSESNORMALFONTINFO->ascent + 
					XCursesFontHeight - 1);

			XDrawLine(XCURSESDISPLAY, XCURSESWIN, rect_cursor_gc,
				xpos + 2,
				ypos - XCURSESNORMALFONTINFO->ascent,
				xpos + 2,
				ypos - XCURSESNORMALFONTINFO->ascent + 
					XCursesFontHeight - 1);
		}
		else
		{
			buf[0] =  (char)(*ch & A_CHARTEXT);
			buf[1] = '\0';

			XSetForeground(XCURSESDISPLAY, block_cursor_gc, 
				colors[fore]);
			XSetBackground(XCURSESDISPLAY, block_cursor_gc, 
				colors[back]);
			XDrawImageString(XCURSESDISPLAY, XCURSESWIN, 
				block_cursor_gc, xpos, ypos, buf, 1);
		}

		PDC_LOG(("%s:XCursesDisplayCursor() - draw cursor at "
			"row: %d col %d char <%s>\n",
			XCLOGMSG, new_row, new_x, buf));
	}
}

void XCursesEnterLeaveWindow(Widget w, XtPointer client_data, XEvent *event,
			     Boolean *continue_to_dispatch)
{
	PDC_LOG(("%s:XCursesEnterLeaveWindow called\n", XCLOGMSG));

	switch(event->type)
	{
	case EnterNotify:
		XC_LOG(("EnterNotify received\n"));

		windowEntered = TRUE;
		break;

	case LeaveNotify:
		XC_LOG(("LeaveNotify received\n"));

		windowEntered = FALSE;

		/* Display the cursor so it stays on while the window is 
		   not current */

		XCursesDisplayCursor(SP->cursrow, SP->curscol, 
			SP->cursrow, SP->curscol);
		break;

	default:
		PDC_LOG(("%s:XCursesEnterleaveWindow - unknown event %d\n",
			XCLOGMSG, event->type));
	}
}

void XCursesSendKeyToCurses(unsigned long key, MOUSE_STATUS *ms)
{
	PDC_LOG(("%s:XCursesSendKeyToCurses() - called: sending %d\n",
		XCLOGMSG, key));

	if (write_socket(key_sock, (char *)&key, sizeof(unsigned long)) < 0)
		XCursesExitXCursesProcess(1, SIGKILL,
			"exiting from XCursesSendKeyToCurses");

	if (ms != NULL)
	{
		MOUSE_LOG(("%s:writing mouse stuff\n", XCLOGMSG));

		if (write_socket(key_sock, (char *)&Mouse_status, 
		    sizeof(MOUSE_STATUS)) < 0)
			XCursesExitXCursesProcess(1, SIGKILL,
				"exiting from XCursesSendKeyToCurses");
	}
}

void XCursesCursorBlink(XtPointer unused, XtIntervalId *id)
{
	PDC_LOG(("%s:XCursesCursorBlink() - called:\n", XCLOGMSG));

	if (windowEntered)
	{
		if (visible_cursor)
		{
			/* Cursor currently ON, turn it off */

			int save_visibility = SP->visibility;
			SP->visibility = 0;
			XCursesDisplayCursor(SP->cursrow, SP->curscol,
				SP->cursrow, SP->curscol);
			SP->visibility = save_visibility;
			visible_cursor = 0;
		}
		else
		{
			/* Cursor currently OFF, turn it on */

			XCursesDisplayCursor(SP->cursrow, SP->curscol,
				SP->cursrow, SP->curscol);
			visible_cursor = 1;
		}
	}

	XtAppAddTimeOut(app_context, XCURSESCURSORBLINKRATE, 
		XCursesCursorBlink, NULL);
}

void XCursesButton(Widget w, XEvent *event, String *params, Cardinal *nparams)
{
	int button_no;
	static int last_button_no = 0;
	static Time last_button_press_time = 0;
	MOUSE_STATUS save_mouse_status;
	bool send_key=TRUE;
	static bool remove_release;
	static bool handle_real_release;

	PDC_LOG(("%s:XCursesButton called\n", XCLOGMSG));

	save_mouse_status = Mouse_status;
	button_no = event->xbutton.button;

	/* It appears that under X11R6 (at least on Linux), that an 
	   event_type of ButtonMotion does not include the mouse button 
	   in the event. The following code is designed to cater for 
	   this situation. */

	if (button_no == 0)
		button_no = last_button_no;

	last_button_no = button_no;

	Mouse_status.changes = 0;

	/* Handle button 4 and 5, which are normally mapped to 
	   the wheel mouse scroll up and down */

	if ((SP->_trap_mbe & MOUSE_WHEEL_SCROLL)
	    && (button_no == 4 || button_no == 5))
	{
		/* Send the KEY_MOUSE to curses program */

		memset(&Mouse_status, 0, sizeof(Mouse_status));

		Mouse_status.changes = (button_no == 5) ? 
			PDC_MOUSE_WHEEL_DOWN : PDC_MOUSE_WHEEL_UP;

		MOUSE_X_POS = MOUSE_Y_POS = -1;

		XCursesSendKeyToCurses((unsigned long)KEY_MOUSE, 
			&Mouse_status);
		return;
	}

	switch(event->type)
	{
	case ButtonPress:
		remove_release = False;
		handle_real_release = False;

		MOUSE_LOG(("\nButtonPress\n"));

		if ((int)(event->xbutton.time - last_button_press_time) <
		    XCURSESDOUBLECLICKPERIOD)
		{
			MOUSE_X_POS = save_mouse_status.x;
			MOUSE_Y_POS = save_mouse_status.y;
			BUTTON_STATUS(button_no) = BUTTON_DOUBLE_CLICKED;

			SelectionOff();

			if (!(SP->_trap_mbe & BUTTON1_DOUBLE_CLICKED)
			    && button_no == 1)
				send_key = FALSE;
			if (!(SP->_trap_mbe & BUTTON2_DOUBLE_CLICKED)
			    && button_no == 2)
				send_key = FALSE;
			if (!(SP->_trap_mbe & BUTTON3_DOUBLE_CLICKED)
			    && button_no == 3)
				send_key = FALSE;

			if (send_key)
				remove_release = True;
		}
		else
		{
			napms(XCURSESCLICKPERIOD);
			event->type = ButtonRelease;
			XSendEvent(event->xbutton.display, 
				event->xbutton.window, True, 0, event);
			last_button_press_time = event->xbutton.time;

			return;
		}

		last_button_press_time = event->xbutton.time;
		break;

	case MotionNotify:
		MOUSE_LOG(("\nMotionNotify: y: %d x: %d Width: %d "
			"Height: %d\n", event->xbutton.y, event->xbutton.x, 
			XCursesFontWidth, XCursesFontHeight));

		if (button_no == 1 && !(event->xbutton.state & ShiftMask)
		    && !(event->xbutton.state & ControlMask)
		    && !(event->xbutton.state & Mod1Mask))
		{
			MOUSE_X_POS = (event->xbutton.x - 
				XCURSESBORDERWIDTH) / XCursesFontWidth;
			MOUSE_Y_POS = (event->xbutton.y - 
				XCURSESBORDERWIDTH) / XCursesFontHeight;

			SelectionExtend(MOUSE_X_POS, MOUSE_Y_POS);
			send_key = FALSE;
		}
		else
			SelectionOff();

		/* Throw away mouse movements if they are in the same 
		   character position as the last mouse event, or if we 
		   are currently in the middle of a double click event. */

		MOUSE_X_POS = (event->xbutton.x - XCURSESBORDERWIDTH) / 
			XCursesFontWidth;
		MOUSE_Y_POS = (event->xbutton.y - XCURSESBORDERWIDTH) / 
			XCursesFontHeight;

		if ((MOUSE_X_POS == save_mouse_status.x &&
		     MOUSE_Y_POS == save_mouse_status.y) || 
		     save_mouse_status.button[button_no - 1] ==
			BUTTON_DOUBLE_CLICKED)
		{
			send_key = FALSE;
			break;
		}

		MOUSE_X_POS = (event->xbutton.x - XCURSESBORDERWIDTH) / 
			XCursesFontWidth;
		MOUSE_Y_POS = (event->xbutton.y - XCURSESBORDERWIDTH) / 
			XCursesFontHeight;

		Mouse_status.changes |= 8;

		if (!(SP->_trap_mbe & BUTTON1_MOVED) && button_no == 1)
			send_key = FALSE;
		if (!(SP->_trap_mbe & BUTTON2_MOVED) && button_no == 2)
			send_key = FALSE;
		if (!(SP->_trap_mbe & BUTTON3_MOVED) && button_no == 3)
			send_key = FALSE;
		break;

      case ButtonRelease:
		if (remove_release)
		{
			MOUSE_LOG(("Release at: %ld - removed\n",
				event->xbutton.time));
			return;
		}
		else
		{
		    if (!handle_real_release)
		    {
			if ((event->xbutton.time - 
			    last_button_press_time) < 100 && 
			    (event->xbutton.time != 
			    last_button_press_time))
			{
			    /* The "real" release was shorter than 
			       usleep() time therefore generate a click 
			       event */

			    MOUSE_LOG(("Release at: %ld - click\n",
				event->xbutton.time));

			    MOUSE_X_POS = (event->xbutton.x - 
				XCURSESBORDERWIDTH) / XCursesFontWidth;
			    MOUSE_Y_POS = (event->xbutton.y - 
				XCURSESBORDERWIDTH) / XCursesFontHeight;
			    BUTTON_STATUS(button_no) = BUTTON_CLICKED;

			    if (!(SP->_trap_mbe & BUTTON1_RELEASED)
				&& button_no == 1)
				    send_key = FALSE;

			    if (!(SP->_trap_mbe & BUTTON2_RELEASED)
				&& button_no == 2)
				    send_key = FALSE;

			    if (!(SP->_trap_mbe & BUTTON3_RELEASED)
				&& button_no == 3)
				    send_key = FALSE;

			    if (button_no == 1
				&& !(event->xbutton.state & ShiftMask)
				&& !(event->xbutton.state & ControlMask)
				&& !(event->xbutton.state & Mod1Mask)
				&& mouse_selection)
			    {
				send_key = FALSE;

				if (XtOwnSelection(topLevel, XA_PRIMARY, 
				    event->xbutton.time, XCursesConvertProc,
				    XCursesLoseOwnership, NULL) == False)
					SelectionOff();
			    }
			    else
				SelectionOff();

			    /* Ensure the "pseudo" release event is ignored */

			    remove_release = True;
			    handle_real_release = False;
			    break;
			}
			else
			{
			    /* Button release longer than usleep() time 
			       therefore generate a press and wait for 
			       the real release to occur later. */

			    MOUSE_LOG(("Generated Release at: %ld - "
				"press & release\n", event->xbutton.time));

			    MOUSE_X_POS = (event->xbutton.x - 
				XCURSESBORDERWIDTH) / XCursesFontWidth;
			    MOUSE_Y_POS = (event->xbutton.y - 
				XCURSESBORDERWIDTH) / XCursesFontHeight;
			    BUTTON_STATUS(button_no) = BUTTON_PRESSED;

			    if (!(SP->_trap_mbe & BUTTON1_PRESSED)
				&& button_no == 1)
				    send_key = FALSE;

			    if (!(SP->_trap_mbe & BUTTON2_PRESSED)
				&& button_no == 2)
				    send_key = FALSE;

			    if (!(SP->_trap_mbe & BUTTON3_PRESSED)
				&& button_no == 3)
				    send_key = FALSE;

			    if (button_no == 1
				&& !(event->xbutton.state & ShiftMask)
				&& !(event->xbutton.state & ControlMask)
				&& !(event->xbutton.state & Mod1Mask))
			    {
				SelectionOff();
				SelectionOn(MOUSE_X_POS, MOUSE_Y_POS);
			    }

			    handle_real_release = True;
			    break;
			}
		    }
		    else
		    {
			MOUSE_LOG(("Release at: %ld - released\n",
			    event->xbutton.time));
		    }
		}

		MOUSE_LOG(("\nButtonRelease\n"));

		MOUSE_X_POS = (event->xbutton.x - XCURSESBORDERWIDTH) / 
		    XCursesFontWidth;
		MOUSE_Y_POS = (event->xbutton.y - XCURSESBORDERWIDTH) / 
		    XCursesFontHeight;
		BUTTON_STATUS(button_no) = BUTTON_RELEASED;

		if (!(SP->_trap_mbe & BUTTON1_RELEASED) && button_no == 1)
		    send_key = FALSE;

		if (!(SP->_trap_mbe & BUTTON2_RELEASED) && button_no == 2)
		    send_key = FALSE;

		if (!(SP->_trap_mbe & BUTTON3_RELEASED) && button_no == 3)
		    send_key = FALSE;

		if (button_no == 1 && !(event->xbutton.state & ShiftMask)
		    && !(event->xbutton.state & ControlMask)
		    && !(event->xbutton.state & Mod1Mask) && mouse_selection)
		{
		    send_key = FALSE;

		    if (XtOwnSelection(topLevel, XA_PRIMARY, 
			event->xbutton.time, XCursesConvertProc, 
			XCursesLoseOwnership, NULL) == False)
			    SelectionOff();

		    SelectionSet();
		}
		else
		    SelectionOff();

		break;
	}

	/* Set up the mouse status fields in preparation for sending */

	Mouse_status.changes |= 1 << (button_no - 1);

	if (event->xbutton.state & ShiftMask)
		BUTTON_STATUS(button_no) |= BUTTON_SHIFT;
	if (event->xbutton.state & ControlMask)
		BUTTON_STATUS(button_no) |= BUTTON_CONTROL;
	if (event->xbutton.state & Mod1Mask)
		BUTTON_STATUS(button_no) |= BUTTON_ALT;

	/* If we are ignoring the event, or the mouse position is 
	   outside the bounds of the screen (because of the border), 
	   return here */

	MOUSE_LOG(("Button: %d x: %d y: %d Button status: %x "
		"Mouse status: %x\n", button_no, MOUSE_X_POS, MOUSE_Y_POS,
		BUTTON_STATUS(button_no), Mouse_status.changes));

	MOUSE_LOG(("Send: %d Button1: %x Button2: %x Button3: %x %d %d\n",
		send_key, BUTTON_STATUS(1), BUTTON_STATUS(2), 
		BUTTON_STATUS(3), XCursesLINES, XCursesCOLS));

	if (!send_key || MOUSE_X_POS < 0 || MOUSE_X_POS >= XCursesCOLS ||
	    MOUSE_Y_POS < 0 || MOUSE_Y_POS >= XCursesLINES)
		return;

	/* Send the KEY_MOUSE to curses program */

	XCursesSendKeyToCurses((unsigned long)KEY_MOUSE, &Mouse_status);
}

void Scroll_up_down(Widget w, XtPointer client_data, XtPointer call_data)
{
	int pixels = (long) call_data;
	int total_y = SP->sb_total_y * XCursesFontHeight;
	int viewport_y = SP->sb_viewport_y * XCursesFontHeight;
	int cur_y = SP->sb_cur_y * XCursesFontHeight;

	/* When pixels is negative, right button pressed, move data 
	   down, thumb moves up.  Otherwise, left button pressed, pixels 
	   positive, move data up, thumb down. */

	cur_y += pixels;

	/* limit panning to size of overall */

	if (cur_y < 0)
		cur_y = 0;
	else
		if (cur_y > (total_y - viewport_y))
			cur_y = total_y - viewport_y;

	SP->sb_cur_y = cur_y / XCursesFontHeight;

	XawScrollbarSetThumb(w, (double)((double)cur_y / (double)total_y),
		(double)((double)viewport_y / (double)total_y));

	/* Send a key: if pixels negative, send KEY_SCROLL_DOWN */

	XCursesSendKeyToCurses((unsigned long)KEY_SF, NULL);
}

void Scroll_left_right(Widget w, XtPointer client_data, XtPointer call_data)
{
	int pixels = (long) call_data;
	int total_x = SP->sb_total_x * XCursesFontWidth;
	int viewport_x = SP->sb_viewport_x * XCursesFontWidth;
	int cur_x = SP->sb_cur_x * XCursesFontWidth;

	/* When pixels is negative, right button pressed, move data 
	   down, thumb moves up.  Otherwise, left button pressed, pixels 
	   positive, move data up, thumb down. */

	cur_x += pixels;

	/* limit panning to size of overall */

	if (cur_x < 0)
		cur_x = 0;
	else
		if (cur_x > (total_x - viewport_x))
			cur_x = total_x - viewport_x;

	SP->sb_cur_x = cur_x / XCursesFontWidth;

	XawScrollbarSetThumb(w, (double)((double)cur_x / (double)total_x),
		(double)((double)viewport_x / (double)total_x));

	/* Send a key: if pixels negative, send KEY_SCROLL_DOWN */

	XCursesSendKeyToCurses((unsigned long)KEY_SR, NULL);
}

void Thumb_up_down(Widget w, XtPointer client_data, XtPointer call_data)
{
	double percent = *(double *) call_data;
	double total_y = (double)SP->sb_total_y;
	double viewport_y = (double)SP->sb_viewport_y;
	int cur_y = SP->sb_cur_y;

	/* If the size of the viewport is > overall area simply return, 
	   as no scrolling is permitted. */

	if (SP->sb_viewport_y >= SP->sb_total_y)
		return;

	if ((SP->sb_cur_y = (int)((double)total_y * percent)) >=
	    (total_y - viewport_y))
		SP->sb_cur_y = total_y - viewport_y;

	XawScrollbarSetThumb(w, (double)(cur_y / total_y),
		(double)(viewport_y / total_y));

	/* Send a key: if pixels negative, send KEY_SCROLL_DOWN */

	XCursesSendKeyToCurses((unsigned long)KEY_SF, NULL);
}

void Thumb_left_right(Widget w, XtPointer client_data, XtPointer call_data)
{
	double percent = *(double *) call_data;
	double total_x = (double)SP->sb_total_x;
	double viewport_x = (double)SP->sb_viewport_x;
	int cur_x = SP->sb_cur_x;

	/* If the size of the viewport is > overall area simply return, 
	   as no scrolling is permitted. */

	if (SP->sb_viewport_x >= SP->sb_total_x)
		return;

	if ((SP->sb_cur_x = (int)((float)total_x * percent)) >=
	    (total_x - viewport_x))
		SP->sb_cur_x = total_x - viewport_x;

	XawScrollbarSetThumb(w, (double)(cur_x / total_x),
		(double)(viewport_x / total_x));
    
	/* Send a key: if pixels negative, send KEY_SCROLL_DOWN */

	XCursesSendKeyToCurses((unsigned long)KEY_SR, NULL);
}

void XCursesExitXCursesProcess(int rc, int sig, char *msg)
{
	if (rc || sig)
	    fprintf(stderr,
		"%s:XCursesExitXCursesProcess() - called: rc:%d sig:%d <%s>\n",
		XCLOGMSG, rc, sig, msg);

	shmdt((char *)SP);
	shmdt((char *)Xcurscr);
	shmctl(shmidSP, IPC_RMID, 0);
	shmctl(shmid_Xcurscr, IPC_RMID, 0);

	XCursesEndwin();

	shutdown(display_sock, 2);
	close(display_sock);

	shutdown(exit_sock, 2);
	close(exit_sock);

	shutdown(key_sock, 2);
	close(key_sock);

	if (sig)
		kill(otherpid, sig);	/* to kill parent process */

	_exit(rc);
}

static void XCursesContinue(void)
{
	if (write_display_socket_int(CURSES_CONTINUE) < 0)
		XCursesExitXCursesProcess(4, SIGKILL,
			"exiting from XCursesProcessRequestsFromCurses");
}

void XCursesProcessRequestsFromCurses(XtPointer client_data, int *fid,
				      XtInputId *id) 
{ 
	int s, idx;
	int old_row, new_row;
	int old_x, new_x;
	int pos, num_cols;

	long length;

	char buf[12];		/* big enough for 2 integers */ 
	char title[1024];	/* big enough for window title */ 

	unsigned char save_atrtab[MAX_ATRTAB]; 

	PDC_LOG(("%s:XCursesProcessRequestsFromCurses()\n", XCLOGMSG));

	if (!ReceivedMapNotify) 
	    return; 

	FD_ZERO(&readfds); 
	FD_SET(display_sock, &readfds); 

	if ((s = select(FD_SETSIZE, (FD_SET_CAST)&readfds, NULL, 
	  NULL, &socket_timeout)) < 0)
	    XCursesExitXCursesProcess(2, SIGKILL, "exiting from "
		"XCursesProcessRequestsFromCurses - select failed");

	if (s == 0)	/* no requests pending - should never happen!*/ 
	    return; 

	if (FD_ISSET(display_sock, &readfds)) 
	{ 
	    /* read first integer to determine total message has 
	       been received */

	    PDC_LOG(("%s:XCursesProcessRequestsFromCurses() - "
		"before read_socket()\n", XCLOGMSG));

	    if (read_socket(display_sock, (char *)&num_cols, sizeof(int)) < 0) 
		XCursesExitXCursesProcess(3, SIGKILL, "exiting from "
		    "XCursesProcessRequestsFromCurses - first read");

	    PDC_LOG(("%s:XCursesProcessRequestsFromCurses() - "
		"after read_socket()\n", XCLOGMSG));

	    after_first_curses_request = True; 

	    switch(num_cols) 
	    { 
	    case CURSES_EXIT:	/* request from curses to stop */
		XC_LOG(("CURSES_EXIT received from child\n"));
		XCursesExitXCursesProcess(0, 0,
		    "XCursesProcess requested to exit by child");
		break;

	    case CURSES_BELL:
		XC_LOG(("CURSES_BELL received from child\n"));
		XBell(XCURSESDISPLAY, 50);
		break;

	    /* request from curses to confirm completion of display */ 

	    case CURSES_REFRESH:
		XC_LOG(("CURSES_REFRESH received from child\n"));
		XCursesRefreshScreen(); 
		XCursesContinue();
		break;

	    case CURSES_REFRESH_SCROLLBAR:
		XCursesRefreshScrollbar();
		break;

	    case CURSES_CURSOR:
		XC_LOG(("CURSES_CURSOR received from child\n"));
		if (read_socket(display_sock, buf, sizeof(int) * 2) < 0)
		    XCursesExitXCursesProcess(5, SIGKILL,
			"exiting from CURSES_CURSOR "
			"XCursesProcessRequestsFromCurses");

		memcpy((char *)&pos, buf, sizeof(int)); 

		old_row = pos & 0xFF;
		old_x = pos >> 8;
		idx = sizeof(int);
		memcpy((char *)&pos, buf + idx, sizeof(int));

		new_row = pos & 0xFF;
		new_x = pos >> 8;
		visible_cursor = 1;
		XCursesDisplayCursor(old_row, old_x, new_row, new_x);
		break;

	    case CURSES_DISPLAY_CURSOR:
		XC_LOG(("CURSES_DISPLAY_CURSOR received from child. "
			"Vis now: "));
		XC_LOG((visible_cursor ? "1\n" : "0\n"));

		/* If the window is not active, ignore this 
		   command. The cursor will stay solid. */

		if (windowEntered) 
		{ 
		    if (visible_cursor) 
		    { 
			/* Cursor currently ON, turn it off */

			int save_visibility = SP->visibility;
			SP->visibility = 0;

			XCursesDisplayCursor(SP->cursrow, SP->curscol, 
			    SP->cursrow, SP->curscol);

			SP->visibility = save_visibility;
			visible_cursor = 0;
		    } 
		    else 
		    { 
			/* Cursor currently OFF, turn it on */ 

			XCursesDisplayCursor(SP->cursrow, SP->curscol,
			    SP->cursrow, SP->curscol);

			visible_cursor = 1; 
		    } 
		} 

		break; 

	    case CURSES_TITLE:
		XC_LOG(("CURSES_TITLE received from child\n"));

		if (read_socket(display_sock, (char *)&pos, sizeof(int)) < 0)
		    XCursesExitXCursesProcess(5, SIGKILL,
			"exiting from CURSES_TITLE "
			"XCursesProcessRequestsFromCurses");

		if (read_socket(display_sock, title, pos) < 0)
		    XCursesExitXCursesProcess(5, SIGKILL,
			"exiting from CURSES_TITLE "
			"XCursesProcessRequestsFromCurses");

		XtVaSetValues(topLevel, XtNtitle, title, NULL);
		break;

	    case CURSES_RESIZE:
		after_first_curses_request = False;
		XC_LOG(("CURSES_RESIZE received from child\n"));

		SP->lines = XCursesLINES = ((resizeXCursesWindowHeight -
		    (2 * XCURSESBORDERWIDTH)) / XCursesFontHeight);

		LINES = XCursesLINES - SP->linesrippedoff - SP->slklines;

		SP->cols = COLS = XCursesCOLS = ((resizeXCursesWindowWidth -
		    (2 * XCURSESBORDERWIDTH)) / XCursesFontWidth);

		XCursesWindowWidth = resizeXCursesWindowWidth;
		XCursesWindowHeight = resizeXCursesWindowHeight;
		visible_cursor = 1;

		/* Draw the border if required */ 

		if (XCURSESBORDERWIDTH) 
		    XDrawRectangle(XCURSESDISPLAY, XCURSESWIN, border_gc,
			XCURSESBORDERWIDTH / 2, XCURSESBORDERWIDTH / 2,
			XCursesWindowWidth - XCURSESBORDERWIDTH,
			XCursesWindowHeight - XCURSESBORDERWIDTH);

		/* Detach and drop the current shared memory segment and 
		   create and attach to a new segment */

		memcpy(save_atrtab, atrtab, sizeof(save_atrtab));

		SP->XcurscrSize = XCURSCR_SIZE;
		shmdt((char *)Xcurscr);
		shmctl(shmid_Xcurscr, IPC_RMID, 0);

		if ((shmid_Xcurscr = shmget(shmkey_Xcurscr,
		    SP->XcurscrSize + XCURSESSHMMIN, 0700 | IPC_CREAT)) < 0)
		{ 
		    perror("Cannot allocate shared memory for curscr");
		    XCursesExitXCursesProcess(4, SIGKILL,
			"exiting from XCursesProcessRequestsFromCurses");
		} 

		Xcurscr = (unsigned char*)shmat(shmid_Xcurscr, 0, 0);
		memset(Xcurscr, 0, SP->XcurscrSize);
		atrtab = (unsigned char *)(Xcurscr + XCURSCR_ATRTAB_OFF);
		memcpy(atrtab, save_atrtab, sizeof(save_atrtab));

		XCursesContinue(); 
		break; 

	    case CURSES_GET_SELECTION:
		XC_LOG(("CURSES_GET_SELECTION received from child\n"));

		XCursesContinue();

		XtGetSelectionValue(topLevel, XA_PRIMARY, XA_STRING,
		    XCursesRequestorCallbackForGetSelection,
		    (XtPointer)NULL, 0);

		break;

	    case CURSES_SET_SELECTION:
		XC_LOG(("CURSES_SET_SELECTION received from child\n"));

		if (read_socket(display_sock, (char *)&length,
		    sizeof(long)) < 0)
			XCursesExitXCursesProcess(5, SIGKILL,
			    "exiting from CURSES_SET_SELECTION "
			    "XCursesProcessRequestsFromCurses");

		if (length > (long)tmpsel_length)
		{
		    if (tmpsel_length == 0)
			tmpsel = (chtype *)malloc((length + 1) * 
				sizeof(chtype));
		    else
			tmpsel = (chtype *)realloc(tmpsel, (length + 1) *
				sizeof(chtype));
		}

		if (!tmpsel)
		{
		    if (write_display_socket_int(PDC_CLIP_MEMORY_ERROR) < 0)
			XCursesExitXCursesProcess(4, SIGKILL,
			    "exiting from XCursesProcessRequestsFromCurses");
		    break;
		}

		if (read_socket(display_sock, (char *)tmpsel,
		    length * sizeof(chtype)) < 0)
			XCursesExitXCursesProcess(5, SIGKILL,
			    "exiting from CURSES_SET_SELECTION "
			    "XCursesProcessRequestsFromCurses");

		tmpsel_length = length;
		tmpsel[length] = '\0';

		if (XtOwnSelection(topLevel, XA_PRIMARY, CurrentTime,
		    XCursesConvertProc, XCursesLoseOwnership, NULL) == False)
		{
		    old_x = PDC_CLIP_ACCESS_ERROR;
		    free(tmpsel);
		    tmpsel = NULL;
		    tmpsel_length = 0;
		}
		else
		    old_x = PDC_CLIP_SUCCESS;

		SelectionOff();

		if (write_display_socket_int(old_x) < 0)
		    XCursesExitXCursesProcess(4, SIGKILL,
			"exiting from XCursesProcessRequestsFromCurses");
		break;

	    case CURSES_CLEAR_SELECTION:
		XC_LOG(("CURSES_CLEAR_SELECTION received from child\n"));

		XCursesContinue();

		SelectionOff();
		break;

	    default: 
		PDC_LOG(("%s:Unknown request %d\n", XCLOGMSG, num_cols));
	    } 
	} 
} 

int XCursesSetupX(int argc, char *argv[])
{
	static char *myargv[] = {"PDCurses", NULL};
	extern bool sb_started;

	int italic_font_valid;
	XColor pointerforecolor, pointerbackcolor;
	XrmValue rmfrom, rmto;
	int i = 0;
	int minwidth, minheight;

	PDC_LOG(("%s:XCursesSetupX called\n", XCLOGMSG));

	/* The following kludge is to force XtVaAppInitialize() to 
	   recognize the name of the program.  Without it, a default 
	   value of "PDCurses" is used. */

	if (!argv)
	{
		argv = myargv;
		argc = 1;
	}

	XCursesProgramName = argv[0];

	/* Keep open the 'write' end of the socket so the XCurses 
	   process can send a CURSES_EXIT to itself from within the 
	   signal handler */

	exit_sock = display_sockets[0];
	display_sock = display_sockets[1];

	close(key_sockets[0]);
	key_sock = key_sockets[1];

	/* Trap all signals when XCurses is the child process, but only 
	   if they haven't already been ignored by the application. */

	for (i = 0; i < PDC_MAX_SIGNALS; i++)
		if (XCursesSetSignal(i, XCursesSignalHandler) == SIG_IGN)
			XCursesSetSignal(i, SIG_IGN);

	/* Start defining X Toolkit things */

#if XtSpecificationRelease > 4
	XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL);
#endif

	/* Exit if no DISPLAY variable set */

	if (getenv("DISPLAY") == NULL)
	{
		fprintf(stderr, "Error: no DISPLAY variable set\n");
		kill(otherpid, SIGKILL);
		return ERR;
	}

	/* Initialise the top level widget */

	topLevel = XtVaAppInitialize(&app_context, XCursesClassName,
		options, XtNumber(options), &argc, argv, NULL, NULL);

	XtVaGetApplicationResources(topLevel, &app_data, app_resources,
		XtNumber(app_resources), NULL);

	/* Check application resource values here */

	XCursesFontWidth = XCURSESNORMALFONTINFO->max_bounds.rbearing - 
		XCURSESNORMALFONTINFO->min_bounds.lbearing;

	XCursesFontHeight = XCURSESNORMALFONTINFO->max_bounds.ascent + 
		XCURSESNORMALFONTINFO->max_bounds.descent;

	XCursesFontAscent = XCURSESNORMALFONTINFO->max_bounds.ascent;
	XCursesFontDescent = XCURSESNORMALFONTINFO->max_bounds.descent;

	/* Check that the italic font and normal fonts are the same size */
	/* This appears backwards */

	italic_font_valid = XCursesFontWidth !=
		XCURSESITALICFONTINFO->max_bounds.rbearing -
		XCURSESITALICFONTINFO->min_bounds.lbearing ||
		XCursesFontHeight !=
		XCURSESITALICFONTINFO->max_bounds.ascent + 
		XCURSESITALICFONTINFO->max_bounds.descent;

	/* Calculate size of display window */

	XCursesCOLS = XCURSESCOLS;
	XCursesLINES = XCURSESLINES;

	XCursesWindowWidth = XCursesFontWidth * XCursesCOLS +
		2 * XCURSESBORDERWIDTH;

	XCursesWindowHeight = XCursesFontHeight * XCursesLINES +
		2 * XCURSESBORDERWIDTH;

	minwidth = XCursesFontWidth * 2 + XCURSESBORDERWIDTH * 2;
	minheight = XCursesFontHeight * 2 + XCURSESBORDERWIDTH * 2;

	/* Set up the icon for the application; the default is an
	   internal one for XCurses. Then set various application level
	   resources. */

	XCursesGetIcon();

#ifdef HAVE_XPM_H
	if (XCURSESPIXMAPFILE != NULL && strcmp(XCURSESPIXMAPFILE, "") != 0)
		XtVaSetValues(topLevel, XtNminWidth, minwidth,
			XtNminHeight, minheight,
			XtNbaseWidth, XCURSESBORDERWIDTH * 2,
			XtNbaseHeight, XCURSESBORDERWIDTH * 2,
			XtNiconPixmap, icon_pixmap,
			XtNiconMask, icon_pixmap_mask, NULL);
	else
#endif
		XtVaSetValues(topLevel, XtNminWidth, minwidth, 
			XtNminHeight, minheight,
			XtNbaseWidth, XCURSESBORDERWIDTH * 2,
			XtNbaseHeight, XCURSESBORDERWIDTH * 2,
			XtNiconPixmap, icon_bitmap, NULL);

	/* Create a BOX widget in which to draw */

	if (XCURSESSCROLLBARWIDTH != 0 && sb_started)
	{
		scrollBox = XtVaCreateManagedWidget(XCursesProgramName, 
			scrollBoxWidgetClass, topLevel, XtNwidth, 
			XCursesWindowWidth + XCURSESSCROLLBARWIDTH, 
			XtNheight, XCursesWindowHeight + 
			XCURSESSCROLLBARWIDTH, XtNwidthInc, 
			XCursesFontWidth, XtNheightInc, 
			XCursesFontHeight, NULL);

		drawing = XtVaCreateManagedWidget(XCursesProgramName, 
			boxWidgetClass, scrollBox, XtNwidth, 
			XCursesWindowWidth, XtNheight, 
			XCursesWindowHeight, XtNwidthInc, 
			XCursesFontWidth, XtNheightInc, 
			XCursesFontHeight, NULL);

		scrollVert = XtVaCreateManagedWidget("scrollVert", 
			scrollbarWidgetClass, scrollBox, XtNorientation, 
			XtorientVertical, XtNheight, 
			XCursesWindowHeight, XtNwidth, 
			XCURSESSCROLLBARWIDTH, NULL);

		XtAddCallback(scrollVert, XtNscrollProc,
			Scroll_up_down, drawing);

		XtAddCallback(scrollVert, XtNjumpProc,
			Thumb_up_down, drawing);

		scrollHoriz = XtVaCreateManagedWidget("scrollHoriz", 
			scrollbarWidgetClass, scrollBox, XtNorientation, 
			XtorientHorizontal, XtNwidth, 
			XCursesWindowWidth, XtNheight, 
			XCURSESSCROLLBARWIDTH, NULL);

		XtAddCallback(scrollHoriz, XtNscrollProc, 
			Scroll_left_right, drawing);

		XtAddCallback(scrollHoriz, XtNjumpProc, 
			Thumb_left_right, drawing);
	}
	else
	{
		drawing = XtVaCreateManagedWidget(XCursesProgramName, 
			boxWidgetClass, topLevel, XtNwidth, 
			XCursesWindowWidth, XtNheight, 
			XCursesWindowHeight, XtNwidthInc, 
			XCursesFontWidth, XtNheightInc, 
			XCursesFontHeight, NULL);

		XtVaSetValues(topLevel, XtNwidthInc, XCursesFontWidth, 
			XtNheightInc, XCursesFontHeight, NULL);
	}

	/* Process any default translations */

	XCursesTranslations = XtParseTranslationTable(defaultTranslations);
	XtAugmentTranslations(drawing, XCursesTranslations);
	XtAppAddActions(app_context, XCursesActions, XtNumber(XCursesActions));

	/* Process the supplied colors */

	if (get_colors() == ERR)
	{
		kill(otherpid, SIGKILL);
		return ERR;
	}

	/* Determine text cursor alignment from resources */

	if (strcmp(XCURSESTEXTCURSOR, "vertical") == 0)
		vertical_cursor = True;

	/* Now have LINES and COLS. Set these in the shared SP so the 
	   curses program can find them. */

	LINES = XCursesLINES;
	COLS = XCursesCOLS;

	if ((shmidSP = shmget(shmkeySP, sizeof(SCREEN) + XCURSESSHMMIN, 
	    0700 | IPC_CREAT)) < 0)
	{
		perror("Cannot allocate shared memory for SCREEN");
		kill(otherpid, SIGKILL);
		return ERR;
	}

	SP = (SCREEN*)shmat(shmidSP, 0, 0);
	memset(SP, 0, sizeof(SCREEN));
	SP->XcurscrSize = XCURSCR_SIZE;
	SP->lines = XCursesLINES;
	SP->cols = XCursesCOLS;

	PDC_LOG(("%s:SHM size for curscr %d\n", XCLOGMSG, SP->XcurscrSize));

	if ((shmid_Xcurscr = shmget(shmkey_Xcurscr, SP->XcurscrSize + 
	    XCURSESSHMMIN, 0700 | IPC_CREAT)) < 0)
	{
		perror("Cannot allocate shared memory for curscr");
		kill(otherpid, SIGKILL);
		shmdt((char *)SP);
		shmctl(shmidSP, IPC_RMID, 0);
		return ERR;
	}

	Xcurscr = (unsigned char *)shmat(shmid_Xcurscr, 0, 0);
	memset(Xcurscr, 0, SP->XcurscrSize); 
	atrtab = (unsigned char *)(Xcurscr + XCURSCR_ATRTAB_OFF);

	PDC_LOG(("%s:shmid_Xcurscr %d shmkey_Xcurscr %d LINES %d COLS %d\n",
		XCLOGMSG, shmid_Xcurscr, shmkey_Xcurscr, LINES, COLS));

	/* Add Event handlers to the drawing widget */

	XtAddEventHandler(drawing, ExposureMask, False, XCursesExpose, NULL);
	XtAddEventHandler(drawing, StructureNotifyMask, False, 
		XCursesStructureNotify, NULL);
	XtAddEventHandler(drawing, EnterWindowMask | LeaveWindowMask, 
		False, XCursesEnterLeaveWindow, NULL);

	XtAddEventHandler(topLevel, 0, True, XCursesNonmaskable, NULL);

	/* Add input handler from display_sock (requests from curses 
	   program) */

	XtAppAddInput(app_context, display_sock, 
		(XtPointer)XtInputReadMask, 
		XCursesProcessRequestsFromCurses, NULL);

	/* If there is a cursorBlink resource, start the Timeout event */

	if (XCURSESCURSORBLINKRATE)
		XtAppAddTimeOut(app_context, XCURSESCURSORBLINKRATE, 
			XCursesCursorBlink, NULL);

	/* Leave telling the curses process that it can start to here so 
	   that when the curses process makes a request, the Xcurses 
	   process can service the request.*/

	write_display_socket_int(CURSES_CHILD);

	XtRealizeWidget(topLevel);

	/* Handle trapping of the WM_DELETE_WINDOW property */

	wm_atom[0] = XInternAtom(XtDisplay(topLevel), 
		"WM_DELETE_WINDOW", False);

	XSetWMProtocols(XtDisplay(topLevel), XtWindow(topLevel), 
		wm_atom, 1);

	/* Create the Graphics Context for drawing. This MUST be done 
	   AFTER the associated widget has been realized. */

	XC_LOG(("before get_GC\n"));

	get_GC(XCURSESDISPLAY, XCURSESWIN, &normal_gc, 
		XCURSESNORMALFONTINFO, COLOR_WHITE, COLOR_BLACK, FALSE);

	if (italic_font_valid)
		get_GC(XCURSESDISPLAY, XCURSESWIN, &italic_gc, 
			XCURSESITALICFONTINFO, COLOR_WHITE, COLOR_BLACK, 
			FALSE);
	else
		get_GC(XCURSESDISPLAY, XCURSESWIN, &italic_gc, 
			XCURSESNORMALFONTINFO, COLOR_WHITE, COLOR_BLACK, 
			FALSE);

	get_GC(XCURSESDISPLAY, XCURSESWIN, &block_cursor_gc,
		XCURSESNORMALFONTINFO, COLOR_BLACK, COLOR_CURSOR, FALSE);

	get_GC(XCURSESDISPLAY, XCURSESWIN, &rect_cursor_gc, 
		XCURSESNORMALFONTINFO, COLOR_CURSOR, COLOR_BLACK, FALSE);

	get_GC(XCURSESDISPLAY, XCURSESWIN, &border_gc, 
		XCURSESNORMALFONTINFO, COLOR_BORDER, COLOR_BLACK, FALSE);

	XSetLineAttributes(XCURSESDISPLAY, rect_cursor_gc, 2, LineSolid, 
		CapButt, JoinMiter);

	XSetLineAttributes(XCURSESDISPLAY, border_gc, 
		XCURSESBORDERWIDTH, LineSolid, CapButt, JoinMiter);

	/* Set the cursor for the application */

	XDefineCursor(XCURSESDISPLAY, XCURSESWIN, XCURSESPOINTER);
	rmfrom.size = sizeof(Pixel);
	rmto.size = sizeof(XColor);

#if XtSpecificationRelease > 4
	rmto.addr = (XPointer)&pointerforecolor;
	rmfrom.addr = (XPointer)&(XCURSESPOINTERFORECOLOR);
	XtConvertAndStore(drawing, XtRPixel, &rmfrom, XtRColor, &rmto);
#else
	rmto.addr = (char *)&pointerforecolor;
	rmfrom.addr = (char *)&(XCURSESPOINTERFORECOLOR);
	XtConvert(drawing, XtRPixel, &rmfrom, XtRColor, &rmto);
#endif

	rmfrom.size = sizeof(Pixel);
	rmto.size = sizeof(XColor);

#if XtSpecificationRelease > 4
	rmfrom.addr = (XPointer)&(XCURSESPOINTERBACKCOLOR);
	rmto.addr = (XPointer)&pointerbackcolor;
	XtConvertAndStore(drawing, XtRPixel, &rmfrom, XtRColor, &rmto);
#else
	rmfrom.addr = (char *)&(XCURSESPOINTERBACKCOLOR);
	rmto.addr = (char *)&pointerbackcolor;
	XtConvert(drawing, XtRPixel, &rmfrom, XtRColor, &rmto);
#endif

	XRecolorCursor(XCURSESDISPLAY, XCURSESPOINTER,
		&pointerforecolor, &pointerbackcolor);

	/* Convert the supplied compose key to a Keysym */

	compose_key = XStringToKeysym(XCURSESCOMPOSEKEY);

	if (compose_key && IsModifierKey(compose_key))
	{
		int i, j;
		KeyCode *kcp;
		XModifierKeymap *map;
		KeyCode compose_keycode = 
			XKeysymToKeycode(XCURSESDISPLAY, compose_key);

		map = XGetModifierMapping(XCURSESDISPLAY);
		kcp = map->modifiermap;

		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < map->max_keypermod; j++, kcp++)
			{
				if (!*kcp)
					continue;

				if (compose_keycode == *kcp)
				{
					compose_mask = state_mask[i];
					break;
				}
			}

			if (compose_mask)
				break;
		}

		XFreeModifiermap(map);
	}

#ifdef FOREIGN
	sleep(20);

	if ((Xim = XOpenIM(XCURSESDISPLAY,NULL,NULL,NULL)) == NULL)
	{
		perror("Cannot open Input Method");
		kill(otherpid, SIGKILL);
		shmdt((char *)SP);
		shmdt((char *)Xcurscr);
		shmctl(shmidSP, IPC_RMID, 0);
		shmctl(shmid_Xcurscr, IPC_RMID, 0);
		return ERR;
	}

	XGetIMValues(Xim,XNQueryInputStyle, &im_supported_styles, NULL);
	my_style = XIMPreeditNone | XIMStatusNone;

	if ((Xic = XCreateIC(Xim, XNInputStyle, my_style, NULL)) == NULL)
	{
		perror("ERROR: Cannot create input context");
		kill(otherpid, SIGKILL);
		shmdt((char *)SP);
		shmdt((char *)Xcurscr);
		shmctl(shmidSP, IPC_RMID, 0);
		shmctl(shmid_Xcurscr, IPC_RMID, 0);
		return ERR;
	}

	XFree(im_supported_styles);
	XGetICValues(Xic, XNFilterEvents, &im_event_mask, NULL);
	XtAddEventHandler(drawing, im_event_mask, False, NULL, NULL);
	XSetICFocus(Xic);
#endif

	/* Wait for events */

	XtAppMainLoop(app_context);
	return OK;			/* won't get here */
}

RETSIGTYPE XCursesSignalHandler(int signo)
{
	int flag = CURSES_EXIT;

	PDC_LOG(("%s:XCursesSignalHandler() - called: %d\n", XCLOGMSG, signo));

	/* Patch by: Georg Fuchs */

	XCursesSetSignal(signo, XCursesSignalHandler);

#ifdef SIGTSTP
	if (signo == SIGTSTP)
	{
		pause();
		return;
	}
#endif
#ifdef SIGCONT
	if (signo == SIGCONT)
		return;
#endif
#ifdef SIGCLD
	if (signo == SIGCLD)
		return;
#endif
#ifdef SIGTTIN
	if (signo == SIGTTIN)
		return;
#endif
#ifdef SIGWINCH
	if (signo == SIGWINCH)
		return;
#endif

	/* End of patch by: Georg Fuchs */

	XCursesSetSignal(signo, SIG_IGN);

	/* Send a CURSES_EXIT to myself */

	if (write_socket(exit_sock, (char *)&flag, sizeof(int)) < 0)
		XCursesExitXCursesProcess(7, signo,
			"exiting from XCursesSignalHandler");
}

void XCursesRequestorCallbackForGetSelection(Widget w, XtPointer data,
					     Atom *selection, Atom *type,
					     XtPointer value,
					     unsigned long *length, int *format)
{
	PDC_LOG(("%s:XCursesRequestorCallbackForSelection() - called\n",
		XCLOGMSG));

	if ((value == NULL) && (*length == 0))
	{
	    if (write_display_socket_int(PDC_CLIP_EMPTY) >= 0)
		return;
	}
	else
	{
	    /* Here all is OK, send PDC_CLIP_SUCCESS, then length, then 
	       contents */

	    if (write_display_socket_int(PDC_CLIP_SUCCESS) >= 0)
		if (write_display_socket_int((int)(*length)) >= 0)
		    if (write_socket(display_sock, (char *)value, *length) >= 0)
			return;
	}

	XCursesExitXCursesProcess(4, SIGKILL,
		"exiting from XCursesRequestorCallbackForGetSelection");
}

void XCursesStructureNotify(Widget w, XtPointer client_data, XEvent *event,
			    Boolean *continue_to_dispatch)
{
	PDC_LOG(("%s:XCursesStructureNotify called\n", XCLOGMSG));

	switch(event->type)
	{
	case ConfigureNotify:
		XC_LOG(("ConfigureNotify received\n"));

		/* Window has been resized, change width and height to 
		   send to place_text and place_graphics in next Expose. 
		   Also will need to kill (SIGWINCH) curses process if 
		   screen size changes. */

		resizeXCursesWindowWidth = (event->xconfigure.width);
		resizeXCursesWindowHeight = (event->xconfigure.height);

		after_first_curses_request = False;

#ifdef SIGWINCH
		SP->resized = 1;

		kill(otherpid, SIGWINCH);
#endif
		XCursesSendKeyToCurses((unsigned long)KEY_RESIZE, NULL);
		break;

	case MapNotify:
		XC_LOG(("MapNotify received\n"));

		ReceivedMapNotify = 1;

		/* Draw the window border */

		if (XCURSESBORDERWIDTH)
			XDrawRectangle(XCURSESDISPLAY, XCURSESWIN, 
				border_gc, XCURSESBORDERWIDTH / 2, 
				XCURSESBORDERWIDTH / 2, 
				XCursesWindowWidth - XCURSESBORDERWIDTH, 
				XCursesWindowHeight - XCURSESBORDERWIDTH);
		break;

	default:
		PDC_LOG(("%s:XCursesStructureNotify - unknown event %d\n",
			XCLOGMSG, event->type));
	}
}