/* Copyright (c) 2018, Robert Harris. */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#include <strings.h>
#include <sparse_array.h>

/* A collection of all statistics required. */
struct word_stat {
	size_t ws_n_letters;
	size_t ws_n_words;
	size_t ws_n_lines;
	struct letter *ws_histogram;
	size_t ws_histogram_size;
};

/* A histogram bin for a letter. */
struct letter {
	wchar_t l_letter;
	size_t l_count;
};

/*
 * qsort() comparison function for report_statistics_letters(), below.  Note
 * that, in general, it cannot be assumed that wide characters are represented
 * in alphabetical order.
 */
static int
compare_letter(const void *ap, const void *bp)
{
	wchar_t astring[2] = { 0 };
	wchar_t bstring[2] = { 0 };

	astring[0] = ((struct letter *)ap)->l_letter;
	bstring[0] = ((struct letter *)bp)->l_letter;
	return (wcscoll(astring, bstring));
}

/* qsort() comparison function for report_statistics_letters(), below. */
static int
compare_count(const void *ap, const void *bp)
{
	int a = ((struct letter *)ap)->l_count;
	int b = ((struct letter *)bp)->l_count;

	return ((a == b) ? 0 : ((a < b) ? 1 : -1));
}

/*
 * Report the most common letter(s) found in the input.  A copy of the histogram
 * found by create_statistics is sorted twice:  once to find the most common
 * letters and then again to order them alphabetically.
 */
static void
report_statistics_letters(const struct word_stat *wsp)
{
	size_t histogram_size = wsp->ws_histogram_size;
	struct letter *histogram;
	unsigned int i;
	unsigned int j;

	(void) fprintf(stdout, "Most common letter(s):\t\t\t");

	if (histogram_size == 0) {
		(void) fprintf(stdout, "N/A\n");
		return;
	}

	if ((histogram = malloc(histogram_size *
	    sizeof (struct letter))) == NULL) {
		perror("malloc");
		exit(1);
	}
	bcopy(wsp->ws_histogram, histogram,
	    histogram_size * sizeof (struct letter));

	qsort(histogram, histogram_size, sizeof (struct letter), compare_count);

	for (i = 1; i < histogram_size; i++) {
		if (histogram[i].l_count != histogram[0].l_count)
			break;
	}

	qsort(histogram, i, sizeof (struct letter), compare_letter);

	for (j = 0; j < i; j++) {
		(void) fprintf(stdout, "%s%lc",
		    (j > 0) ? " " : "", (wint_t)histogram[j].l_letter);
	}
	(void) fprintf(stdout, "\n");

	free(histogram);
}

static void
report_statistics_average(const struct word_stat *wsp)
{
	(void) fprintf(stdout, "Average number of letters per word:\t");
	if (wsp->ws_n_words) {
		(void) fprintf(stdout, "%.1f\n",
		    (float)wsp->ws_n_letters / wsp->ws_n_words);
	} else {
		(void) fprintf(stdout, "N/A\n");
	}
}

static void
report_statistics_lines(const struct word_stat *wsp)
{
	(void) fprintf(stdout, "Line count:\t\t\t\t%zu\n", wsp->ws_n_lines);
}

static void
report_statistics_words(const struct word_stat *wsp)
{
	(void) fprintf(stdout, "Whitespace-delimited word count:\t%zu\n",
	    wsp->ws_n_words);
}

static void
destroy_statistics(struct word_stat *wsp)
{
	if (wsp->ws_histogram)
		free(wsp->ws_histogram);
	free(wsp);
}

/* Sparse array iterator callback for create_statistics(), below. */
static void
create_statistics_cb(sparse_index_t index, void *valuep, void *arg)
{
	struct letter **letterpp = (struct letter **)arg;
	struct letter letter;

	letter.l_letter = (wchar_t)index;
	letter.l_count = *((size_t *)valuep);

	*(*letterpp)++ = letter;
}

