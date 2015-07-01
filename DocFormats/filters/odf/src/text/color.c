#include "color.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *show_color(char *color, const char* str)
{
    char *r;
    if (COLOR_ON) {
        size_t len = strlen (str) + 2 * COLOR_SIZEOF + 1;
        r = malloc(len);
        snprintf (r, len, "%s%s%s", color, str, RESET);
    }
    else {
        size_t len = strlen (str) + 1;
        r = malloc(len);
        snprintf (r, len, "%s",  str);
    }

    return r;
}

char *red(const char *str) { return show_color(RED, str); }
char *blue(const char *str) { return show_color(BLUE, str); }
char *green(const char *str) { return show_color(GREEN, str); }
char *yellow(const char *str) { return show_color(YELLOW, str); }
char *cyan(const char *str) { return show_color(CYAN, str); }
char *magenta(const char *str) { return show_color(MAGENTA, str); }



