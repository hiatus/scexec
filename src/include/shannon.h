#ifndef SHANNON_H
#define SHANNON_H

#include <stddef.h>

void shannon_init(void);
void shannon_update(void *, size_t len);

double shannon_entropy(void);
#endif
