// qsort.h

void qsort (
    void *base,
    unsigned num,
    unsigned width,
    int (*comp)(const void *, const void *),
	 void (*swap)(const void *, const void *)
    );
