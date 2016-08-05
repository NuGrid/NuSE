#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <SE.h>

int main(int argc, char *argv[])
{
    int i, file_id, len;
    char *f;

    if (argc != 2) {
	len = strlen(argv[0]) + 1;
	f = (char *)malloc(len * sizeof(char));
	strncpy(f, argv[0], len);
	fprintf(stderr, "usage: %s file ...\n", basename(f));
	free(f);
	exit(1);
    }

    for (i = 1; i < argc; ++i) {
	file_id = SEopen(argv[i]);
	SEclose(file_id);
    }

    return 0;
}
