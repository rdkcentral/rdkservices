#pragma once
#include <stdio.h>
#include <stdlib.h>

#  ifdef __cplusplus
extern "C" {
#  endif


FILE *v_secure_popen(const char *direction, const char *command, ...);

int v_secure_pclose(FILE *);


#  ifdef __cplusplus
}
#  endif

