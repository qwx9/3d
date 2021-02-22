#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <keyboard.h>
#include <mouse.h>
#include "dat.h"
#include "fns.h"

void (*stepsimfn)(void);

typedef struct Kev Kev;
typedef struct Mev Mev;
struct Kev{
	int down;
	Rune r;
};
struct Mev{
	Point;
	int dx;
	int dy;
	int b;
};
static Channel *reszc, *kc, *mc, *tmc;

void *
emalloc(ulong n)
{
	void *p;

	if((p = mallocz(n, 1)) == nil)
		sysfatal("emalloc: %r");
	setmalloctag(p, getcallerpc(&n));
	return p;
}

static void
mproc(void *)
{
	int n, fd, nerr;
	char buf[1+5*12];
	Mev m, om;

	if((fd = open("/dev/mouse", OREAD)) < 0)
		sysfatal("mproc: %r");
	nerr = 0;
	memset(&om, 0, sizeof om);
	for(;;){
		if((n = read(fd, buf, sizeof buf)) != 1+4*12){
			if(n < 0 || ++nerr > 10)
				break;
			fprint(2, "mproc: bad count %d not 49: %r\n", n);
			continue;
		}
		nerr = 0;
		switch(buf[0]){
		case 'r': send(reszc, nil); /* wet floor */
		case 'm':
			m.x = strtol(buf+1+12*0, nil, 10);
			m.y = strtol(buf+1+12*1, nil, 10);
			m.b = strtol(buf+1+12*2, nil, 10);
			m.dx = m.x - om.x;
			m.dy = m.y - om.y;
			if((m.b & 1) == 1 && (om.b & 1) == 0
			|| (m.b & 4) == 4 && (om.b & 4) == 0
			|| m.b & 2)
				send(mc, &m);
			om = m;
			break;
		}
	}
}

static void
kproc(void *)
{
	int n, fd;
	char buf[256], down[128], *s, *p;
	Rune r;
	Kev ke;

	if((fd = open("/dev/kbd", OREAD)) < 0)
		sysfatal("kproc: %r");
	memset(buf, 0, sizeof buf);
	for(;;){
		if(buf[0] != 0){
			n = strlen(buf)+1;
			memmove(buf, buf+n, sizeof(buf)-n);
		}
		if(buf[0] == 0){
			n = read(fd, buf, sizeof(buf)-1);
			if(n <= 0)
				break;
			buf[n-1] = 0;
			buf[n] = 0;
		}
		switch(buf[0]){
		default: continue;
		case 'k': s = buf+1; p = down+1; ke.down = 1; break;
		case 'K': s = down+1; p = buf+1; ke.down = 0; break;
		}
		while(*s != 0){
			s += chartorune(&r, s);
			if(utfrune(p, r) == nil){
				ke.r = r;
				if(send(kc, &ke) < 0)
					break;
			}
		}
		strcpy(down, buf);
	}
}

static void
timeproc(void *)
{
	for(;;){
		sleep(tdiv);
		nbsendul(tmc, 0);
	}
}


void
sim(void)
{
	Kev ke;
	Mev me;
	Key *k;

	enum{
		Aresize,
		Amouse,
		Akbd,
		Atic,
	};
	Alt a[] = {
		{reszc, nil, CHANRCV},
		{mc, &me, CHANRCV},
		{kc, &ke, CHANRCV},
		{tmc, nil, CHANRCV},
		{nil, nil, CHANEND}
	};
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			resetdraw();
			break;
		case Amouse:
			break;
		case Akbd:
			if(ke.r == Kdel)
				threadexitsall(nil);
			for(k=keys; k<keys+nkeys; k++)
				if(ke.r == k->r){
					k->down = ke.down;
					break;
				}
			break;
		case Atic:
			stepsimfn();
			updatedraw();
			break;
		}
	}
}

void
sysinit(void (*initrender)(void), void (*initsim)(void), void (*render)(void), void (*stepsim)(void))
{
	srand(time(nil));
	if(initdraw(nil, nil, progname) < 0)
		sysfatal("initdraw: %r");
	if((reszc = chancreate(sizeof(int), 2)) == nil
	|| (kc = chancreate(sizeof(Kev), 20)) == nil
	|| (mc = chancreate(sizeof(Mev), 20)) == nil)
		sysfatal("chancreate: %r");
	if(proccreate(kproc, nil, 8192) < 0
	|| proccreate(mproc, nil, 8192) < 0)
		sysfatal("proccreate: %r");
	if((tmc = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %r");
	if(proccreate(timeproc, nil, 8192) < 0)
		sysfatal("init: %r");
	initrender();
	initsim();
	renderfn = render;
	stepsimfn = stepsim;
	resetdraw();
}