/*
 * This function collates essential statistics from the input and returns them
 * in an object that subsequent consumers should treat as immutable.
 */
static struct word_stat *
create_statistics(FILE *ifile)
{
	struct word_stat *wsp;
	wint_t wi;
	enum { WORD, DELIMITER } this_cursor, last_cursor;
	struct sparse_array *sap = NULL;
	struct letter *letterp;

	if ((wsp = calloc(1, sizeof (struct word_stat))) == NULL) {
		perror("malloc");
		exit(1);
	}

	/* Walk through the input collecting some essential statistics. */
	last_cursor = DELIMITER;
	while ((wi = getwc(ifile)) != WEOF) {
		this_cursor = (iswspace(wi)) ? DELIMITER : WORD;

		if (this_cursor == DELIMITER) {
			if (wi == L'\n')
				wsp->ws_n_lines++;
		} else {
			if (last_cursor == DELIMITER)
				wsp->ws_n_words++;

			if (iswalpha(wi)) {
				size_t *countp;

				/*
				 * Use the numerical value of the letter as the
				 * index into a sparse array to be used as a
				 * histogram to track letter frequencies.
				 */
				countp = sparse_get_addr(&sap,
				    (wchar_t)toupper(wi));
				if ((*countp)++ == 0)
					wsp->ws_histogram_size++;

				wsp->ws_n_letters++;
			}
		}

		last_cursor = this_cursor;
	}

	if (ferror(ifile)) {
		perror("error parsing input stream");
		exit(1);
	}

	/*
	 * Transform the sparse array into an array of key/value pairs.  For
	 * now, the only consumer of this will be report_statistics_letter();
	 * however, rather than do its work here, the array is left without
	 * further processing to facilitate any possible future additions, e.g.
	 * a printed histogram of letter frequencies.
	 */
	if (wsp->ws_histogram_size) {
		if ((letterp = wsp->ws_histogram =
		    malloc(wsp->ws_histogram_size *
		    sizeof (struct letter))) == NULL) {
			perror("malloc");
			exit(1);
		}
		sparse_iter(sap, create_statistics_cb, &letterp);
	}
	sparse_destroy(sap);

	return (wsp);
}

int
main(int argc, char **argv)
{
	int opt;
	int errflag = 0;
	int wflag = 0;
	int lflag = 0;
	int aflag = 0;
	int Lflag = 0;
	char *iname;
	FILE *ifile;
	struct word_stat *wsp;

	if (setlocale(LC_ALL, "") == NULL) {
		(void) fprintf(stderr,
		    "error setting locale from environment\n");
		exit(1);
	}

	while ((opt = getopt(argc, argv, "wlaL")) != -1) {
		switch (opt) {
		case 'w':
			if (wflag++)
				errflag++;
			break;
		case 'l':
			if (lflag++)
				errflag++;
			break;
		case 'a':
			if (aflag++)
				errflag++;
			break;
		case 'L':
			if (Lflag++)
				errflag++;
			break;
		default:
			errflag++;
		}
	}

	if ((iname = argv[optind]) == NULL)
		errflag++;

	if (errflag) {
		(void) fprintf(stderr, "usage: %s [-w] [-l] [-a] [-L] file\n",
		    argv[0]);
		exit(1);
	}

	if ((ifile = fopen(iname, "r")) == NULL) {
		perror(iname);
		exit(1);
	}

	wsp = create_statistics(ifile);

	if (!wflag && !lflag && !aflag && !Lflag)
		wflag = lflag = aflag = Lflag = 1;
	if (wflag)
		report_statistics_words(wsp);
	if (lflag)
		report_statistics_lines(wsp);
	if (aflag)
		report_statistics_average(wsp);
	if (Lflag)
		report_statistics_letters(wsp);

	destroy_statistics(wsp);
	(void) fclose(ifile);

	return (0);
}
