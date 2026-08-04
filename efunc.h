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
extern int cmd_wrap_word(int f, int n);
extern int cmd_previous_word(int f, int n);
extern int cmd_next_word(int f, int n);
extern int cmd_case_word_upper(int f, int n);
extern int cmd_case_word_lower(int f, int n);
extern int cmd_case_word_capitalize(int f, int n);
extern int cmd_delete_next_word(int f, int n);
extern int cmd_delete_previous_word(int f, int n);
extern int inword(void);
extern int cmd_fill_paragraph(int f, int n);
extern int cmd_justify_paragraph(int f, int n);
extern int cmd_kill_paragraph(int f, int n);
extern int cmd_count_words(int f, int n);

/* window.c */
extern int cmd_split_current_window(int f, int n);
extern int cmd_next_window(int f, int n);
extern int cmd_previous_window(int f, int n);
extern int cmd_delete_other_windows(int f, int n);
extern int cmd_delete_window(int f, int n);
extern int cmd_grow_window(int f, int n);
extern int cmd_shrink_window(int f, int n);
extern int cmd_move_window_up(int f, int n);
extern int cmd_move_window_down(int f, int n);
extern int cmd_resize_window(int f, int n);
extern int cmd_scroll_next_up(int f, int n);
extern int cmd_scroll_next_down(int f, int n);
extern int cmd_save_window(int f, int n);
extern int cmd_restore_window(int f, int n);
extern int cmd_redraw_display(int f, int n);
extern int cmd_clear_and_redraw(int f, int n);
extern int cmd_change_screen_size(int f, int n);
extern int cmd_change_screen_width(int f, int n);

/* basic.c */
extern int cmd_beginning_of_line(int f, int n);
extern int cmd_backward_character(int f, int n);
extern int cmd_end_of_line(int f, int n);
extern int cmd_forward_character(int f, int n);
extern int cmd_goto_line(int f, int n);
extern int cmd_beginning_of_file(int f, int n);
extern int cmd_end_of_file(int f, int n);
extern int cmd_next_line(int f, int n);
extern int cmd_previous_line(int f, int n);
extern int cmd_previous_paragraph(int f, int n);
extern int cmd_next_paragraph(int f, int n);
extern int cmd_next_page(int f, int n);
extern int cmd_previous_page(int f, int n);
extern int cmd_set_mark(int f, int n);
extern int cmd_exchange_point_and_mark(int f, int n);

/* random.c */
extern int tabsize;				/* Tab size (0: use real tabs). */
extern int cmd_set_fill_column(int f, int n);
extern int cmd_buffer_position(int f, int n);
extern int getcline(void);
extern int getccol(int bflg);
extern int setccol(int pos);
extern int cmd_transpose_characters(int f, int n);
extern int cmd_quote_character(int f, int n);
extern int cmd_handle_tab(int f, int n);
extern int cmd_detab_line(int f, int n);
extern int cmd_entab_line(int f, int n);
extern int cmd_trim_line(int f, int n);
extern int cmd_open_line(int f, int n);
extern int cmd_newline(int f, int n);
extern int cinsert(void);
extern int insbrace(int n, int c);
extern int inspound(void);
extern int cmd_delete_blank_lines(int f, int n);
extern int cmd_newline_and_indent(int f, int n);
extern int cmd_delete_next_character(int f, int n);
extern int cmd_delete_previous_character(int f, int n);
extern int cmd_kill_to_end_of_line(int f, int n);
extern int cmd_add_mode(int f, int n);
extern int cmd_delete_mode(int f, int n);
extern int cmd_add_global_mode(int f, int n);
extern int cmd_delete_global_mode(int f, int n);
extern int adjustmode(int kind, int global);
extern int cmd_clear_message_line(int f, int n);
extern int cmd_write_message(int f, int n);
extern int cmd_goto_matching_fence(int f, int n);
extern int fmatch(int ch);
extern int cmd_insert_string(int f, int n);
extern int cmd_overwrite_string(int f, int n);

/* main.c */
extern int spellcheck(const char *word);
extern void edinit(char *bname);
extern int execute(int c, int f, int n);
extern int cmd_quick_exit(int f, int n);
extern int cmd_exit_emacs(int f, int n);
extern int cmd_begin_macro(int f, int n);
extern int cmd_end_macro(int f, int n);
extern int cmd_execute_macro(int f, int n);
extern int cmd_abort_command(int f, int n);
extern int rdonly(void);
extern int resterr(void);
extern int cmd_nop(int f, int n);
extern int cmd_meta_prefix(int f, int n);
extern int cmd_ctlx_prefix(int f, int n);
extern int cmd_universal_argument(int f, int n);

