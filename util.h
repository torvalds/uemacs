#ifndef UTIL_H_
#define UTIL_H_

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

// Overly simplistic "how does the column number change
// based on character 'c'" function
static inline int next_column(int old, unicode_t c)
{
	if (c == '\t')
		return (old | tabmask) + 1;

	if (c < 0x20 || c == 0x7F)
		return old + 2;

	if (c >= 0x80 && c <= 0xa0)
		return old + 3;

	return old + 1;
}

#endif  /* UTIL_H_ */
