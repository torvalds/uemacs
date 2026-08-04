/*	ebind.h
 *
 *	Initial default key to function bindings
 *
 *	Modified by Petri Kutvonen
 */

#ifndef EBIND_H_
#define EBIND_H_

#include "line.h"

/*
 * Command table.
 * This table  is *roughly* in ASCII order, left to right across the
 * characters of the command. This explains the funny location of the
 * control-X commands.
 */
struct key_tab keytab[NBINDS] = {
	{ CONTROL | 'A', cmd_beginning_of_line },
	{ CONTROL | 'B', cmd_backward_character },
	{ CONTROL | 'C', cmd_insert_space },
	{ CONTROL | 'D', cmd_delete_next_character },
	{ CONTROL | 'E', cmd_end_of_line },
	{ CONTROL | 'F', cmd_forward_character },
	{ CONTROL | 'G', cmd_abort_command },
	{ CONTROL | 'H', cmd_delete_previous_character },
	{ CONTROL | 'I', cmd_handle_tab },
	{ CONTROL | 'J', cmd_newline_and_indent },
	{ CONTROL | 'K', cmd_kill_to_end_of_line },
	{ CONTROL | 'L', cmd_clear_and_redraw },
	{ CONTROL | 'M', cmd_newline },
	{ CONTROL | 'N', cmd_next_line },
	{ CONTROL | 'O', cmd_open_line },
	{ CONTROL | 'P', cmd_previous_line },
	{ CONTROL | 'Q', cmd_quote_character },
	{ CONTROL | 'R', cmd_search_reverse },
	{ CONTROL | 'S', cmd_search_forward },
	{ CONTROL | 'T', cmd_transpose_characters },
	{ CONTROL | 'U', cmd_universal_argument },
	{ CONTROL | 'V', cmd_next_page },
	{ CONTROL | 'W', cmd_kill_region },
	{ CONTROL | 'X', cmd_ctlx_prefix },
	{ CONTROL | 'Y', cmd_yank },
	{ CONTROL | 'Z', cmd_previous_page },
	{ CONTROL | ']', cmd_meta_prefix },
	{ CTLX | CONTROL | 'B', cmd_list_buffers },
	{ CTLX | CONTROL | 'C', cmd_exit_emacs },		/* Hard quit.           */
	{ CTLX | CONTROL | 'A', cmd_detab_line },
	{ CTLX | CONTROL | 'D', cmd_save_file },	/* alternative          */
	{ CTLX | CONTROL | 'E', cmd_entab_line },
	{ CTLX | CONTROL | 'F', cmd_find_file },
	{ CTLX | CONTROL | 'I', cmd_insert_file },
	{ CTLX | CONTROL | 'L', cmd_case_region_lower },
	{ CTLX | CONTROL | 'M', cmd_delete_mode },
	{ CTLX | CONTROL | 'N', cmd_move_window_down },
	{ CTLX | CONTROL | 'O', cmd_delete_blank_lines },
	{ CTLX | CONTROL | 'P', cmd_move_window_up },
	{ CTLX | CONTROL | 'R', cmd_read_file },
	{ CTLX | CONTROL | 'S', cmd_save_file },
	{ CTLX | CONTROL | 'T', cmd_trim_line },
	{ CTLX | CONTROL | 'U', cmd_case_region_upper },
	{ CTLX | CONTROL | 'V', cmd_view_file },
	{ CTLX | CONTROL | 'W', cmd_write_file },
	{ CTLX | CONTROL | 'X', cmd_exchange_point_and_mark },
	{ CTLX | CONTROL | 'Z', cmd_shrink_window },
	{ CTLX | '0', cmd_delete_window },
	{ CTLX | '1', cmd_delete_other_windows },
	{ CTLX | '2', cmd_split_current_window },
	{ CTLX | '?', cmd_describe_key },
	{ CTLX | '!', cmd_shell_command },
	{ CTLX | '#', cmd_filter_buffer },
	{ CTLX | '$', cmd_execute_program },
	{ CTLX | '=', cmd_buffer_position },
	{ CTLX | '(', cmd_begin_macro },
	{ CTLX | ')', cmd_end_macro },
	{ CTLX | 'A', cmd_set },
	{ CTLX | 'B', cmd_select_buffer },
	{ CTLX | 'C', cmd_interactive_shell },
	{ CTLX | 'D', cmd_suspend_emacs },
	{ CTLX | 'E', cmd_execute_macro },
	{ CTLX | 'F', cmd_set_fill_column },
	{ CTLX | 'K', cmd_delete_buffer },
	{ CTLX | 'M', cmd_add_mode },
	{ CTLX | 'N', cmd_change_file_name },
	{ CTLX | 'O', cmd_next_window },
	{ CTLX | 'P', cmd_previous_window },
	{ CTLX | 'Q', cmd_quote_character },			/* alternative  */
	{ CTLX | 'R', cmd_reverse_incremental_search },
	{ CTLX | 'S', cmd_incremental_search },
	{ CTLX | 'W', cmd_resize_window },
	{ CTLX | 'X', cmd_next_buffer },
	{ CTLX | 'Z', cmd_grow_window },
	{ CTLX | '^', cmd_grow_window },
	{ META | CONTROL | 'C', cmd_count_words },
	{ META | CONTROL | 'D', cmd_change_screen_size },
	{ META | CONTROL | 'E', cmd_execute_procedure },
	{ META | CONTROL | 'F', cmd_goto_matching_fence },
	{ META | CONTROL | 'H', cmd_delete_previous_word },
	{ META | CONTROL | 'K', cmd_unbind_key },
	{ META | CONTROL | 'L', cmd_redraw_display },
	{ META | CONTROL | 'M', cmd_delete_global_mode },
	{ META | CONTROL | 'N', cmd_name_buffer },
	{ META | CONTROL | 'R', cmd_query_replace_string },
	{ META | CONTROL | 'S', cmd_change_screen_size },
	{ META | CONTROL | 'T', cmd_change_screen_width },
	{ META | CONTROL | 'V', cmd_scroll_next_down },
	{ META | CONTROL | 'W', cmd_kill_paragraph },
	{ META | CONTROL | 'Z', cmd_scroll_next_up },
	{ META | ' ', cmd_set_mark },
	{ META | '!', cmd_redraw_display },
	{ META | '.', cmd_set_mark },
	{ META | '>', cmd_end_of_file },
	{ META | '<', cmd_beginning_of_file },
	{ META | '?', cmd_help },
	{ META | '~', cmd_unmark_buffer },
	{ META | 'A', cmd_apropos },
	{ META | 'B', cmd_previous_word },
	{ META | 'C', cmd_case_word_capitalize },
	{ META | 'D', cmd_delete_next_word },
	{ META | 'F', cmd_next_word },
	{ META | 'G', cmd_goto_line },
	{ META | 'J', cmd_justify_paragraph },
	{ META | 'K', cmd_bind_to_key },
	{ META | 'L', cmd_case_word_lower },
	{ META | 'M', cmd_add_global_mode },
	{ META | 'N', cmd_next_paragraph },
	{ META | 'P', cmd_previous_paragraph },
	{ META | 'Q', cmd_fill_paragraph },
	{ META | 'R', cmd_replace_string },
	{ META | 'S', cmd_search_forward },		/* alternative P.K.     */
	{ META | 'U', cmd_case_word_upper },
	{ META | 'V', cmd_previous_page },
	{ META | 'W', cmd_copy_region },
	{ META | 'X', cmd_execute_named_command },
	{ META | 'Z', cmd_quick_exit },
	{ META | 0x7F, cmd_delete_previous_word },
	{ SPEC | '1', cmd_incremental_search },		/* VT220 keys   */
	{ SPEC | '2', cmd_yank },
	{ SPEC | '3', cmd_kill_region },
	{ SPEC | '4', cmd_set_mark },
	{ SPEC | '5', cmd_previous_page },
	{ SPEC | '6', cmd_next_page },
	{ SPEC | 'A', cmd_previous_line },
	{ SPEC | 'B', cmd_next_line },
	{ SPEC | 'C', cmd_forward_character },
	{ SPEC | 'D', cmd_backward_character },
	{ SPEC | 'c', cmd_meta_prefix },
	{ SPEC | 'd', cmd_backward_character },
	{ SPEC | 'e', cmd_next_line },
	{ SPEC | 'f', cmd_beginning_of_file },
	{ SPEC | 'i', cmd_ctlx_prefix },

	{ 0x7F, cmd_delete_previous_character },

	/* special internal bindings */
	{ SPEC | META | 'W', cmd_wrap_word },	/* called on word wrap */
	{ SPEC | META | 'C', cmd_nop },	/*  every command input */
	{ SPEC | META | 'R', cmd_nop },	/*  on file read */
	{ SPEC | META | 'X', cmd_nop },	/*  on buffer change */

	{ 0, NULL }
};

#endif				/* EBIND_H_ */
