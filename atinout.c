
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_LINE_LENGTH (4 * 1024)
static char buf[MAX_LINE_LENGTH];

/* Replace '\n' with '\r', aka `tr '\012' '\015'` */
static bool tr_lf_cr(const char *s)
{
	char *p;
	p = strchr(s, '\n');
	if (p == NULL || p[1] != '\0') {
		return false;
	}
	*p = '\r';
	return true;
}

bool is_final_result(const char * const response)
{
#define STARTS_WITH(a, b) ( strncmp((a), (b), strlen(b)) == 0)
	switch (response[0]) {
	case '+':
		if (STARTS_WITH(&response[1], "CME ERROR:")) {
			return true;
		}
		if (STARTS_WITH(&response[1], "CMS ERROR:")) {
			return true;
		}
		return false;
	case 'B':
		if (strcmp(&response[1], "USY\r\n") == 0) {
			return true;
		}
		return false;

	case 'E':
		if (strcmp(&response[1], "RROR\r\n") == 0) {
			return true;
		}
		return false;
	case 'N':
		if (strcmp(&response[1], "O ANSWER\r\n") == 0) {
			return true;
		}
		if (strcmp(&response[1], "O CARRIER\r\n") == 0) {
			return true;
		}
		if (strcmp(&response[1], "O DIALTONE\r\n") == 0) {
			return true;
		}
		return false;
	case 'O':
		if (strcmp(&response[1], "K\r\n") == 0) {
			return true;
		}
	default:
		return false;
	}

}

int main(int argc, char *argv[])
{
	FILE *atcmds;
	FILE *modem;
	char *line;
	bool success;

	(void)argc;

#define INPUT_FILE   argv[1]
#define MODEM_DEVICE argv[2]

	if (strcmp(INPUT_FILE, "-") == 0) {
		atcmds = stdin;
	} else {
		atcmds = fopen(INPUT_FILE, "rb");
		if (atcmds == NULL) {
			fprintf(stderr, "fopen(%s) failed: %s\n", INPUT_FILE, strerror(errno));
			return EXIT_FAILURE;
		}
	}

	modem = fopen(MODEM_DEVICE, "r+b");
	if (modem == NULL) {
		fprintf(stderr, "fopen(%s) failed: %s\n", MODEM_DEVICE, strerror(errno));
		return EXIT_FAILURE;
	}

	goto start;
	while (line != NULL) {
		success = tr_lf_cr(line);
		if (! success) {
			fprintf(stderr, "invalid string: '%s'\n", line);
			return EXIT_FAILURE;
		}
		fputs(line, modem);
		do {
			line = fgets(buf, sizeof(buf), modem);
			if (line == NULL) {
				fputs("EOF from modem\n", stderr);
				return EXIT_FAILURE;
			}
		} while (! is_final_result(line));
start:
		line = fgets(buf, sizeof(buf), atcmds);
	}

	fclose(modem);
	if (strcmp(INPUT_FILE, "-") != 0) {
		fclose(atcmds);
	}
	return EXIT_SUCCESS;
}

