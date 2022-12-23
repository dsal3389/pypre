#ifndef _ANSI_H_
#define _ANSI_H_ 1

#define ANSI_ESC "\x1b"
#define _ANSI_FORMAT(color_code) ANSI_ESC "[" color_code "m"

#define BOLD _ANSI_FORMAT("1")

#define BLACK   _ANSI_FORMAT("30")
#define RED     _ANSI_FORMAT("31")
#define GREEN   _ANSI_FORMAT("32")
#define YELLOW  _ANSI_FORMAT("33")
#define RESET   _ANSI_FORMAT("39")

#define BRIGHT_BLACK  _ANSI_FORMAT("90")
#define BRIGHT_RED    _ANSI_FORMAT("91")
#define BRIGHT_GREEN  _ANSI_FORMAT("92")
#define BRIGHT_YELLOW _ANSI_FORMAT("93")

#define BG_BLACK  _ANSI_FORMAT("40")
#define BG_RED    _ANSI_FORMAT("41")
#define BG_GREEN  _ANSI_FORMAT("42")
#define BG_YELLOW _ANSI_FORMAT("43")
#define BG_RESET  _ANSI_FORMAT("49")

#define ANSI_COLOR_TEXT(color, text) \
    color text RESET BG_RESET

#define ANSI_BOLD_TEXT(text) \
    BOLD text RESET _ANSI_FORMAT("0")

#endif 
