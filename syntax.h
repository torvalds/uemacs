/*
 * syntax.h - Syntax highlighting definitions
 */

#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <stdint.h>
#include "utf8.h"

/* Token types for syntax highlighting */
enum syn_token {
	SYN_NORMAL,
	SYN_KEYWORD,
	SYN_TYPE,
	SYN_COMMENT,
	SYN_STRING,
	SYN_NUMBER,
	SYN_PREPROC,
	SYN_MAX,
};

/* Highlighted region - fixed 8 bytes on all platforms */
struct syn_region {
	uint16_t start;
	uint16_t end;
	uint8_t type;
	uint8_t pad[3];
};

#define MAX_SYN_REGIONS 64

int syn_analyze(const unicode_t *text, int len,
		struct syn_region *reg, int maxreg);
int syn_getcolor(enum syn_token type);

#endif
