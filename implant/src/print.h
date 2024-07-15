
#ifdef NDEBUG
#define PRINTF(...)
#else
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#endif
