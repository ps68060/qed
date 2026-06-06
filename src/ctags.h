/*
 * CTAG file handling for QED
 */

#ifndef CTAGS_H
#define CTAGS_H

#include <stdbool.h>
#include "global.h"

int load_ctags(const char *filename);
const char *find_tag(const char *tag_name);
const char *get_tag_file(const char *tag_name);
int get_tag_line(const char *tag_name);

/* Helpers: load the target text or navigate to a tag
   (numeric 1-based line or pattern). */
extern TEXTP load_or_get_text(const char *full_tag_path);
extern bool navigate_to_tag_in_text(TEXTP target_text, int tag_line, const char *word);

#endif /* CTAGS_H */
