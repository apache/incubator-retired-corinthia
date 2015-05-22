#ifndef __COLOR_H
#define __COLOR_H
//
#ifndef COLOR_ON
#define COLOR_ON 0
#define RED     ""
#define GREEN   ""
#define YELLOW  ""
#define BLUE    ""
#define MAGENTA ""
#define CYAN    ""
#define RESET   ""
#define COLOR_SIZEOF 0
//  
#else // let's light the X-mas tree!
//
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define COLOR_SIZEOF 8  
#endif // COLOR_ON
//
#endif // __COLOR_H

char *red(const char *str);
char *blue(const char *str);
char *green(const char *str);
char *yellow(const char *str);
char *cyan(const char *str);
char *magenta(const char *str);
