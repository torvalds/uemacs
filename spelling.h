#ifndef EM_SPELLING_H_
#define EM_SPELLING_H_

/*
 *  Initialize spelling module.
 *  Returns 0 on successful init.
 */
int spelling_init(void);

/*
 *  Spell checks a word.
 *  Returns 0 if a word is spelled correctly.
 */
int spellcheck(const char *word);

/*
 *  Free any resources used by the spelling module
 */
void spelling_shutdown(void);

#endif /* EM_SPELLING_H_ */
