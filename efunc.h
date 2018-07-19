/*	efunc.h
 *
 *	Function declarations and names.
 *
 *	This file list all the C code functions used and the names to use
 *      to bind keys to them. To add functions,	declare it here in both the
 *      extern function list and the name binding table.
 *
 *	modified by Petri Kutvonen
 */

/* External function declarations. */

/* word.c */
extern int wrapword(int f, int n);
extern int backword(int f, int n);
extern int forwword(int f, int n);
extern int upperword(int f, int n);
extern int lowerword(int f, int n);
extern int capword(int f, int n);
extern int delfword(int f, int n);
extern int delbword(int f, int n);
extern int inword(void);
extern int fillpara(int f, int n);
extern int justpara(int f, int n);
extern int killpara(int f, int n);
extern int wordcount(int f, int n);

/* window.c */
extern int reposition(int f, int n);
extern int redraw(int f, int n);
extern int nextwind(int f, int n);
extern int prevwind(int f, int n);
extern int mvdnwind(int f, int n);
extern int mvupwind(int f, int n);
extern int onlywind(int f, int n);
extern int delwind(int f, int n);
extern int splitwind(int f, int n);
extern int enlargewind(int f, int n);
extern int shrinkwind(int f, int n);
extern int resize(int f, int n);
extern int scrnextup(int f, int n);
extern int scrnextdw(int f, int n);
extern int savewnd(int f, int n);
extern int restwnd(int f, int n);
extern int newsize(int f, int n);
extern int newwidth(int f, int n);
extern int getwpos(void);
extern void cknewwindow(void);
extern struct window *wpopup(void);  /* Pop up window creation. */


/* basic.c */
extern int gotobol(int f, int n);
extern int backchar(int f, int n);
extern int gotoeol(int f, int n);
extern int forwchar(int f, int n);
extern int gotoline(int f, int n);
extern int gotobob(int f, int n);
extern int gotoeob(int f, int n);
extern int forwline(int f, int n);
extern int backline(int f, int n);
extern int gotobop(int f, int n);
extern int gotoeop(int f, int n);
extern int forwpage(int f, int n);
extern int backpage(int f, int n);
extern int setmark(int f, int n);
extern int swapmark(int f, int n);

/* random.c */
extern int tabsize;  /* Tab size (0: use real tabs). */
extern int setfillcol(int f, int n);
extern int showcpos(int f, int n);
extern int getcline(void);
extern int getccol(int bflg);
extern int setccol(int pos);
extern int twiddle(int f, int n);
extern int quote(int f, int n);
extern int insert_tab(int f, int n);
extern int detab(int f, int n);
extern int entab(int f, int n);
extern int trim(int f, int n);
extern int openline(int f, int n);
extern int insert_newline(int f, int n);
extern int cinsert(void);
extern int insbrace(int n, int c);
extern int inspound(void);
extern int deblank(int f, int n);
extern int indent(int f, int n);
extern int forwdel(int f, int n);
extern int backdel(int f, int n);
extern int killtext(int f, int n);
extern int setemode(int f, int n);
extern int delmode(int f, int n);
extern int setgmode(int f, int n);
extern int delgmode(int f, int n);
extern int adjustmode(int kind, int global);
extern int clrmes(int f, int n);
extern int writemsg(int f, int n);
extern int getfence(int f, int n);
extern int fmatch(int ch);
extern int istring(int f, int n);
extern int ovstring(int f, int n);

/* main.c */
extern void edinit(char *bname);
extern int execute(int c, int f, int n);
extern int quickexit(int f, int n);
extern int quit(int f, int n);
extern int ctlxlp(int f, int n);
extern int ctlxrp(int f, int n);
extern int ctlxe(int f, int n);
extern int ctrlg(int f, int n);
extern int rdonly(void);
extern int resterr(void);
extern int nullproc(int f, int n);
extern int metafn(int f, int n);
extern int cex(int f, int n);
extern int unarg(int f, int n);
extern int cexit(int status);