/* display.c */
extern void display_open(void);
extern void display_close(void);
extern int cmd_update_screen(int f, int n);
extern void update(void);
extern void update_now(void);
extern void update_modeline(void);
extern void movecursor(int row, int col);
extern void msg_erase(void);
extern void msg_printf(const char *fmt, ...);
extern void msg_force(char *s);
extern void msg_append(const char *s);
extern void msg_puts(const char *s);
extern void getscreensize(int *widthp, int *heightp);
extern void sizesignal(int signr);
extern void checkwinsize(void);

/* region.c */
extern int cmd_kill_region(int f, int n);
extern int cmd_copy_region(int f, int n);
extern int cmd_case_region_lower(int f, int n);
extern int cmd_case_region_upper(int f, int n);
extern int getregion(struct region *rp);

/* tcap.c */
extern void tcapopen(void);
extern void tcapclose(void);
extern void tcapkopen(void);
extern void tcapkclose(void);
extern void tcapmove(int row, int col);
extern void tcapeeol(void);
extern void tcapeeop(void);
extern void tcapbeep(void);
extern void tcaprev(int state);

/* posix.c */
extern void ttopen(void);
extern void ttclose(void);
extern int ttputc(int c);
extern void ttflush(void);
extern void ttpause(void);
extern int ttgetc(void);
extern void ttungetc(int c);
extern int typahead(void);

/* input.c */
extern int ask_yesno(char *prompt);
extern int ask_string(char *prompt, char *buf, int nbuf);
extern int ask_string_until(char *prompt, char *buf, int nbuf, int eolchar);
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
extern int cmd_describe_key(int f, int n);
extern int cmd_bind_to_key(int f, int n);
extern int cmd_unbind_key(int f, int n);
extern int unbindchar(int c);
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
extern int cmd_select_buffer(int f, int n);
extern int cmd_next_buffer(int f, int n);
extern int swbuffer(struct buffer *bp);
extern void shown_buffer_changed(void);
extern int cmd_delete_buffer(int f, int n);
extern int zotbuf(struct buffer *bp);
extern int cmd_name_buffer(int f, int n);
extern int makelist(int iflag);
extern int cmd_list_buffers(int f, int n);
extern void ltoa(char *buf, int width, long num);
extern int addline(char *text);
extern int anycb(void);
extern int bclear(struct buffer *bp);
extern int cmd_unmark_buffer(int f, int n);
/* Lookup a buffer by name. */
extern struct buffer *bfind(char *bname, int cflag, int bflag);

/* file.c */
extern int cmd_read_file(int f, int n);
extern int cmd_insert_file(int f, int n);
extern int cmd_find_file(int f, int n);
extern int cmd_view_file(int f, int n);
extern int getfile(char *fname, int lockfl);
extern int readin(char *fname, int lockfl);
extern void makename(char *bname, char *fname);
extern void unqname(char *name);
extern int cmd_write_file(int f, int n);
extern int cmd_save_file(int f, int n);
extern int writeout(char *fn);
extern int cmd_change_file_name(int f, int n);
extern int ifile(char *fname);
extern int file_changed(struct buffer *bp, char *fn);

/* fileio.c */
extern int file_open_read(char *fn);
extern int file_open_write(char *fn);
extern int file_close(void);
extern int file_put_line(char *buf, int nbuf);
extern int file_get_line(void);
extern int file_exists(char *fname);

