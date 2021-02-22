/* global map + transformed map
 * like in opengl, the player's position on the transformed map
 * is fixed, and the world around it moves.
 */

#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

enum{
	Hz = 30,
};
char *progname = "3d02";

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
	Rectangle wl, rwl;

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
	/* ... but don't rotate */
	rwl = wl;
	/* offset to second view */
	pl.x = 200;
	pl.y = 50;
	rwl.min.x = pl.x - rwl.min.x;
	rwl.min.y = pl.y - rwl.min.y;
	rwl.max.x = pl.x - rwl.max.x;
	rwl.max.y = pl.y - rwl.max.y;
	line(fb, rwl.min, rwl.max, 0, 0, 1, col[Cwall], ZP);
	ellipse(fb, Pt(pl.x, pl.y), 2, 2, 0, col[Cplayer], ZP);
	line(fb, Pt(pl.x, pl.y),
		Pt(pl.x + cos(-PI/2) * 15, pl.y + sin(-PI/2) * 15),	/* neg since y is flipped */
		0, 0, 0, col[Carrow], ZP);

	/* same, but do rotate */
	rwl = wl;
	rwl.min.x = wl.min.x * sin(player.θ) - wl.min.y * cos(player.θ);
	rwl.min.y = wl.min.x * cos(player.θ) + wl.min.y * sin(player.θ);
	rwl.max.x = wl.max.x * sin(player.θ) - wl.max.y * cos(player.θ);
	rwl.max.y = wl.max.x * cos(player.θ) + wl.max.y * sin(player.θ);
	/* offset to third view */
	pl.x = 400;
	pl.y = 50;
	rwl.min.x = pl.x - rwl.min.x;
	rwl.min.y = pl.y - rwl.min.y;
	rwl.max.x = pl.x - rwl.max.x;
	rwl.max.y = pl.y - rwl.max.y;
	line(fb, rwl.min, rwl.max, 0, 0, 1, col[Cwall], ZP);
	ellipse(fb, Pt(pl.x, pl.y), 2, 2, 0, col[Cplayer], ZP);
	line(fb, Pt(pl.x, pl.y),
		Pt(pl.x + cos(-PI/2) * 15, pl.y + sin(-PI/2) * 15),
		0, 0, 0, col[Carrow], ZP);
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
