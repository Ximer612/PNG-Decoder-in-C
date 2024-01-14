#include <stdio.h>

#ifdef DEBUG_PRINT
    #define SET_BLUE_PRINT() printf("\033[0;34m")
    #define SET_GREEN_PRINT() printf("\033[0;32m")
    #define SET_RED_PRINT() printf("\033[0;31m")
    #define SET_DEFAULT_PRINT() printf("\033[0;00m")
    #define RED_PRINT(string,...)      printf("\033[0;31m"string"\033[0;00m\n", ##__VA_ARGS__)
    #define GREEN_PRINT(string,...)    printf("\033[0;32m"string"\033[0;00m\n", ##__VA_ARGS__)
    #define YELLOW_PRINT(string,...)   printf("\033[0;33m"string"\033[0;00m\n", ##__VA_ARGS__)
    #define BLUE_PRINT(string,...)     printf("\033[0;34m"string"\033[0;00m\n", ##__VA_ARGS__)
    #define MAGENTA_PRINT(string,...)  printf("\033[0;35m"string"\033[0;00m\n", ##__VA_ARGS__)
    #define CYAN_PRINT(string,...)     printf("\033[0;36m"string"\033[0;00m\n", ##__VA_ARGS__)
    #define WHITE_PRINT(string,...)    printf("\033[0;37m"string"\033[0;00m\n", ##__VA_ARGS__)
#else
    #define SET_BLUE_PRINT()   
    #define SET_GREEN_PRINT()  
    #define SET_RED_PRINT()    
    #define SET_DEFAULT_PRINT()
    #define RED_PRINT(string,...)    
    #define GREEN_PRINT(string,...)  
    #define YELLOW_PRINT(string,...) 
    #define BLUE_PRINT(string,...)   
    #define MAGENTA_PRINT(string,...)
    #define CYAN_PRINT(string,...)   
    #define WHITE_PRINT(string,...)  
#endif