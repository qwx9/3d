#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

Rectangle fbr;
Image *fb;

void (*renderfn)(void);

Image *
eallocimage(Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

void
updatedraw(void)
{
	renderfn();
	draw(screen, screen->r, fb, nil, ZP);
	flushimage(display, 1);
}

void
redraw(void)
{
	updatedraw();
}

void
resetdraw(void)
{
	freeimage(fb);
	fbr = rectsubpt(screen->r, screen->r.min);
	fb = eallocimage(fbr, screen->chan, 0, DNofill);
	redraw();
}
