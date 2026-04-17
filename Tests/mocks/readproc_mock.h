#pragma once
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROC_FILLMEM 0
#define PROC_FILLSTAT 0
#define PROC_FILLSTATUS 0
#define PROC_FILLCOM 0

typedef struct PROCTAB { int dummy; } PROCTAB;
typedef struct proc_t { int dummy; } proc_t;

static inline PROCTAB* openproc(int) { return 0; }
static inline proc_t* readproc(PROCTAB*, proc_t*) { return 0; }
static inline void closeproc(PROCTAB*) {}

#ifdef __cplusplus
}
#endif
