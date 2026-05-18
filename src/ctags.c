#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ctags.h"
#include "global.h"
#include "memory.h"
#include "text.h"
#include "edit.h"
#include "window.h"
#include <stdlib.h>
#include <string.h>

/* Purpose: CTAG parsing logic:
             Parses standard CTAG format (name<tab>file<tab>pattern)</tab></tab>
             Stores tags in memory for fast lookup
             Handles line number extraction from patterns
             Memory management for tag storage
             Uses file utility functions for consistency with project infrastructure
*/

#define MAX_TAG_LINE 1024

typedef struct
{
    char *name;
    char *file;
    char *pattern;
    int line_number;
} tag_entry_t;

static tag_entry_t *tags = NULL;
static int num_tags = 0;
static int tags_loaded = 0;

int load_ctags(const char *filename)
{
	short fd;
	long file_sz;
	char *buffer = NULL;
	char *line_ptr, *line_end;
	char line[MAX_TAG_LINE];
	char *tab1, *tab2, *tab3;
	int count = 0;
	long bytes_read = 0;

	/* Free previous tags */
	if (tags)
	{
		int i;
		for (i = 0; i < num_tags; i++)
		{
			if (tags[i].name) free(tags[i].name);
			if (tags[i].file) free(tags[i].file);
			if (tags[i].pattern) free(tags[i].pattern);
		}
		free(tags);
		tags = NULL;
	}

	/* Check if file exists using project utilities */
	file_sz = file_size((char *)filename);
	if (file_sz <= 0)
	{
		return 0;
	}

	/* Open file using project utilities */
	fd = (short)Fopen((char *)filename, 0);
	if (fd <= 0)
	{
		return 0;
	}

	/* Allocate buffer for file reading */
	buffer = (char *)malloc(file_sz + 1);
	if (!buffer)
	{
		Fclose(fd);
		return 0;
	}

	/* Read entire file */
	bytes_read = Fread(fd, file_sz, buffer);
	Fclose(fd);

	if (bytes_read <= 0)
	{
		free(buffer);
		return 0;
	}

	buffer[bytes_read] = '\0';

	/* First pass: count valid tag entries */
	line_ptr = buffer;
	while (line_ptr < buffer + bytes_read)
	{
		/* Find end of line */
		line_end = strchr(line_ptr, '\n');
		if (!line_end)
			line_end = buffer + bytes_read;

		/* Skip empty lines and comments */
		if (line_end != line_ptr && line_ptr[0] != '!')
		{
			if (strchr(line_ptr, '\t'))
				count++;
		}

		line_ptr = line_end + 1;
	}

	if (count == 0)
	{
		free(buffer);
		return 0;
	}

	/* Allocate memory for tags */
	tags = (tag_entry_t *)malloc(count * sizeof(tag_entry_t));
	if (!tags)
	{
		free(buffer);
		return 0;
	}

	/* Second pass: parse tags */
	num_tags = 0;
	line_ptr = buffer;

	while (line_ptr < buffer + bytes_read && num_tags < count)
	{
		/* Extract line */
		line_end = strchr(line_ptr, '\n');
		if (!line_end)
			line_end = buffer + bytes_read;

		/* Copy line to buffer, handling CR/LF */
		int line_len = line_end - line_ptr;
		if (line_len > 0 && line_ptr[line_len - 1] == '\r')
			line_len--;
		if (line_len >= MAX_TAG_LINE)
			line_len = MAX_TAG_LINE - 1;

		strncpy(line, line_ptr, line_len);
		line[line_len] = '\0';

		/* Skip empty lines and comments */
		if (line[0] == '\0' || line[0] == '!')
		{
			line_ptr = line_end + 1;
			continue;
		}

		/* Parse: name<TAB>file<TAB>pattern */
		tab1 = strchr(line, '\t');
		if (!tab1)
		{
			line_ptr = line_end + 1;
			continue;
		}
		*tab1++ = '\0';

		tab2 = strchr(tab1, '\t');
		if (!tab2)
		{
			line_ptr = line_end + 1;
			continue;
		}
		*tab2++ = '\0';

		tab3 = strchr(tab2, '\t');
		if (tab3) *tab3++ = '\0';

		/* Allocate and copy strings */
		tags[num_tags].name = strdup(line);
		tags[num_tags].file = strdup(tab1);
		tags[num_tags].pattern = strdup(tab2);
		tags[num_tags].line_number = 0;

		/* Try to extract line number from pattern like /pattern/ or ?pattern? */
		if (tags[num_tags].pattern && tags[num_tags].pattern[0])
		{
			char pattern_delim = tags[num_tags].pattern[0];
			if (pattern_delim == '/' || pattern_delim == '?')
			{
				char *end = strrchr(tags[num_tags].pattern, pattern_delim);
				if (end && end != tags[num_tags].pattern)
				{
					*end = '\0';
					/* Check if it's a line number */
					if (tags[num_tags].pattern[1] && isdigit(tags[num_tags].pattern[1]))
					{
						tags[num_tags].line_number = atoi(tags[num_tags].pattern + 1);
					}
				}
			}
		}

		if (tags[num_tags].name && tags[num_tags].file && tags[num_tags].pattern)
		{
			num_tags++;
		}

		line_ptr = line_end + 1;
	}

	free(buffer);
	tags_loaded = 1;
	return num_tags;
}