/* display.c */
extern void vtinit(void);
extern void vtfree(void);
extern void vttidy(void);
extern void vtmove(int row, int col);
extern int upscreen(int f, int n);
extern int update(int force);
extern void updpos(void);
extern void upddex(void);
extern void updgar(void);
extern int updupd(int force);
extern void upmode(void);
extern void movecursor(int row, int col);
extern void mlerase(void);
extern void mlwrite(const char *fmt, ...);
extern void mlforce(char *s);
extern void mlputs(char *s);
extern void getscreensize(int *widthp, int *heightp);
extern void sizesignal(int signr);

/* region.c */
extern int killregion(int f, int n);
extern int copyregion(int f, int n);
extern int lowerregion(int f, int n);
extern int upperregion(int f, int n);
extern int getregion(struct region *rp);

/* posix.c */
extern void ttopen(void);
extern void ttclose(void);
extern int ttputc(int c);
extern void ttflush(void);
extern int ttgetc(void);
extern int typahead(void);

/* input.c */
extern int mlyesno(char *prompt);
extern int mlreply(char *prompt, char *buf, int nbuf);
extern int mlreplyt(char *prompt, char *buf, int nbuf, int eolchar);
extern int ectoc(int c);
extern int ctoec(int c);
extern fn_t getname(void);
extern int tgetc(void);
extern int get1key(void);
extern int getcmd(void);
extern int getstring(char *prompt, char *buf, int nbuf, int eolchar);
extern void outstring(char *s);
extern void ostring(char *s);

/* bind.c */
extern int help(int f, int n);
extern int deskey(int f, int n);
extern int bindtokey(int f, int n);
extern int unbindkey(int f, int n);
extern int unbindchar(int c);
extern int desbind(int f, int n);
extern int apro(int f, int n);
extern int buildlist(int type, char *mstring);
extern int strinc(char *source, char *sub);
extern unsigned int getckey(int mflag);
extern int startup(char *sfname);
extern char *flook(char *fname, int hflag);
extern void cmdstr(int c, char *seq);
extern fn_t getbind(int c);
extern char *getfname(fn_t);
extern fn_t fncmatch(char *);
extern unsigned int stock(char *keyname);
extern char *transbind(char *skey);

/* buffer.c */
extern int usebuffer(int f, int n);
extern int nextbuffer(int f, int n);
extern int swbuffer(struct buffer *bp);
extern int killbuffer(int f, int n);
extern int zotbuf(struct buffer *bp);
extern int namebuffer(int f, int n);
extern int listbuffers(int f, int n);
extern int makelist(int iflag);
extern void ltoa(char *buf, int width, long num);
extern int addline(char *text);
extern int anycb(void);
extern int bclear(struct buffer *bp);
extern int unmark(int f, int n);
/* Lookup a buffer by name. */
extern struct buffer *bfind(char *bname, int cflag, int bflag);

/* file.c */
extern int fileread(int f, int n);
extern int insfile(int f, int n);
extern int filefind(int f, int n);
extern int viewfile(int f, int n);
extern int getfile(char *fname, int lockfl);
extern int readin(char *fname, int lockfl);
extern void makename(char *bname, char *fname);
extern void unqname(char *name);
extern int filewrite(int f, int n);
extern int filesave(int f, int n);
extern int writeout(char *fn);
extern int filename(int f, int n);
extern int ifile(char *fname);

/* fileio.c */
extern int ffropen(char *fn);
extern int ffwopen(char *fn);
extern int ffclose(void);
extern int ffputline(char *buf, int nbuf);
extern int ffgetline(void);
extern int fexist(char *fname);

