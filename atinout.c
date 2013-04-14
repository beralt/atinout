
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>

#define MAX_LINE_LENGTH (4 * 1024)
static char buf[MAX_LINE_LENGTH];
static char buf2[MAX_LINE_LENGTH];

#define VERSION "0.9"

static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'V'},
	{"usage", no_argument, 0, 0},
	{0, 0, 0, 0}
};

static void usage(const char * const argv0)
{
	printf("Usage: %s <input_file> <modem_device> <output_file>\n", argv0);
	printf("\n");
	printf("\t<input_file> is a file with AT commands to be executed. Value '-' means standard input.\n");
	printf("\t<input_file> is the device file for the modem (e.g. /dev/ttyACM0, /dev/ttyS0, etc).\n");
	printf("\t<output_file> is a file the responses to the AT commands are saved. Value '-' means standard output.\n");
	printf("\n");
	printf("In addition the program supports the following options:\n");
	printf("\n");
	printf("\t-h|--help\n");
	printf("\t-V|--version\n");
	printf("\t--usage\n");
	printf("\n");
}

static void help(const char * const argv0)
{
	printf("This program takes AT commands as input, sends them to the modem and outputs\n");
	printf("the responses.\n");
	printf("\n");
	printf("Example:\n");
	printf("\n");
	printf("$ echo 'AT+CPBS=\"ME\",\"ME\"; +CPBR=?' | %s - /dev/ttyACM0 -\n", argv0);
	printf("AT+CPBS=\"ME\",\"ME\"; +CPBR=?\n");
	printf("\n");
	printf("+CPBR: (1-7000),80,62\n");
	printf("\n");
	printf("OK\n");
	printf("$\n");

}

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

static void strip_cr(char *s)
{
	char *from, *to;
	from = to = s;
	while (*from != '\0') {
		if (*from == '\r') {
			from++;
			continue;
		}
		*to++ = *from++;
	}
	*to = '\0';
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
	FILE *output;
	char *line;
	bool success;


	while (1) {
		int option_index = 0;
		int c;

		c = getopt_long(argc, argv, "hV", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;
		case 'V':
			printf("atinout version " VERSION "\n");
			if (argc == 2) {
				return EXIT_SUCCESS;
			}
			break;
		case 0:
			if (strcmp("usage", long_options[option_index].name) == 0) {
				usage(argv[0]);
				return EXIT_SUCCESS;
			}
			break;
		case '?':
			break;
		default:
			fprintf(stderr, "getopt returned character code 0%o\n", c);
		}
	}

	if ((argc - optind) != 3) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

#define INPUT_FILE   argv[optind + 0]
#define MODEM_DEVICE argv[optind + 1]
#define OUTPUT_FILE  argv[optind + 2]

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

	if (strcmp(OUTPUT_FILE, "-") == 0) {
		output = stdout;
	} else {
		output = fopen(OUTPUT_FILE, "wb");
		if (modem == NULL) {
			fprintf(stderr, "fopen(%s) failed: %s\n", OUTPUT_FILE, strerror(errno));
			return EXIT_FAILURE;
		}
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
			strcpy(buf2, line);
			strip_cr(buf2);
			fputs(buf2, output);
		} while (! is_final_result(line));
start:
		line = fgets(buf, sizeof(buf), atcmds);
	}

	if (strcmp(OUTPUT_FILE, "-") != 0) {
		fclose(output);
	}
	fclose(modem);
	if (strcmp(INPUT_FILE, "-") != 0) {
		fclose(atcmds);
	}
	return EXIT_SUCCESS;
}

