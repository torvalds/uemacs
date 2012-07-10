#ifndef UTF8_H
#define UTF8_H

typedef unsigned int unicode_t;

unsigned utf8_to_unicode(char *line, unsigned index, unsigned len, unicode_t *res);
unsigned unicode_to_utf8(unsigned int c, char *utf8);

static inline int is_beginning_utf8(unsigned char c)
{
	return (c & 0xc0) != 0x80;
}

#endif
