/* global map + transformed map + perspective */

#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

enum{
	Hz = 30,
};
char *progname = "3d03";

int tdiv = Te3 / Hz;

enum{
	Cbg,
	Cplayer,
	Carrow,
	Cwall,
	Cend,
};
static Image *col[Cend];

typedef struct Player Player;
struct Player{
	Foint;
	double θ;
};
static Player player;
static Rectangle wall;

static void
forward(void)
{
	player.x += cos(player.θ);
	player.y += sin(player.θ);
}

static void
backward(void)
{
	player.x -= cos(player.θ);
	player.y -= sin(player.θ);
}

static void
turnleft(void)
{
	player.θ -= 0.1;
}

static void
turnright(void)
{
	player.θ += 0.1;
}

static void
transleft(void)
{
	player.x += sin(player.θ);
	player.y -= cos(player.θ);
}

static void
transright(void)
{
	player.x -= sin(player.θ);
	player.y += cos(player.θ);
}

Key keys[] = {
	{'w', forward},
	{'s', backward},
	{'a', turnleft},
	{'d', turnright},
	{'q', transleft},
	{'e', transright},
};
int nkeys = nelem(keys);

static void
stepsim(void)
{
	Key *k;

	for(k=keys; k<keys+nkeys; k++)
		if(k->down)
			k->fn();
}

static void
render(void)
{
	Player pl;
	Rectangle r, wl, rwl, top, bottom;

	draw(fb, fb->r, col[Cbg], nil, ZP);
	wl = wall;
	pl = player;

	/* absolute map */
	line(fb, wl.min, wl.max, 0, 0, 1, col[Cwall], ZP);
	ellipse(fb, Pt(pl.x, pl.y), 2, 2, 0, col[Cplayer], ZP);
	line(fb, Pt(pl.x, pl.y),
		Pt(pl.x + cos(player.θ) * 15, pl.y + sin(player.θ) * 15),
		0, 0, 0, col[Carrow], ZP);

	/* transform vertices relative to player coordinates */
	wl.min.x -= pl.x;
	wl.min.y -= pl.y;
	wl.max.x -= pl.x;
	wl.max.y -= pl.y;
	rwl.min.x = wl.min.x * sin(player.θ) - wl.min.y * cos(player.θ);
	rwl.min.y = wl.min.x * cos(player.θ) + wl.min.y * sin(player.θ);	/* now left depth (since camera looks towards -∞ along y axis */
	rwl.max.x = wl.max.x * sin(player.θ) - wl.max.y * cos(player.θ);
	rwl.max.y = wl.max.x * cos(player.θ) + wl.max.y * sin(player.θ);	/* now right depth */
	/* offset view */
	pl.x = 200;
	pl.y = 50;
	r.min.x = pl.x - rwl.min.x;
	r.min.y = pl.y - rwl.min.y;
	r.max.x = pl.x - rwl.max.x;
	r.max.y = pl.y - rwl.max.y;
	line(fb, r.min, r.max, 0, 0, 1, col[Cwall], ZP);
	ellipse(fb, Pt(pl.x, pl.y), 2, 2, 0, col[Cplayer], ZP);
	line(fb, Pt(pl.x, pl.y),
		Pt(pl.x + cos(-PI/2) * 15, pl.y + sin(-PI/2) * 15),
		0, 0, 0, col[Carrow], ZP);

	/* perspective-transformed map:
	 * just take the transformed wall coordinates and divide by depth */
	if(rwl.min.y == 0 || rwl.max.y == 0)
		return;
	/* length of the wall = 50 (euclidean distance), we divide for smaller viewsize */
	top.min.x = 50 + (double)rwl.min.x * 50/2 / rwl.min.y;
	top.max.x = 50 + (double)rwl.max.x * 50/2 / rwl.max.y;
	top.min.y = 50 + -50 / (double)rwl.min.y;
	top.max.y = 50 + -50 / (double)rwl.max.y;
	bottom.min.x = top.min.x;
	bottom.max.x = top.max.x;
	bottom.min.y = 50 + 50 / (double)rwl.min.y;
	bottom.max.y = 50 + 50 / (double)rwl.max.y;
	/* offset view */
	pl.x = 300;
	top = rectaddpt(top, Pt(pl.x, pl.y));
	bottom = rectaddpt(bottom, Pt(pl.x, pl.y));
	line(fb, top.min, top.max, 0, 0, 1, col[Cwall], ZP);
	line(fb, bottom.min, bottom.max, 0, 0, 1, col[Cwall], ZP);
}

static void
initrender(void)
{
	col[Cbg] = display->black;
	col[Cplayer] = display->white;
	col[Carrow] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x777777ff);
	col[Cwall] = eallocimage(Rect(0,0,1,1), screen->chan, 1, DYellow);
}

static void
initsim(void)
{
	wall = Rect(70, 20, 70, 70);
	player = (Player){(Foint){50, 50}, 0};
}

void
threadmain(int, char**)
{
	sysinit(initrender, initsim, render, stepsim);
	sim();
}
