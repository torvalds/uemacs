#include "utf8.h"

/*
 * utf8_to_unicode()
 *
 * Convert a UTF-8 sequence to its unicode value, and return the length of
 * the sequence in bytes.
 *
 * NOTE! Invalid UTF-8 will be converted to a one-byte sequence, so you can
 * either use it as-is (ie as Latin1) or you can check for invalid UTF-8
 * by checking for a length of 1 and a result > 127.
 *
 * NOTE 2! This does *not* verify things like minimality. So overlong forms
 * are happily accepted and decoded, as are the various "invalid values".
 */
unsigned utf8_to_unicode(char *line, unsigned index, unsigned len, unicode_t *res)
{
	unsigned value;
	unsigned char c = line[index];
	unsigned bytes, mask, i;

	*res = c;
	line += index;
	len -= index;

	/*
	 * 0xxxxxxx is valid utf8
	 * 10xxxxxx is invalid UTF-8, we assume it is Latin1
	 */
	if (c < 0xc0)
		return 1;

	/* Ok, it's 11xxxxxx, do a stupid decode */
	mask = 0x20;
	bytes = 2;
	while (c & mask) {
		bytes++;
		mask >>= 1;
	}

	/* Invalid? Do it as a single byte Latin1 */
	if (bytes > 6)
		return 1;
	if (bytes > len)
		return 1;

	value = c & (mask-1);

	/* Ok, do the bytes */
	for (i = 1; i < bytes; i++) {
		c = line[i];
		if ((c & 0xc0) != 0x80)
			return 1;
		value = (value << 6) | (c & 0x3f);
	}
	*res = value;
	return bytes;
}

static void reverse_string(char *begin, char *end)
{
	do {
		char a = *begin, b = *end;
		*end = a; *begin = b;
		begin++; end--;
	} while (begin < end);
}

/*
 * unicode_to_utf8()
 *
 * Convert a unicode value to its canonical utf-8 sequence.
 *
 * NOTE! This does not check for - or care about - the "invalid" unicode
 * values.  Also, converting a utf-8 sequence to unicode and back does
 * *not* guarantee the same sequence, since this generates the shortest
 * possible sequence, while utf8_to_unicode() accepts both Latin1 and
 * overlong utf-8 sequences.
 */
unsigned unicode_to_utf8(unsigned int c, char *utf8)
{
	int bytes = 1;

	*utf8 = c;
	if (c > 0x7f) {
		int prefix = 0x40;
		char *p = utf8;
		do {
			*p++ = 0x80 + (c & 0x3f);
			bytes++;
			prefix >>= 1;
			c >>= 6;
		} while (c > prefix);
		*p = c - 2*prefix;
		reverse_string(utf8, p);
	}
	return bytes;
}

#define WIDTH_ROW 38

static unsigned int widths[WIDTH_ROW][2] = {
	{126,    1}, {159,    0}, {687,     1}, {710,   0}, {711,   1},
	{727,    0}, {733,    1}, {879,     0}, {1154,  1}, {1161,  0},
	{4347,   1}, {4447,   2}, {7467,    1}, {7521,  0}, {8369,  1},
	{8426,   0}, {9000,   1}, {9002,    2}, {11021, 1}, {12350, 2},
	{12351,  1}, {12438,  2}, {12442,   0}, {19893, 2}, {19967, 1},
	{55203,  2}, {63743,  1}, {64106,   2}, {65039, 1}, {65059, 0},
	{65131,  2}, {65279,  1}, {65376,   2}, {65500, 1}, {65510, 2},
	{120831, 1}, {262141, 2}, {1114109, 1}
};

/*
 * char_width()
 *
 * Get the character display width.
 */
int char_width(unsigned int c)
{
	int i;

	if (c == 0xe || c == 0xf)
		return 0;

	for (i = 0; i < WIDTH_ROW; i++) {
		if (c <= widths[i][0])
			return widths[i][1];
	}
	return 1;
}
