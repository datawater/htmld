#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>
#include <fcntl.h>

#include "version.h"

#define FLAG_IMPLEMENTATION
#include "include/flag.h" /* https://github.com/tsoding/flag.h */

#define CONSOLE_IMPLEMENTATION
#include "include/console.h"

#define UTILS_IMPLEMENTATION
#include "include/utils.h"

typedef enum {
	HREF_PATH,
	SRC_PATH
} pathType;

int* PIPEDORNOT;

void usage(FILE *stream, int PIPEDORNOT) {
	if (PIPEDORNOT) {
    	fprintf(stream, "Usage: ./htmld [OPTIONS] [--] <INPUT FILES..>\n");
    	fprintf(stream, "OPTIONS:\n");
	} else {
		color(FYELLOW); printf("Author: "); color(RESET); printf("datawater\n");
		color(FYELLOW); printf("Version: "); color(RESET); printf("%d.%d.%d\n", MAJOR_VERSION, MINOR_VERSION, SMALL_VERSION);
		color(FYELLOW); printf("Usage: "); color(RESET); printf("Usage: ./htmld [OPTIONS] [--] <INPUT FILE>\n");
		color(FYELLOW); printf("OPTIONS:\n"); color(RESET);
	}
    flag_print_options(stream);
}

char* get_path(pathType type, char* str) {
	switch (type) {	
		case HREF_PATH: {
			for (int i = 0; i < (int) strlen(str)-4; ++i) {
				char f[5]; for (int j = 0; j < 4; ++j) {f[j] = str[i+j];}
				if (strcmp(f, "href") == 0) {
					// Remove all of the whitespace and the `" or '`
					i += 5; str = trim(str); str += i+1; str = trim(str); str += 1; 
					char toreturn[256];
					for (int p = 0; p < 256; p++) {
						if (str[p] == '"' || str[p] == '\'') {
							toreturn[p] = '\0';
							return strdup(toreturn);
						} 
						toreturn[p] = str[p];
					}
				}
			}
		} break;
		case SRC_PATH: {
			for (int i = 0; i < (int) strlen(str)-3; ++i) {
				char f[4]; for (int j = 0; j < 3; ++j) {f[j] = str[i+j];}
				if (strcmp(f, "src") == 0) {
					// Remove all of the whitespace and the `" or '`
					i += 3; str = trim(str); str += i+1; str = trim(str); str += 1; 
					char toreturn[256];
					for (int p = 0; p < 256; p++) {
						if (str[p] == '"' || str[p] == '\'') {
							toreturn[p] = '\0';
							return strdup(toreturn);
						} 
						toreturn[p] = str[p];
					}
				}
			}
		} break;
		default: {
			return "UNREACHABLE";
		}
	}
	return NULL;
}

void file_parse(char* input, char* output) {
	FILE* inputfp = fopen(input, "r"); FILE* outputfp = fopen(output, "w");

	if (inputfp == NULL || outputfp == NULL) {
		char* errmsg = malloc(sizeof(char) * 64); sprintf(errmsg,"Error opening a file: %s\n", strerror(errno));
		!*PIPEDORNOT ? error(errmsg) : (void) printf(errmsg);
		exit(1);
	}

	int len = 1024; char line[len]; 
 
	while (fgets(line,len, inputfp)) {
		char* five = strchp(trim(line), 5);
		if (strcmp(five, "<link") == 0 &&  strstr(trim(line), "stylesheet")) {
			if (strstr(line, "http://") || strstr(line,"https://")) {
				// TODO: add http(s) support
				todo("Implement http/https support", __LINE__, __FILE__, 1);
				fprintf(outputfp, "%s", line);
				continue;
			}
			char* path_     = get_path(HREF_PATH,trim(line));
			char* inputdir = remove_filename(input);
			char* path = strcat(inputdir, path_);

			// TODO: make this cross-platform

			int fd = open(path, O_RDONLY);
			int len = lseek(fd, 0, SEEK_END);
			// TODO: also minify the code
			unsigned char *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

			fprintf(outputfp, "<style>%s</style>\n", data);
			free(path_);
			free(inputdir);
		} else if (strcmp(five, "<scri") == 0) {
			if (strstr(line, "http://") || strstr(line,"https://")) {
				// TODO: add http(s) support
				todo("Implement http/https support", __LINE__, __FILE__, 1);
				fprintf(outputfp, "%s", line);
				continue;
			}
			char* path_     = get_path(SRC_PATH,trim(line));
			char* inputdir = remove_filename(input);
			char* path = strcat(inputdir, path_);
			// TODO: make this cross-platform

			int fd = open(path, O_RDONLY);
			int len = lseek(fd, 0, SEEK_END);
			// TODO: also minify the code
			unsigned char *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

			fprintf(outputfp, "<script>%s</script>\n", data);

			free(path_);
			free(inputdir);
		} else {
			fprintf(outputfp, "%s", line);
		}
		free(five);
	}
	fclose(inputfp); fclose(outputfp);

}

int main(int argc, char** argv) {
	(void) argc; (void) argv;
	PIPEDORNOT = malloc(sizeof(int));
	*PIPEDORNOT = !(isatty(STDOUT_FILENO));

	bool *help        = flag_bool("help", false, "Print the help message!");
	char **input    = flag_str("input", "\0", "Input file (Required)");
	char **output = flag_str("output", "\0", "Output file");

	if (!flag_parse(argc, argv)) {
		usage(stdout, *PIPEDORNOT);
		*PIPEDORNOT ? flag_print_options(stdout) :  color(FRED); flag_print_error(stdout); color(RESET); 
		exit(1);
	} else if (*help) {
		usage(stdout, *PIPEDORNOT);
        exit(0);
	}

    if (strcmp(*input, "\0") == 0) {
        usage(stdout, *PIPEDORNOT);
        !*PIPEDORNOT ? error("No Input file provided") : (void) printf("[ERROR] No Input file provided\n");
        exit(1);
    } else if (strcmp(*output, "\0") == 0) {
		!*PIPEDORNOT ? warn("No output file provided using default output file") : (void) printf("[WARN] No output file provided using default output file");
		*output = "output.html";
	}

	if (!file_exist(*input)) {
		!*PIPEDORNOT ? error("Input file doesn't exist") : (void) printf("[ERROR] Input file doesn't exist\n");
	}

	file_parse(*input, *output);
	free(PIPEDORNOT);
	return 0;
}