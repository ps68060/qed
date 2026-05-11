#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctags.h"

/* Purpose: CTAG parsing logic:
            Parses standard CTAG format (name<tab>file<tab>pattern)</tab></tab>
            Stores tags in memory for fast lookup
            Handles line number extraction from patterns
            Memory management for tag storage
*/

#define MAX_TAG_LINE 1024

typedef struct {
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
	FILE *fp;
	char line[MAX_TAG_LINE];
	char *tab1, *tab2, *tab3;
	int count = 0;

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

	fp = fopen(filename, "r");
	if (!fp)
	{
	    printf("%s tag file not found\n", filename);
		return 0;
	}

	/* Count lines first */
	while (fgets(line, sizeof(line), fp))
	{
		count++;
	}
	rewind(fp);

	if (count == 0)
	{
		fclose(fp);
		return 0;
	}

	/* Allocate memory */
	tags = (tag_entry_t *)malloc(count * sizeof(tag_entry_t));
	if (!tags)
	{
		fclose(fp);
		return 0;
	}

	/* Read tags */
	num_tags = 0;
	while (fgets(line, sizeof(line), fp) && num_tags < count)
	{
		/* Remove newline */
		line[strcspn(line, "\n")] = 0;

		/* Skip empty lines and comments */
		if (line[0] == '\0' || line[0] == '!')
		{
			continue;
		}

		/* Parse: name<TAB>file<TAB>pattern */
		tab1 = strchr(line, '\t');
		if (!tab1) continue;
		*tab1++ = '\0';

		tab2 = strchr(tab1, '\t');
		if (!tab2) continue;
		*tab2++ = '\0';

		tab3 = strchr(tab2, '\t');
		if (tab3) *tab3++ = '\0';

		/* Allocate and copy strings */
		tags[num_tags].name = strdup(line);
		tags[num_tags].file = strdup(tab1);
		tags[num_tags].pattern = strdup(tab2);
		tags[num_tags].line_number = 0;

		/* Try to extract line number from pattern like /pattern/ or ?pattern? */
		if (tags[num_tags].pattern[0] == '/' || tags[num_tags].pattern[0] == '?')
		{
			char *end = strrchr(tags[num_tags].pattern, tags[num_tags].pattern[0]);
			if (end && end != tags[num_tags].pattern)
			{
				*end = '\0';
				/* Check if it's a line number */
				if (isdigit(tags[num_tags].pattern[1]))
				{
					tags[num_tags].line_number = atoi(tags[num_tags].pattern + 1);
				}
			}
		}

		if (tags[num_tags].name && tags[num_tags].file && tags[num_tags].pattern)
		{
			num_tags++;
		}
	}

	fclose(fp);
	tags_loaded = 1;
	return num_tags;
}

const char *find_tag(const char *tag_name)
{
	int i;

	if (!tags_loaded || !tags) {
		return NULL;
	}

	for (i = 0; i < num_tags; i++) {
		if (strcmp(tags[i].name, tag_name) == 0) {
			return tags[i].pattern;
		}
	}

	return NULL;
}

const char *get_tag_file(const char *tag_name)
{
	int i;

	if (!tags_loaded || !tags) {
		return NULL;
	}

	for (i = 0; i < num_tags; i++) {
		if (strcmp(tags[i].name, tag_name) == 0) {
			return tags[i].file;
		}
	}

	return NULL;
}

int get_tag_line(const char *tag_name)
{
	int i;

	if (!tags_loaded || !tags) {
		return -1;
	}

	for (i = 0; i < num_tags; i++) {
		if (strcmp(tags[i].name, tag_name) == 0) {
			return tags[i].line_number;
		}
	}

	return -1;
}
