#include <stdio.h>
#include <stdarg.h>
#include <curses.h>

#define max_string 1024

static struct {
	WINDOW *mainwin;
	WINDOW *text;
	WINDOW *cmd;
} mw;

static void initialize (void);
static void printwin (const char *fmt, ...);

void main ()
{
	char buf[128];

	initialize ();

	while (1) {
	wgetnstr (mw.cmd, buf, sizeof (buf));
	printwin ("%s\n", buf);
	}
}

static void printwin (const char *fmt, ...)
{
	static int cur_line;
	
	va_list ap;
	char buf[max_string];

	memset (buf, 0, sizeof (buf));

	va_start (ap, fmt);
	vsnprintf (buf, max_string, fmt, ap);
	va_end (ap);

	wprintw (mw.text, buf);
	wrefresh (mw.text);

	wclear (mw.cmd);
	wrefresh (mw.cmd);
	
	cur_line++;
	
	if (cur_line==22) {
		scroll(mw.text);
		cur_line=0;
		}
}

static void initialize (void)
{
	initscr ();
	cbreak ();

	if (LINES < 5 || COLS < 10) {
		fprintf (stderr, "no way\n");
		exit (1);
	}

	mw.mainwin = newwin (LINES - 1, 0, 0, 0);
	mw.text = newwin (LINES - 3, COLS - 2, 1, 1);
	mw.cmd = newwin (1, 0, LINES - 1, 0);
       
        scrollok(mw.text, TRUE);
       
	
	box (mw.mainwin, ACS_VLINE, ACS_HLINE);

	wrefresh (stdscr);
	wrefresh (mw.mainwin);
	wrefresh (mw.text);
	wrefresh (mw.cmd);
	doupdate ();
}

