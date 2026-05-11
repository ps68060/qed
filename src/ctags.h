/*
 * CTAG file handling for QED
 */

#ifndef CTAGS_H
#define CTAGS_H

int load_ctags(const char *filename);
const char *find_tag(const char *tag_name);
const char *get_tag_file(const char *tag_name);
int get_tag_line(const char *tag_name);

#endif /* CTAGS_H */
