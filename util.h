#ifndef UTIL_H_
#define UTIL_H_

#include <string.h>
#include <wchar.h>
#include "utf8.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/* Safe zeroing, no complaining about overlap */
static inline void mystrscpy(char *dst, const char *src, int size)
{
	if (!size)
		return;
	while (--size) {
		char c = *src++;
		if (!c)
			break;
		*dst++ = c;
	}
	*dst = 0;
}

/* Append to it, with 'size' being the whole of dst rather than what is left */
static inline void mystrscat(char *dst, const char *src, int size)
{
	int len = strlen(dst);

	if (len < size)
		mystrscpy(dst + len, src, size - len);
}

/* Return the display width of a Unicode codepoint */
static inline int utf8_width(unicode_t c)
{
	int w = wcwidth((wchar_t)c);
	if (w < 0)
		return 1;
	return w;
}

// "how does the column number change based on character 'c'"
static inline int next_column(int old, unicode_t c)
{
	if (c == '\t')
		return (old | tabmask) + 1;

	if (c < 0x20 || c == 0x7F)
		return old + 2;

	if (c >= 0x80 && c <= 0xa0)
		return old + 3;

	return old + utf8_width(c);
}

#endif				/* UTIL_H_ */
