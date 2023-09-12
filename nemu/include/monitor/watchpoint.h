#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	char *expr;
	uint32_t new_val;
	uint32_t old_val;

} WP;

int wp_set(char *e);
bool wp_remove(int NO);
void wp_display();
WP* wp_scanner();

#endif
