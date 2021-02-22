#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

enum{
	Hz = 30,
};
char *progname = "3d01";

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

/* absolute map: global 2d topdown view */
static void
render(void)
{
	draw(fb, fb->r, col[Cbg], nil, ZP);
	line(fb, wall.min, wall.max, 0, 0, 1, col[Cwall], ZP);
	ellipse(fb, Pt(player.x, player.y), 2, 2, 0, col[Cplayer], ZP);
	line(fb, Pt(player.x, player.y),
		Pt(player.x + cos(player.θ) * 15, player.y + sin(player.θ) * 15),
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
