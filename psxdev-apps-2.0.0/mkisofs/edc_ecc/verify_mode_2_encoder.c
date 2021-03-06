/* test program for the sector formatting library */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ecc.h"

int main(int argc, char *argv[])
{
	int fin;
	unsigned int sector = 0;
	unsigned int verified = 0;
	int address;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s 2448-byte-sector-file\n", argv[0]);
		return 0;
	}

	if (argv[1][0] == '-') {
		fin = STDIN_FILENO;
	} else {
		fin = open(argv[1], O_RDONLY);
	}
	if (fin == -1) {
		perror("");
		fprintf(stderr, "could not open file %s for reading.\n",argv[1]);
		exit(1);
	}

	address = 150;
	for (; 1; sector++) {
		unsigned char inbuf[2448];
		unsigned char inbuf_copy[2448];
		int have_read;

		/* get one original sector */
		have_read = 0;
		while (2448 != have_read) {
			int retval;

			retval = read(fin, inbuf+have_read, 2448-have_read);
			if (retval < 0) break;

			if (retval == 0)
				break;
			have_read += retval;
		}

	        /* make a copy to work on */
		memcpy(inbuf_copy, inbuf, 2448);

		/* clear fields,  which shall be built */
		memset(inbuf_copy, 0, 16);

		/* build fields */
		do_encode_L2(inbuf_copy, MODE_2, address++);

		/* check fields */
		if (memcmp(inbuf_copy, inbuf, 2448) != 0) {
			fprintf(stderr, "difference at sector %u\n", sector);
		} else verified++;
	}
fprintf(stderr, "%u sectors, %u verified ok\n", sector, verified);
	close(fin);
	return 0;
}