const char *find_tag(const char *tag_name)
{
	int i;

	if (!tags_loaded || !tags)
	{
		return NULL;
	}

	for (i = 0; i < num_tags; i++)
	{
		if (strcmp(tags[i].name, tag_name) == 0)
		{
			return tags[i].pattern;
		}
	}

	return NULL;
}

const char *get_tag_file(const char *tag_name)
{
	int i;

	if (!tags_loaded || !tags)
	{
		return NULL;
	}

	for (i = 0; i < num_tags; i++)
	{
		if (strcmp(tags[i].name, tag_name) == 0)
		{
			return tags[i].file;
		}
	}

	return NULL;
}

int get_tag_line(const char *tag_name)
{
	int i;

	if (!tags_loaded || !tags)
	{
		return -1;
	}

	for (i = 0; i < num_tags; i++)
	{
		if (strcmp(tags[i].name, tag_name) == 0)
		{
			return tags[i].line_number;
		}
	}

	return -1;
}

/* Load a file into an edit window if not already loaded and return its TEXTP.
   Returns NULL on failure. */
TEXTP load_or_get_text(const char *full_tag_path)
{
	short win_link = text_still_loaded((char *)full_tag_path);
	TEXTP target_text = NULL;

	if (win_link >= 0)
	{
		target_text = get_text(win_link);
	}
	else
	{
		load_edit((char *)full_tag_path, FALSE);
		win_link = text_still_loaded((char *)full_tag_path);
		if (win_link >= 0)
			target_text = get_text(win_link);
	}
	return target_text;
}

/* Navigate inside an already-loaded TEXTP to the tag location. If tag_line > 0
   it is treated as 1-based and converted to 0-based. Otherwise the function
   will attempt to find the CTAGS pattern for 'word' and search the file for
   the first line containing that pattern. Returns TRUE if navigation moved
   the cursor, FALSE otherwise. */
bool navigate_to_tag_in_text(TEXTP target_text, int tag_line, const char *word)
{
	if (!target_text) return FALSE;

	if (tag_line > 0)
	{
		long target_y = (long)tag_line - 1L;
		LINEP line_ptr = get_line(&target_text->text, target_y);
		if (line_ptr && !IS_TAIL(line_ptr))
		{
			target_text->cursor_line = line_ptr;
			target_text->xpos = 0;
			target_text->ypos = target_y;
			target_text->up_down = FALSE;
			make_chg(target_text->link, POS_CHANGE, 0);

			WINDOWP target_window = get_window(target_text->link);
			if (target_window)
				top_window(target_window);
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		const char *tag_pattern = find_tag(word);
		if (tag_pattern && tag_pattern[0] != EOS)
		{
			const char *pat = tag_pattern;
			if (pat[0] == '/' || pat[0] == '?') pat++;

			char *patbuf = strdup(pat);
			if (!patbuf) return FALSE;

			size_t bl = strlen(patbuf);
			if (bl > 0 && patbuf[0] == '^') memmove(patbuf, patbuf + 1, bl);
			bl = strlen(patbuf);
			if (bl > 0 && patbuf[bl - 1] == '$') patbuf[bl - 1] = '\0';

			LINEP scan = FIRST(&target_text->text);
			long scan_y = 0;
			while (!IS_TAIL(scan))
			{
				if (patbuf[0] != '\0' && strstr(TEXT(scan), patbuf) != NULL)
				{
					target_text->cursor_line = scan;
					target_text->xpos = 0;
					target_text->ypos = scan_y;
					target_text->up_down = FALSE;
					make_chg(target_text->link, POS_CHANGE, 0);

					WINDOWP target_window = get_window(target_text->link);
					if (target_window) top_window(target_window);

					free(patbuf);
					return TRUE;
				}
				NEXT(scan);
				scan_y++;
			}
			free(patbuf);
		}
		return FALSE;
	}
}

