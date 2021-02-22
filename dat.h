typedef struct Key Key;
typedef struct Foint Foint;
typedef struct Fectangle Fectangle;

enum{
	Te9 = 1000000000,
	Te6 = 1000000,
	Te3 = 1000,
};
extern int tdiv;

struct Key{
	Rune r;
	void (*fn)(void);
	int down;
};
extern Key keys[];
extern int nkeys;

extern Image *fb;
extern Rectangle fbr;

struct Foint{
	double x;
	double y;
};
struct Fectangle{
	Foint min;
	Foint max;
};

extern char *progname;
extern void (*renderfn)(void);
extern void (*stepsimfn)(void);