/* exec.c */
extern int cmd_execute_named_command(int f, int n);
extern int cmd_execute_command_line(int f, int n);
extern int docmd(char *cline);
extern char *token(char *src, char *tok, int size);
extern int macarg(char *tok);
extern int nextarg(char *prompt, char *buffer, int size, int terminator);
extern int cmd_store_macro(int f, int n);
extern int cmd_store_procedure(int f, int n);
extern int cmd_execute_procedure(int f, int n);
extern int cmd_execute_buffer(int f, int n);
extern int dobuf(struct buffer *bp);
extern void freewhile(struct while_block *wp);
extern int cmd_execute_file(int f, int n);
extern int dofile(char *fname);
extern int cbuf(int f, int n, int bufnum);
extern int cmd_execute_macro_1(int f, int n);
extern int cmd_execute_macro_2(int f, int n);
extern int cmd_execute_macro_3(int f, int n);
extern int cmd_execute_macro_4(int f, int n);
extern int cmd_execute_macro_5(int f, int n);
extern int cmd_execute_macro_6(int f, int n);
extern int cmd_execute_macro_7(int f, int n);
extern int cmd_execute_macro_8(int f, int n);
extern int cmd_execute_macro_9(int f, int n);
extern int cmd_execute_macro_10(int f, int n);
extern int cmd_execute_macro_11(int f, int n);
extern int cmd_execute_macro_12(int f, int n);
extern int cmd_execute_macro_13(int f, int n);
extern int cmd_execute_macro_14(int f, int n);
extern int cmd_execute_macro_15(int f, int n);
extern int cmd_execute_macro_16(int f, int n);
extern int cmd_execute_macro_17(int f, int n);
extern int cmd_execute_macro_18(int f, int n);
extern int cmd_execute_macro_19(int f, int n);
extern int cmd_execute_macro_20(int f, int n);
extern int cmd_execute_macro_21(int f, int n);
extern int cmd_execute_macro_22(int f, int n);
extern int cmd_execute_macro_23(int f, int n);
extern int cmd_execute_macro_24(int f, int n);
extern int cmd_execute_macro_25(int f, int n);
extern int cmd_execute_macro_26(int f, int n);
extern int cmd_execute_macro_27(int f, int n);
extern int cmd_execute_macro_28(int f, int n);
extern int cmd_execute_macro_29(int f, int n);
extern int cmd_execute_macro_30(int f, int n);
extern int cmd_execute_macro_31(int f, int n);
extern int cmd_execute_macro_32(int f, int n);
extern int cmd_execute_macro_33(int f, int n);
extern int cmd_execute_macro_34(int f, int n);
extern int cmd_execute_macro_35(int f, int n);
extern int cmd_execute_macro_36(int f, int n);
extern int cmd_execute_macro_37(int f, int n);
extern int cmd_execute_macro_38(int f, int n);
extern int cmd_execute_macro_39(int f, int n);
extern int cmd_execute_macro_40(int f, int n);

/* spawn.c */
extern int cmd_interactive_shell(int f, int n);
extern int cmd_suspend_emacs(int f, int n);
extern void rtfrmshell(void);
extern int cmd_shell_command(int f, int n);
extern int cmd_execute_program(int f, int n);
extern int cmd_filter_buffer(int f, int n);

/* search.c */
extern int cmd_search_forward(int f, int n);
extern int cmd_hunt_forward(int f, int n);
extern int cmd_search_reverse(int f, int n);
extern int cmd_hunt_backward(int f, int n);
extern int mcscanner(struct magic *mcpatrn, int direct, int beg_or_end);
extern int scanner(const char *patrn, int direct, int beg_or_end);
extern int eq(unsigned char bc, unsigned char pc);
extern void savematch(void);
extern void rvstrcpy(char *rvstr, char *str);
extern int cmd_replace_string(int f, int n);
extern int cmd_query_replace_string(int f, int n);
extern int delins(int dlength, char *instr, int use_meta);
extern int expandp(char *srcstr, char *deststr, int maxlength);
extern int boundry(struct line *curline, int curoff, int dir);
extern void mcclear(void);
extern void rmcclear(void);

/* isearch.c */
extern int cmd_reverse_incremental_search(int f, int n);
extern int cmd_incremental_search(int f, int n);
extern int isearch(int f, int n);
extern int checknext(char chr, char *patrn, int dir);
extern int scanmore(char *patrn, int dir);
extern int match_pat(char *patrn);
extern int promptpattern(char *prompt);
extern int get_char(void);

/* eval.c */
extern void varinit(void);
extern char *gtfun(char *fname);
extern char *gtusr(char *vname);
extern char *gtenv(char *vname);
extern int cmd_set(int f, int n);
extern void findvar(char *var, struct variable_description *vd, int size);
extern int svar(struct variable_description *var, char *value);
extern char *itoa(int i);
extern int gettyp(char *token);
extern char *getval(char *token, char *result, int size);
extern int stol(char *val);
extern char *ltos(int val);
extern char *mkupper(const char *str, char *result);
extern char *mklower(const char *str, char *result);
extern int abs(int x);
extern int ernd(void);
extern int sindex(char *source, char *pattern);
extern char *xlat(char *source, char *lookup, char *trans);

/* lock.c */
extern int lockchk(char *fname);
