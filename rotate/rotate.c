#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define	BUFFER_SIZE		3	/* for this exercise */
#define	ROTATE_RIGHT(e, n)	(((n) & 0x01) << 7) | ((e) >> 1)
#define	ROTATE_LEFT(e, n)	((e) << 1) | (((n) & 0x80) >> 7)
#define	MIN(a, b)		((a) < (b) ? (a) : (b))

typedef void (*rotate_func)(int, off_t, off_t, uint8_t *, size_t, size_t *,
    size_t *);

/* See the comment for rotate(), below. */
void
rotate_left(int fd, off_t filesize, off_t foffset, uint8_t *buffer,
    size_t bufsize, size_t *writeoff, size_t *writesize)
{
	size_t i;

	for (i = 0; i < bufsize - 1; i++)
		buffer[i] = ROTATE_LEFT(buffer[i], buffer[i + 1]);

	if (foffset + i == filesize - 1) {
		uint8_t first;

		if (pread(fd, &first, 1, 0) != 1) {
			perror("unexpected read error");
			exit(1);
		}
		buffer[i] = ROTATE_LEFT(buffer[i], first);
		i++;
	}

	*writeoff = 0;
	*writesize = i;
}

/* See the comment for rotate(), below. */
void
rotate_right(int fd, off_t filesize, off_t foffset, uint8_t *buffer,
    size_t bufsize, size_t *writeoff, size_t *writesize)
{
	size_t i;

	for (i = bufsize - 1; i > 0; i--)
		buffer[i] = ROTATE_RIGHT(buffer[i], buffer[i - 1]);

	if (foffset == 0) {
		uint8_t last;

		if (pread(fd, &last, 1, filesize - 1) != 1) {
			perror("unexpected read error");
			exit(1);
		}
		buffer[0] = ROTATE_RIGHT(buffer[0], last);
		*writeoff = 0;
		*writesize = bufsize;
	} else {
		*writeoff = 1;
		*writesize = bufsize - 1;
	}
}

/*
 * rotate() slides a window across the input file, copying the visible bytes
 * into a buffer.  The buffer is passed to a rotator function — left or
 * right — which transforms the bytes in place.  Successive window placements
 * overlap by one byte since, in general, a rotator cannot process the byte at
 * its leading edge.  The rotator advertises the location of transformed bytes,
 * which are written on its return.
 */
void
rotate(int in, int out, off_t filesize, uint8_t *buffer, size_t bufsize,
    rotate_func f)
{
	off_t foffset = 0;		/* offset into file for next read */

	for (;;) {
		size_t roffset;		/* offset into buffer for next read */
		size_t rremaining;	/* bytes remaining to be read */
		size_t woffset;		/* offset into buffer for next write */
		size_t wremaining;	/* bytes remaining to be written */

		rremaining = MIN(bufsize, filesize - foffset);
		roffset = 0;
		while (rremaining) {
			size_t nread;

			nread = pread(in, buffer + roffset, rremaining,
			    foffset + roffset);
			if (nread == -1 || nread == 0) {
				perror("error reading input");
				exit(1);
			}
			roffset += nread;
			rremaining -= nread;
		}

		f(in, filesize, foffset, buffer, roffset, &woffset,
		    &wremaining);

		while (wremaining) {
			ssize_t nwritten;

			nwritten = write(out, buffer + woffset, wremaining);
			if (nwritten == -1) {
				perror("error writing output");
				exit(1);
			}
			wremaining -= nwritten;
		}

		if (foffset + roffset == filesize)
			break;
		else
			foffset += roffset - 1;
	}
}

void
usage(char **argv)
{
	fprintf(stderr, "usage: %s left|right <input> <output>\n", argv[0]);
	exit(1);
}

int
main(int argc, char **argv)
{
	rotate_func rotator;
	char *infile;
	int in;
	struct stat instat;
	char *outfile;
	int out;
	uint8_t buffer[BUFFER_SIZE];

	if (argc != 4)
		usage(argv);

	if (strcmp(argv[1], "left") == 0)
		rotator = &rotate_left;
	else if (strcmp(argv[1], "right") == 0)
		rotator = &rotate_right;
	else
		usage(argv);

	infile = argv[2];
	if ((in = open(infile, O_RDONLY)) == -1) {
		fprintf(stderr, "opening %s failed: %s\n", infile,
		    strerror(errno));
		exit(1);
	}

	if (fstat(in, &instat) == -1) {
		fprintf(stderr, "fstat() failed for %s: %s\n", infile,
		    strerror(errno));
		exit(1);
	}

	if (!S_ISREG(instat.st_mode)) {
		fprintf(stderr, "%s is not a regular file\n", infile);
		exit(1);
	}

	outfile = argv[3];
	out = open(outfile, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (out == -1) {
		fprintf(stderr, "creating %s failed: %s\n", outfile,
		    strerror(errno));
		exit(1);
	}

	if (instat.st_size) {
		rotate(in, out, instat.st_size, buffer, sizeof (buffer),
		    rotator);
	}

	(void) close(in);
	(void) close(out);

	return (0);
}
