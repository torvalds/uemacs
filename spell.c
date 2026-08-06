/*
 *	spell.c
 *
 *	The spell checking, which is hunspell and the search for its
 *	dictionaries.  The editor's whole side of it is spellcheck(),
 *	called for one word at a time by the display.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "estruct.h"
#include "globals.h"
#include "efunc.h"
#include "utf8.h"
#include "util.h"

#ifdef HUNSPELL

#include <hunspell.h>

/*
 * The languages to check against.  A word is spelled correctly if any one
 * of these says so, which is how "colour" and "color" both come out right
 * without having to pick a side.  Another language is a line here.
 *
 * They cannot share one hunspell handle.  Hunspell_add_dic() bolts an
 * extra word list onto an existing one and the two share the first one's
 * affix rules, which is right for a personal word list and wrong for a
 * second language: en_GB.dic's entries carry flags that mean what
 * en_GB.aff says they mean, and en_GB.aff defines sixty-four of them
 * against en_US.aff's twenty-three.  Loaded that way the stems arrive and
 * the inflections quietly do not - "anarchical" and "almsman" are the two
 * that turned up while checking this.
 */
static const char *languages[] = { "en_US", "en_GB" };

/*
 * Where to look for them.  $DICPATH is hunspell's own variable and is a
 * colon-separated search path, so it is split like $PATH; the rest is
 * where the distributions that were to hand keep theirs.
 */
static const char *dictionary_path[] = {
	"/usr/share/hunspell", "/usr/share/myspell",
	"/usr/local/share/hunspell", "/usr/local/share/myspell",
};

static Hunhandle *spellers[ARRAY_SIZE(languages)];
static int nspellers;

int spellcheck(const char *word)
{
	for (int i = 0; i < nspellers; i++)
		if (Hunspell_spell(spellers[i], word))
			return 1;
	/* with nothing to check against, nothing is misspelled */
	return nspellers == 0;
}

static void local_dictionary(Hunhandle *handle, const char *filename)
{
	struct stat st;
	if (!stat(filename, &st) && S_ISREG(st.st_mode))
		Hunspell_add_dic(handle, filename);
}

/*
 * One language out of one directory, or NULL if it is not there.  Both
 * halves have to exist: hunspell will hand back a handle for a missing
 * dictionary and then say every word is wrong.
 */
static Hunhandle *open_dictionary(const char *dir, int dirlen, const char *lang)
{
	char aff[NFILEN], dic[NFILEN];

	if (dirlen <= 0)
		return NULL;
	snprintf(aff, sizeof(aff), "%.*s/%s.aff", dirlen, dir, lang);
	snprintf(dic, sizeof(dic), "%.*s/%s.dic", dirlen, dir, lang);
	if (access(aff, R_OK) || access(dic, R_OK))
		return NULL;
	return Hunspell_create(aff, dic);
}

static Hunhandle *find_dictionary(const char *lang)
{
	const char *dicpath = getenv("DICPATH");

	while (dicpath && *dicpath) {
		const char *end = strchr(dicpath, ':');
		int len = end ? end - dicpath : (int)strlen(dicpath);
		Hunhandle *h = open_dictionary(dicpath, len, lang);

		if (h)
			return h;
		dicpath = end ? end + 1 : NULL;
	}
	for (int i = 0; i < ARRAY_SIZE(dictionary_path); i++) {
		Hunhandle *h = open_dictionary(dictionary_path[i],
					       strlen(dictionary_path[i]), lang);
		if (h)
			return h;
	}
	return NULL;
}

void spell_init(void)
{
	const char *home;

	for (int i = 0; i < ARRAY_SIZE(languages); i++) {
		Hunhandle *h = find_dictionary(languages[i]);

		if (h)
			spellers[nspellers++] = h;
	}
	if (nspellers == 0)
		return;

	/* the personal word list goes on the first, and shares its rules */
	local_dictionary(spellers[0], ".dictionary");
	home = getenv("HOME");
	if (home) {
		char buf[NFILEN];

		snprintf(buf, sizeof(buf), "%s/.dictionary", home);
		local_dictionary(spellers[0], buf);
	}
}

#else

/*
 * Built without hunspell.  This is the same answer the real thing gives
 * when it finds no dictionaries to open: with nothing to check against,
 * nothing is misspelled.  Spell mode still exists and still toggles, it
 * just never has anything to say.
 */
void spell_init(void)
{
}

int spellcheck(const char *word)
{
	return 1;
}

#endif
