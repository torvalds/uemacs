/* Name to function binding table.
 *
 * This table gives the names of all the bindable functions
 * end their C function address. These are used for the bind-to-key
 * function.
*/

#include "estruct.h"
#include "edef.h"
#include "efunc.h"
#include "line.h"

struct name_bind names[] = {
	{"abort-command", ctrlg},
	{"add-mode", setemode},
	{"add-global-mode", setgmode},
#if	APROP
	{"apropos", apro},
#endif
	{"backward-character", backchar},
	{"begin-macro", ctlxlp},
	{"beginning-of-file", gotobob},
	{"beginning-of-line", gotobol},
	{"bind-to-key", bindtokey},
	{"buffer-position", showcpos},
	{"case-region-lower", lowerregion},
	{"case-region-upper", upperregion},
	{"case-word-capitalize", capword},
	{"case-word-lower", lowerword},
	{"case-word-upper", upperword},
	{"change-file-name", filename},
	{"change-screen-size", newsize},
	{"change-screen-width", newwidth},
	{"clear-and-redraw", redraw},
	{"clear-message-line", clrmes},
	{"copy-region", copyregion},
#if	WORDPRO
	{"count-words", wordcount},
#endif
	{"ctlx-prefix", cex},
	{"delete-blank-lines", deblank},
	{"delete-buffer", killbuffer},
	{"delete-mode", delmode},
	{"delete-global-mode", delgmode},
	{"delete-next-character", forwdel},
	{"delete-next-word", delfword},
	{"delete-other-windows", onlywind},
	{"delete-previous-character", backdel},
	{"delete-previous-word", delbword},
	{"delete-window", delwind},
	{"describe-bindings", desbind},
	{"describe-key", deskey},
#if	AEDIT
	{"detab-line", detab},
#endif
	{"end-macro", ctlxrp},
	{"end-of-file", gotoeob},
	{"end-of-line", gotoeol},
#if	AEDIT
	{"entab-line", entab},
#endif
	{"exchange-point-and-mark", swapmark},
	{"execute-buffer", execbuf},
	{"execute-command-line", execcmd},
	{"execute-file", execfile},
	{"execute-macro", ctlxe},
	{"execute-macro-1", cbuf1},
	{"execute-macro-2", cbuf2},
	{"execute-macro-3", cbuf3},
	{"execute-macro-4", cbuf4},
	{"execute-macro-5", cbuf5},
	{"execute-macro-6", cbuf6},
	{"execute-macro-7", cbuf7},
	{"execute-macro-8", cbuf8},
	{"execute-macro-9", cbuf9},
	{"execute-macro-10", cbuf10},
	{"execute-macro-11", cbuf11},
	{"execute-macro-12", cbuf12},
	{"execute-macro-13", cbuf13},
	{"execute-macro-14", cbuf14},
	{"execute-macro-15", cbuf15},
	{"execute-macro-16", cbuf16},
	{"execute-macro-17", cbuf17},
	{"execute-macro-18", cbuf18},
	{"execute-macro-19", cbuf19},
	{"execute-macro-20", cbuf20},
	{"execute-macro-21", cbuf21},
	{"execute-macro-22", cbuf22},
	{"execute-macro-23", cbuf23},
	{"execute-macro-24", cbuf24},
	{"execute-macro-25", cbuf25},
	{"execute-macro-26", cbuf26},
	{"execute-macro-27", cbuf27},
	{"execute-macro-28", cbuf28},
	{"execute-macro-29", cbuf29},
	{"execute-macro-30", cbuf30},
	{"execute-macro-31", cbuf31},
	{"execute-macro-32", cbuf32},
	{"execute-macro-33", cbuf33},
	{"execute-macro-34", cbuf34},
	{"execute-macro-35", cbuf35},
	{"execute-macro-36", cbuf36},
	{"execute-macro-37", cbuf37},
	{"execute-macro-38", cbuf38},
	{"execute-macro-39", cbuf39},
	{"execute-macro-40", cbuf40},
	{"execute-named-command", namedcmd},
#if	PROC
	{"execute-procedure", execproc},
#endif
	{"execute-program", execprg},
	{"exit-emacs", quit},
#if	WORDPRO
	{"fill-paragraph", fillpara},
#endif
	{"filter-buffer", filter_buffer},
	{"find-file", filefind},
	{"forward-character", forwchar},
	{"goto-line", gotoline},
#if	CFENCE
	{"goto-matching-fence", getfence},
#endif
	{"grow-window", enlargewind},
	{"handle-tab", insert_tab},
	{"hunt-forward", forwhunt},
	{"hunt-backward", backhunt},
	{"help", help},
	{"i-shell", spawncli},
#if	ISRCH
	{"incremental-search", fisearch},
#endif
	{"insert-file", insfile},
	{"insert-space", insspace},
	{"insert-string", istring},
#if	WORDPRO
#if	PKCODE
	{"justify-paragraph", justpara},
#endif
	{"kill-paragraph", killpara},
#endif
	{"kill-region", killregion},
	{"kill-to-end-of-line", killtext},
	{"list-buffers", listbuffers},
	{"meta-prefix", metafn},
	{"move-window-down", mvdnwind},
	{"move-window-up", mvupwind},
	{"name-buffer", namebuffer},
	{"newline", insert_newline},
	{"newline-and-indent", indent},
	{"next-buffer", nextbuffer},
	{"next-line", forwline},
	{"next-page", forwpage},
#if	WORDPRO
	{"next-paragraph", gotoeop},
#endif
	{"next-window", nextwind},
	{"next-word", forwword},
	{"nop", nullproc},
	{"open-line", openline},
	{"overwrite-string", ovstring},
	{"pipe-command", pipecmd},
	{"previous-line", backline},
	{"previous-page", backpage},
#if	WORDPRO
	{"previous-paragraph", gotobop},
#endif
	{"previous-window", prevwind},
	{"previous-word", backword},
	{"query-replace-string", qreplace},
	{"quick-exit", quickexit},
	{"quote-character", quote},
	{"read-file", fileread},
	{"redraw-display", reposition},
	{"resize-window", resize},
	{"restore-window", restwnd},
	{"replace-string", sreplace},
#if	ISRCH
	{"reverse-incremental-search", risearch},
#endif
#if	PROC
	{"run", execproc},
#endif
	{"save-file", filesave},
	{"save-window", savewnd},
	{"scroll-next-up", scrnextup},
	{"scroll-next-down", scrnextdw},
	{"search-forward", forwsearch},
	{"search-reverse", backsearch},
	{"select-buffer", usebuffer},
	{"set", setvar},
#if	CRYPT
	{"set-encryption-key", set_encryption_key},
#endif
	{"set-fill-column", setfillcol},
	{"set-mark", setmark},
	{"shell-command", spawn},
	{"shrink-window", shrinkwind},
	{"split-current-window", splitwind},
	{"store-macro", storemac},
#if	PROC
	{"store-procedure", storeproc},
#endif
#if	BSD | __hpux | SVR4
	{"suspend-emacs", bktoshell},
#endif
	{"transpose-characters", twiddle},
#if	AEDIT
	{"trim-line", trim},
#endif
	{"unbind-key", unbindkey},
	{"universal-argument", unarg},
	{"unmark-buffer", unmark},
	{"update-screen", upscreen},
	{"view-file", viewfile},
	{"wrap-word", wrapword},
	{"write-file", filewrite},
	{"write-message", writemsg},
	{"yank", yank},

	{"", NULL}
};
