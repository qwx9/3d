void*	emalloc(ulong);
void	sim(void);
void	sysinit(void (*)(void), void (*)(void), void (*)(void), void (*)(void));
Image*	eallocimage(Rectangle, ulong, int, ulong);
void	updatedraw(void);
void	redraw(void);
void	resetdraw(void);