/* exec.c */
extern int namedcmd(int f, int n);
extern int execcmd(int f, int n);
extern int docmd(char *cline);
extern char *token(char *src, char *tok, int size);
extern int macarg(char *tok);
extern int nextarg(char *prompt, char *buffer, int size, int terminator);
extern int storemac(int f, int n);
extern int storeproc(int f, int n);
extern int execproc(int f, int n);
extern int execbuf(int f, int n);
extern int dobuf(struct buffer *bp);
extern void freewhile(struct while_block *wp);
extern int execfile(int f, int n);
extern int dofile(char *fname);
extern int cbuf(int f, int n, int bufnum);
extern int cbuf1(int f, int n);
extern int cbuf2(int f, int n);
extern int cbuf3(int f, int n);
extern int cbuf4(int f, int n);
extern int cbuf5(int f, int n);
extern int cbuf6(int f, int n);
extern int cbuf7(int f, int n);
extern int cbuf8(int f, int n);
extern int cbuf9(int f, int n);
extern int cbuf10(int f, int n);
extern int cbuf11(int f, int n);
extern int cbuf12(int f, int n);
extern int cbuf13(int f, int n);
extern int cbuf14(int f, int n);
extern int cbuf15(int f, int n);
extern int cbuf16(int f, int n);
extern int cbuf17(int f, int n);
extern int cbuf18(int f, int n);
extern int cbuf19(int f, int n);
extern int cbuf20(int f, int n);
extern int cbuf21(int f, int n);
extern int cbuf22(int f, int n);
extern int cbuf23(int f, int n);
extern int cbuf24(int f, int n);
extern int cbuf25(int f, int n);
extern int cbuf26(int f, int n);
extern int cbuf27(int f, int n);
extern int cbuf28(int f, int n);
extern int cbuf29(int f, int n);
extern int cbuf30(int f, int n);
extern int cbuf31(int f, int n);
extern int cbuf32(int f, int n);
extern int cbuf33(int f, int n);
extern int cbuf34(int f, int n);
extern int cbuf35(int f, int n);
extern int cbuf36(int f, int n);
extern int cbuf37(int f, int n);
extern int cbuf38(int f, int n);
extern int cbuf39(int f, int n);
extern int cbuf40(int f, int n);

/* spawn.c */
extern int spawncli(int f, int n);
extern int bktoshell(int f, int n);
extern void rtfrmshell(void);
extern int spawn(int f, int n);
extern int execprg(int f, int n);
extern int pipecmd(int f, int n);
extern int filter_buffer(int f, int n);
extern int sys(char *cmd);
extern int shellprog(char *cmd);
extern int execprog(char *cmd);

/* search.c */
extern int forwsearch(int f, int n);
extern int forwhunt(int f, int n);
extern int backsearch(int f, int n);
extern int backhunt(int f, int n);
extern int mcscanner(struct magic *mcpatrn, int direct, int beg_or_end);
extern int scanner(const char *patrn, int direct, int beg_or_end);
extern int eq(unsigned char bc, unsigned char pc);
extern void savematch(void);
extern void rvstrcpy(char *rvstr, char *str);
extern int sreplace(int f, int n);
extern int qreplace(int f, int n);
extern int delins(int dlength, char *instr, int use_meta);
extern int expandp(char *srcstr, char *deststr, int maxlength);
extern int boundry(struct line *curline, int curoff, int dir);
extern void mcclear(void);
extern void rmcclear(void);

/* isearch.c */
extern int risearch(int f, int n);
extern int fisearch(int f, int n);
extern int isearch(int f, int n);
extern int checknext(char chr, char *patrn, int dir);
extern int scanmore(char *patrn, int dir);
extern int match_pat(char *patrn);
extern int promptpattern(char *prompt);
extern int get_char(void);
extern int uneat(void);
extern void reeat(int c);

/* eval.c */
extern void varinit(void);
extern char *gtfun(char *fname);
extern char *gtusr(char *vname);
extern char *gtenv(char *vname);
extern char *getkill(void);
extern int setvar(int f, int n);
extern void findvar(char *var, struct variable_description *vd, int size);
extern int svar(struct variable_description *var, char *value);
extern char *itoa(int i);
extern int gettyp(char *token);
extern char *getval(char *token, char *result, int size);
extern int stol(char *val);
extern char *ltos(int val);
extern char *mkupper(char *str);
extern char *mklower(char *str);
extern int abs(int x);
extern int ernd(void);
extern int sindex(char *source, char *pattern);
extern char *xlat(char *source, char *lookup, char *trans);

/* crypt.c */
extern int set_encryption_key(int f, int n);
extern void myencrypt(char *bptr, unsigned len);

/* lock.c */
extern int lockchk(char *fname);
extern int lockrel(void);
extern int lock(char *fname);
extern int unlock(char *fname);
extern void lckerror(char *errstr);

/* pklock.c */
extern char *dolock(char *fname);
extern char *undolock(char *fname);
