
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_LINE_LENGTH (4 * 1024)
static char buf[MAX_LINE_LENGTH];

int main(int argc, char *argv[])
{
	FILE *atcmds;
	char *line;

	(void)argc;

#define INPUT_FILE   argv[1]

	if (strcmp(INPUT_FILE, "-") == 0) {
		atcmds = stdin;
	} else {
		atcmds = fopen(INPUT_FILE, "rb");
		if (atcmds == NULL) {
			fprintf(stderr, "fopen(%s) failed: %s\n", INPUT_FILE, strerror(errno));
			return EXIT_FAILURE;
		}
	}

	goto start;
	while (line != NULL) {
start:
		line = fgets(buf, sizeof(buf), atcmds);
	}

	if (strcmp(INPUT_FILE, "-") != 0) {
		fclose(atcmds);
	}
	return EXIT_SUCCESS;
}

