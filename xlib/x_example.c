/* File: window.c
                  * On Linux, use "gcc -o window window.c -L/usr/X11R6/lib -lX11
		                    * to compile this code.
				                      */
#include <X11/X.h>
#include <X11/Xlib.h>

int
main(void)
{
    Display *d;
    Window w, r;
    int s;
    d = XOpenDisplay(":0.0");
    s = DefaultScreen(d);
    r = RootWindow(d, s);
    w = XCreateSimpleWindow(d, r, 0, 0, 100, 100, 1, 0, 0);
    XMapWindow(d, w);
    XFlush(d);
    getchar();
    XCloseDisplay(d);
    return (0);
}
