#ifndef LINE_H_
#define LINE_H_

#include "utf8.h"

/*
 * All text is kept in circularly linked lists of "struct line" structures. These
 * begin at the header line (which is the blank line beyond the end of the
 * buffer). This line is pointed to by the "struct buffer". Each line contains a the
 * number of bytes in the line (the "used" size), the size of the text array,
 * and the text. The end of line is not stored as a byte; it's implied. Future
 * additions will include update hints, and a list of marks into the line.
 */
struct line {
	struct line *l_fp;			/* Link to the next line        */
	struct line *l_bp;			/* Link to the previous line    */
	int l_size;				/* Allocated size               */
	int l_used;				/* Used size                    */
	char l_text[1];				/* A bunch of characters.       */
};

#define line_next(lp)       ((lp)->l_fp)
#define line_prev(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)]&0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define line_length(lp)     ((lp)->l_used)

extern void line_free(struct line *lp);
extern void buffer_changed(int flag);
extern int cmd_insert_space(int f, int n);
extern int insert_string(char *instr);
extern int insert_char(int n, int c);
extern int overwrite_char(int c);
extern int overwrite_string(char *ostr);
extern int insert_newline(void);
extern int delete_bytes(long n, int kflag);
extern int delete_characters(long n, int kflag);
extern int char_at_dot(unicode_t *);
extern char *getctext(void);
extern int putctext(char *iline);
extern int delete_newline(void);
extern void kdelete(void);
extern int kinsert(int c);
extern int cmd_yank(int f, int n);
extern struct line *line_alloc(int);		/* Allocate a line. */

#endif				/* LINE_H_ */
