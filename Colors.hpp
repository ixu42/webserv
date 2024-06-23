#pragma once

// Text color macros
#define TEXT_BLACK   "\033[30m"
#define TEXT_RED     "\033[31m"
#define TEXT_GREEN   "\033[32m"
#define TEXT_YELLOW  "\033[33m"
#define TEXT_BLUE    "\033[34m"
#define TEXT_MAGENTA "\033[35m"
#define TEXT_CYAN    "\033[36m"
#define TEXT_WHITE   "\033[37m"

// Background color macros
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

// Text style macros
#define TEXT_BOLD      "\033[1m"
#define TEXT_UNDERLINE "\033[4m"
#define TEXT_BOLD_UNDERLINE "\033[1;4m"

// Reset macros
#define RESET "\033[0m"
#define RESET_UNDERLINE "\033[24m"
#define RESET_BOLD "\033[22m"
#define RESET_BOLD_UNDERLINE "\033[22;24m"
#define RESET_BG "\033[49m"