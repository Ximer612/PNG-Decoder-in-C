#include <stdio.h>

#ifdef DEBUG_PRINT
#define SET_PRINT_CONSOLE_COLOR(color) printf(color);
#elif
#define SET_PRINT_CONSOLE_COLOR(color) 
#endif

#define SET_WHITE_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;37m")
#define SET_CYAN_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;36m")
#define SET_PURPLE_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;35m")
#define SET_BLUE_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;34m")
#define SET_YELLOW_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;33m")
#define SET_GREEN_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;32m")
#define SET_RED_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;31m")
#define SET_BLACK_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;30m")
#define SET_DEFAULT_PRINT() SET_PRINT_CONSOLE_COLOR("\033[0;00m\n")

#define WHITE_PRINT(string,...) SET_WHITE_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define CYAN_PRINT(string,...) SET_CYAN_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define PURPLE_PRINT(string,...) SET_PURPLE_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define BLUE_PRINT(string,...) SET_BLUE_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define YELLOW_PRINT(string,...) SET_YELLOW_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define GREEN_PRINT(string,...) SET_GREEN_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define RED_PRINT(string,...) SET_RED_PRINT() printf(string, ##__VA_ARGS__);SET_DEFAULT_PRINT()
#define BLACK_PRINT(string,...) SET_BLACK_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()
#define DEFAULT_PRINT(string,...) SET_DEFAULT_PRINT() printf(string, ##__VA_ARGS__); SET_DEFAULT_PRINT()