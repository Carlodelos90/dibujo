#include <locale.h>
#include <ncurses.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>


/* Box-drawing characters in UTF-8 */
static const char *BorderVertical      = "│";
static const char *BorderHorizontal    = "─";
static const char *BorderTopLeft       = "┌";
static const char *BorderTopRight      = "┐";
static const char *BorderBottomLeft    = "└";
static const char *BorderBottomRight   = "┘";

int main(void) {


    refresh();
    getch();
    endwin();

    return 0;
}
