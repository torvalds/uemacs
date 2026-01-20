/*
 * syntax.c - Simple C syntax highlighting
 *
 * Line-based lexer for C syntax elements. No allocations,
 * single pass through text, O(n) complexity.
 */

#include <string.h>
#include "estruct.h"
#include "syntax.h"

/* C keywords - sorted for binary search */
static const char * const keywords[] = {
	"break",
	"case",
	"continue",
	"default",
	"do",
	"else",
	"extern",
	"for",
	"goto",
	"if",
	"inline",
	"register",
	"return",
	"sizeof",
	"static",
	"switch",
	"typedef",
	"volatile",
	"while",
};

/* C types - sorted for binary search */
static const char * const types[] = {
	"auto",
	"char",
	"const",
	"double",
	"enum",
	"float",
	"int",
	"long",
	"short",
	"signed",
	"struct",
	"union",
	"unsigned",
	"void",
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* ANSI color indices for each token type */
static const unsigned char colors[SYN_MAX] = {
	7,	/* SYN_NORMAL  - white */
	3,	/* SYN_KEYWORD - yellow */
	2,	/* SYN_TYPE    - green */
	6,	/* SYN_COMMENT - cyan */
	1,	/* SYN_STRING  - red */
	5,	/* SYN_NUMBER  - magenta */
	4,	/* SYN_PREPROC - blue */
};

static inline int is_alpha(unsigned int c)
{
	return (c - 'A' <= 'Z' - 'A') || (c - 'a' <= 'z' - 'a');
}

static inline int is_digit(unsigned int c)
{
	return c - '0' <= 9;
}

static inline int is_xdigit(unsigned int c)
{
	return is_digit(c) || (c - 'a' <= 5) || (c - 'A' <= 5);
}

static inline int is_ident(unsigned int c)
{
	return is_alpha(c) || is_digit(c) || c == '_';
}

static inline int is_ident_start(unsigned int c)
{
	return is_alpha(c) || c == '_';
}

/*
 * Binary search in sorted string array.
 * Returns 1 if found, 0 otherwise.
 */
static int bsearch_str(const char *key, const char * const *arr, int n)
{
	int lo = 0;
	int hi = n - 1;

	while (lo <= hi) {
		int mid = lo + (hi - lo) / 2;
		int cmp;

		/* Quick first-char check before full strcmp */
		cmp = (unsigned char)key[0] - (unsigned char)arr[mid][0];
		if (cmp == 0)
			cmp = strcmp(key, arr[mid]);

		if (cmp == 0)
			return 1;
		if (cmp < 0)
			hi = mid - 1;
		else
			lo = mid + 1;
	}
	return 0;
}

/*
 * Analyze a line and identify syntax regions.
 *
 * Returns number of regions found. Regions are sorted by position
 * and non-overlapping, suitable for sequential iteration.
 */
int syn_analyze(const unicode_t *text, int len,
		struct syn_region *reg, int maxreg)
{
	int only_whitespace_before = 1;
	int n = 0;
	int i = 0;

	while (i < len && n < maxreg) {
		unicode_t c = text[i];
		unicode_t next = (i + 1 < len) ? text[i + 1] : 0;
		int start;

		/* Skip whitespace */
		if (c == ' ' || c == '\t') {
			i++;
			continue;
		}

		/* Preprocessor: # at start of line (after optional whitespace) */
		if (c == '#' && only_whitespace_before) {
			reg[n].start = i;
			reg[n].end = len;
			reg[n].type = SYN_PREPROC;
			n++;
			break;
		}

		only_whitespace_before = 0;

		/* C++ comment: // to end of line */
		if (c == '/' && next == '/') {
			reg[n].start = i;
			reg[n].end = len;
			reg[n].type = SYN_COMMENT;
			n++;
			break;
		}

		/* C comment: slash-star to star-slash, single line only */
		if (c == '/' && next == '*') {
			start = i;
			i += 2;
			while (i < len) {
				if (text[i] == '*' && i + 1 < len &&
				    text[i + 1] == '/') {
					i += 2;
					break;
				}
				i++;
			}
			reg[n].start = start;
			reg[n].end = i;
			reg[n].type = SYN_COMMENT;
			n++;
			continue;
		}

		/* String literal */
		if (c == '"' || c == '\'') {
			unicode_t quote = c;

			start = i++;
			while (i < len) {
				if (text[i] == '\\' && i + 1 < len) {
					i += 2;
					continue;
				}
				if (text[i] == quote) {
					i++;
					break;
				}
				i++;
			}
			reg[n].start = start;
			reg[n].end = i;
			reg[n].type = SYN_STRING;
			n++;
			continue;
		}

		/* Number: starts with digit or decimal point followed by digit */
		if (is_digit(c) || (c == '.' && is_digit(next))) {
			start = i;
			if (c == '0' && (next | 0x20) == 'x') {
				/* Hex: 0x... */
				i += 2;
				while (i < len && is_xdigit(text[i]))
					i++;
			} else {
				/* Decimal or float */
				while (i < len && is_digit(text[i]))
					i++;
				if (i < len && text[i] == '.') {
					i++;
					while (i < len && is_digit(text[i]))
						i++;
				}
				if (i < len && (text[i] | 0x20) == 'e') {
					i++;
					if (i < len &&
					    (text[i] == '+' || text[i] == '-'))
						i++;
					while (i < len && is_digit(text[i]))
						i++;
				}
			}
			/* Type suffix: u, l, f */
			while (i < len) {
				unicode_t s = text[i] | 0x20;

				if (s != 'u' && s != 'l' && s != 'f')
					break;
				i++;
			}
			reg[n].start = start;
			reg[n].end = i;
			reg[n].type = SYN_NUMBER;
			n++;
			continue;
		}

		/* Identifier - check for keyword/type */
		if (is_ident_start(c)) {
			char word[16];
			int wlen = 0;

			start = i;
			while (i < len && is_ident(text[i])) {
				unicode_t ch = text[i];

				/* Only ASCII identifiers can be keywords */
				if (ch < 128 && wlen < (int)sizeof(word) - 1)
					word[wlen++] = ch;
				i++;
			}
			word[wlen] = '\0';

			/* Skip keyword check if identifier was truncated or
			 * contained non-ASCII */
			if (wlen > 0 && wlen == i - start) {
				if (bsearch_str(word, keywords,
						ARRAY_SIZE(keywords))) {
					reg[n].start = start;
					reg[n].end = i;
					reg[n].type = SYN_KEYWORD;
					n++;
				} else if (bsearch_str(word, types,
						       ARRAY_SIZE(types))) {
					reg[n].start = start;
					reg[n].end = i;
					reg[n].type = SYN_TYPE;
					n++;
				}
			}
			continue;
		}

		i++;
	}

	return n;
}

int syn_getcolor(enum syn_token type)
{
	if (type < SYN_MAX)
		return colors[type];
	return colors[SYN_NORMAL];
}
